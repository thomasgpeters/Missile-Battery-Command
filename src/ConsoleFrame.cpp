#include "ConsoleFrame.h"
#include <cstdio>
#include <cmath>
#include <algorithm>

#if USE_COCOS2DX
// ============================================================================
// Cocos2d-x ConsoleFrame — AN/TSQ-73 console housing + live data panels
// ============================================================================

ConsoleFrame* ConsoleFrame::create(float scopeRadius, float panelWidth)
{
    auto* ret = new (std::nothrow) ConsoleFrame();
    if (ret && ret->init(scopeRadius, panelWidth)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ConsoleFrame::init(float scopeRadius, float panelWidth)
{
    if (!Node::init()) return false;

    scopeRadius_ = scopeRadius;
    panelWidth_ = panelWidth;
    trackManager_ = nullptr;
    fireControl_ = nullptr;
    threatBoard_ = nullptr;
    battalionHQ_ = nullptr;
    score_ = 0;
    level_ = 1;
    selectedTrackId_ = -1;

    // Create draw layers
    housingNode_   = cocos2d::DrawNode::create();
    dataPanelNode_ = cocos2d::DrawNode::create();
    dataLabelNode_ = cocos2d::Node::create();

    // Housing behind everything, data panels on top
    addChild(housingNode_,   0);
    addChild(dataPanelNode_, 1);
    addChild(dataLabelNode_, 2);

    // Draw static elements once
    drawShelterBackground();
    drawConsoleHousing();
    drawScopeBezel();
    drawBottomControlStrip();
    drawTopIndicators();

    scheduleUpdate();
    return true;
}

void ConsoleFrame::addMessage(const std::string& msg)
{
    messages_.push_back(msg);
    if ((int)messages_.size() > MAX_MESSAGES) {
        messages_.erase(messages_.begin());
    }
}

void ConsoleFrame::update(float dt)
{
    dataPanelNode_->clear();
    dataLabelNode_->removeAllChildren();

    drawLeftPanel();
    drawRightPanel();
    drawBottomBar();
}

// ============================================================================
// Static background: S-280 shelter interior
// ============================================================================

void ConsoleFrame::drawShelterBackground()
{
    // Dark shelter interior behind the console
    housingNode_->drawSolidRect(
        cocos2d::Vec2(-800, -600),
        cocos2d::Vec2(800, 600),
        cocos2d::Color4F(0.06f, 0.07f, 0.05f, 1.0f));

    // Ceiling (slightly lighter)
    housingNode_->drawSolidRect(
        cocos2d::Vec2(-800, 350),
        cocos2d::Vec2(800, 600),
        cocos2d::Color4F(0.08f, 0.09f, 0.07f, 1.0f));

    // Red night ops light strips on ceiling
    for (int lx = -500; lx <= 500; lx += 250) {
        housingNode_->drawSolidRect(
            cocos2d::Vec2((float)lx - 25.0f, 520.0f),
            cocos2d::Vec2((float)lx + 25.0f, 525.0f),
            cocos2d::Color4F(0.25f, 0.06f, 0.06f, 0.5f));
        // Glow below strip
        housingNode_->drawSolidRect(
            cocos2d::Vec2((float)lx - 40.0f, 500.0f),
            cocos2d::Vec2((float)lx + 40.0f, 520.0f),
            cocos2d::Color4F(0.15f, 0.04f, 0.04f, 0.2f));
    }

    // Floor (darker)
    housingNode_->drawSolidRect(
        cocos2d::Vec2(-800, -600),
        cocos2d::Vec2(800, -400),
        cocos2d::Color4F(0.04f, 0.05f, 0.03f, 1.0f));
}

// ============================================================================
// Console housing — metal cabinet around the scope
// ============================================================================

void ConsoleFrame::drawConsoleHousing()
{
    float r = scopeRadius_;
    float pw = panelWidth_;

    float left   = -r - pw - 30.0f;
    float right  =  r + pw + 30.0f;
    float top    =  r + 45.0f;
    float bottom = -r - 85.0f;

    cocos2d::Color4F housing(0.17f, 0.19f, 0.15f, 1.0f);

    // Top strip
    housingNode_->drawSolidRect(
        cocos2d::Vec2(left, r), cocos2d::Vec2(right, top), housing);

    // Bottom strip
    housingNode_->drawSolidRect(
        cocos2d::Vec2(left, bottom), cocos2d::Vec2(right, -r), housing);

    // Left strip
    housingNode_->drawSolidRect(
        cocos2d::Vec2(left, -r), cocos2d::Vec2(-r, r), housing);

    // Right strip
    housingNode_->drawSolidRect(
        cocos2d::Vec2(r, -r), cocos2d::Vec2(right, r), housing);

    // Corner wedges (fill gap between square strips and circular scope)
    drawCornerWedges(r);

    // Bevel highlight on top edge
    housingNode_->drawLine(
        cocos2d::Vec2(left + 6, top),
        cocos2d::Vec2(right - 6, top),
        cocos2d::Color4F(0.27f, 0.29f, 0.24f, 1.0f));

    // Shadow on bottom edge
    housingNode_->drawLine(
        cocos2d::Vec2(left + 6, bottom),
        cocos2d::Vec2(right - 6, bottom),
        cocos2d::Color4F(0.10f, 0.11f, 0.08f, 1.0f));

    // Console outline
    housingNode_->drawRect(
        cocos2d::Vec2(left, bottom),
        cocos2d::Vec2(right, top),
        cocos2d::Color4F(0.24f, 0.26f, 0.20f, 1.0f));
}

void ConsoleFrame::drawCornerWedges(float r)
{
    cocos2d::Color4F housing(0.17f, 0.19f, 0.15f, 1.0f);
    int steps = 12;

    // Four corners: top-left, top-right, bottom-right, bottom-left
    struct Corner { float cx, cy; float startDeg, endDeg; };
    Corner corners[] = {
        { -r,  r,  90.0f, 180.0f },
        {  r,  r,   0.0f,  90.0f },
        {  r, -r, 270.0f, 360.0f },
        { -r, -r, 180.0f, 270.0f }
    };

    for (const auto& corner : corners) {
        cocos2d::Vec2 prev(corner.cx, corner.cy);
        for (int i = 0; i <= steps; i++) {
            float angle = (corner.startDeg +
                (corner.endDeg - corner.startDeg) * (float)i / steps) * M_PI / 180.0f;
            cocos2d::Vec2 arcPt(r * std::cos(angle), r * std::sin(angle));

            if (i > 0) {
                cocos2d::Vec2 verts[] = {
                    cocos2d::Vec2(corner.cx, corner.cy), prev, arcPt
                };
                housingNode_->drawPolygon(verts, 3, housing, 0, housing);
            }
            prev = arcPt;
        }
    }
}

// ============================================================================
// Scope bezel — rubber hood and metal ring around the PPI scope
// ============================================================================

void ConsoleFrame::drawScopeBezel()
{
    float r = scopeRadius_;

    // Outer rubber hood ring
    housingNode_->drawCircle(
        cocos2d::Vec2::ZERO, r + 9.0f, 0, 72, false, 18.0f,
        cocos2d::Color4F(0.08f, 0.08f, 0.08f, 1.0f));

    // Inner hood bevel
    housingNode_->drawCircle(
        cocos2d::Vec2::ZERO, r + 3.0f, 0, 72, false, 6.0f,
        cocos2d::Color4F(0.12f, 0.12f, 0.12f, 1.0f));

    // Outer metal bezel
    housingNode_->drawCircle(
        cocos2d::Vec2::ZERO, r + 18.0f, 0, 72, false, 2.0f,
        cocos2d::Color4F(0.31f, 0.33f, 0.28f, 1.0f));

    // Inner bezel edge
    housingNode_->drawCircle(
        cocos2d::Vec2::ZERO, r + 1.0f, 0, 72, false, 1.5f,
        cocos2d::Color4F(0.20f, 0.22f, 0.18f, 1.0f));

    // Mounting screws at 45-degree positions
    float screwDist = r + 15.0f;
    float positions[][2] = {
        { -0.707f,  0.707f },
        {  0.707f,  0.707f },
        {  0.707f, -0.707f },
        { -0.707f, -0.707f }
    };
    for (const auto& pos : positions) {
        cocos2d::Vec2 screwPos(screwDist * pos[0], screwDist * pos[1]);
        housingNode_->drawSolidCircle(screwPos, 4.0f, 0, 8,
            cocos2d::Color4F(0.35f, 0.37f, 0.32f, 1.0f));
        housingNode_->drawCircle(screwPos, 4.0f, 0, 8, false,
            cocos2d::Color4F(0.20f, 0.22f, 0.18f, 1.0f));
        // Phillips slot
        housingNode_->drawLine(
            screwPos + cocos2d::Vec2(-2, 0), screwPos + cocos2d::Vec2(2, 0),
            cocos2d::Color4F(0.24f, 0.26f, 0.20f, 1.0f));
        housingNode_->drawLine(
            screwPos + cocos2d::Vec2(0, -2), screwPos + cocos2d::Vec2(0, 2),
            cocos2d::Color4F(0.24f, 0.26f, 0.20f, 1.0f));
    }
}

// ============================================================================
// Bottom control strip: rotary knobs and toggle switches
// ============================================================================

void ConsoleFrame::drawBottomControlStrip()
{
    float r = scopeRadius_;
    float pw = panelWidth_;

    float stripLeft  = -r - pw - 20.0f;
    float stripRight =  r + pw + 20.0f;
    float stripBot   = -r - 75.0f;
    float stripTop   = -r - 25.0f;

    // Strip background
    housingNode_->drawSolidRect(
        cocos2d::Vec2(stripLeft, stripBot),
        cocos2d::Vec2(stripRight, stripTop),
        cocos2d::Color4F(0.16f, 0.17f, 0.14f, 1.0f));

    housingNode_->drawRect(
        cocos2d::Vec2(stripLeft, stripBot),
        cocos2d::Vec2(stripRight, stripTop),
        cocos2d::Color4F(0.22f, 0.24f, 0.19f, 1.0f));

    float knobY = (stripBot + stripTop) * 0.5f;
    float knobPositions[] = { -200.0f, -80.0f, 0.0f, 80.0f, 200.0f };

    for (float kx : knobPositions) {
        cocos2d::Vec2 center(kx, knobY);
        // Knob base
        housingNode_->drawSolidCircle(center, 14.0f, 0, 12,
            cocos2d::Color4F(0.12f, 0.13f, 0.11f, 1.0f));
        // Knob cap
        housingNode_->drawSolidCircle(center, 10.0f, 0, 12,
            cocos2d::Color4F(0.22f, 0.24f, 0.19f, 1.0f));
        // Indicator line
        housingNode_->drawLine(
            center, center + cocos2d::Vec2(0, 8),
            cocos2d::Color4F(0.7f, 0.7f, 0.67f, 1.0f));
        // Ring
        housingNode_->drawCircle(center, 14.0f, 0, 12, false,
            cocos2d::Color4F(0.31f, 0.33f, 0.28f, 1.0f));
    }

    // Toggle switches between knobs
    float switchPositions[] = { -140.0f, -40.0f, 40.0f, 140.0f };
    for (float sx : switchPositions) {
        // Housing
        housingNode_->drawSolidRect(
            cocos2d::Vec2(sx - 6, knobY - 10),
            cocos2d::Vec2(sx + 6, knobY + 10),
            cocos2d::Color4F(0.12f, 0.13f, 0.11f, 1.0f));
        // Toggle (up position)
        housingNode_->drawSolidRect(
            cocos2d::Vec2(sx - 3, knobY - 2),
            cocos2d::Vec2(sx + 3, knobY + 10),
            cocos2d::Color4F(0.63f, 0.63f, 0.59f, 1.0f));
    }
}

// ============================================================================
// Top indicator LEDs
// ============================================================================

void ConsoleFrame::drawTopIndicators()
{
    float r = scopeRadius_;
    float indicatorY = r + 30.0f;

    struct Indicator { float x; float cr, cg, cb; const char* label; };
    Indicator indicators[] = {
        { -100.0f, 0.0f, 0.8f, 0.0f, "PWR" },
        {  -50.0f, 0.8f, 0.8f, 0.0f, "RDR" },
        {    0.0f, 0.0f, 0.8f, 0.0f, "IFF" },
        {   50.0f, 0.8f, 0.0f, 0.0f, "WPN" },
        {  100.0f, 0.0f, 0.8f, 0.0f, "COM" }
    };

    for (const auto& ind : indicators) {
        cocos2d::Vec2 pos(ind.x, indicatorY);

        // LED
        housingNode_->drawSolidCircle(pos, 4.0f, 0, 8,
            cocos2d::Color4F(ind.cr, ind.cg, ind.cb, 0.85f));

        // Glow
        housingNode_->drawSolidCircle(pos, 8.0f, 0, 12,
            cocos2d::Color4F(ind.cr, ind.cg, ind.cb, 0.15f));

        // Label
        auto* label = cocos2d::Label::createWithSystemFont(ind.label, "Courier", 7);
        label->setPosition(pos + cocos2d::Vec2(0, -12));
        label->setTextColor(cocos2d::Color4B(
            (uint8_t)(ind.cr * 200), (uint8_t)(ind.cg * 200),
            (uint8_t)(ind.cb * 200), 160));
        housingNode_->addChild(label);
    }

    // Manufacturer plate
    housingNode_->drawSolidRect(
        cocos2d::Vec2(-60, -scopeRadius_ - 58),
        cocos2d::Vec2(60, -scopeRadius_ - 44),
        cocos2d::Color4F(0.20f, 0.22f, 0.18f, 1.0f));
    housingNode_->drawRect(
        cocos2d::Vec2(-60, -scopeRadius_ - 58),
        cocos2d::Vec2(60, -scopeRadius_ - 44),
        cocos2d::Color4F(0.26f, 0.28f, 0.23f, 1.0f));

    auto* mfgLabel = cocos2d::Label::createWithSystemFont(
        "HUGHES AIRCRAFT", "Courier", 7);
    mfgLabel->setPosition(cocos2d::Vec2(0, -scopeRadius_ - 51));
    mfgLabel->setTextColor(cocos2d::Color4B(140, 145, 130, 200));
    housingNode_->addChild(mfgLabel);
}

// ============================================================================
// Left Panel: Live Track Table
// ============================================================================

void ConsoleFrame::drawLeftPanel()
{
    float r = scopeRadius_;
    float pw = panelWidth_;

    float panelX = -r - pw - 20.0f;
    float panelY = -r + 20.0f;
    float panelH = r * 2.0f - 40.0f;

    // Panel background (dark, overlaid on housing)
    dataPanelNode_->drawSolidRect(
        cocos2d::Vec2(panelX + 4, panelY),
        cocos2d::Vec2(panelX + pw - 4, panelY + panelH),
        cocos2d::Color4F(0.06f, 0.02f, 0.02f, 0.95f));

    dataPanelNode_->drawRect(
        cocos2d::Vec2(panelX + 4, panelY),
        cocos2d::Vec2(panelX + pw - 4, panelY + panelH),
        cocos2d::Color4F(0.16f, 0.06f, 0.06f, 1.0f));

    float textX = panelX + 10.0f;
    float topY = panelY + panelH - 10.0f;

    // Header
    auto* header = cocos2d::Label::createWithSystemFont(
        "TRACK TABLE", "Courier", 9);
    header->setPosition(cocos2d::Vec2(textX, topY));
    header->setAnchorPoint(cocos2d::Vec2(0, 1));
    header->setTextColor(cocos2d::Color4B(255, 60, 60, 255));
    dataLabelNode_->addChild(header);

    // Column headers
    float hdrY = topY - 14.0f;
    const char* cols[] = { "TRK", "CLS", "RNG", "AZM" };
    float colX[] = { 0.0f, 32.0f, 64.0f, 96.0f };

    for (int c = 0; c < 4; c++) {
        auto* colLabel = cocos2d::Label::createWithSystemFont(
            cols[c], "Courier", 7);
        colLabel->setPosition(cocos2d::Vec2(textX + colX[c], hdrY));
        colLabel->setAnchorPoint(cocos2d::Vec2(0, 1));
        colLabel->setTextColor(cocos2d::Color4B(180, 40, 40, 200));
        dataLabelNode_->addChild(colLabel);
    }

    // Separator
    dataPanelNode_->drawLine(
        cocos2d::Vec2(textX, hdrY - 4),
        cocos2d::Vec2(panelX + pw - 12, hdrY - 4),
        cocos2d::Color4F(0.47f, 0.12f, 0.12f, 0.6f));

    // Track rows
    if (!trackManager_) return;

    auto tracks = trackManager_->getAllTracks();
    int maxRows = std::min((int)tracks.size(), 20);
    float lineH = 11.0f;

    for (int i = 0; i < maxRows; i++) {
        const auto& track = tracks[i];
        if (!track.isAlive) continue;

        float rowY = hdrY - 8.0f - lineH * (i + 1);

        // Classification-based color
        cocos2d::Color4B rowColor;
        switch (track.classification) {
            case TrackClassification::HOSTILE:
                rowColor = cocos2d::Color4B(255, 60, 60, 255); break;
            case TrackClassification::FRIENDLY:
                rowColor = cocos2d::Color4B(60, 120, 255, 220); break;
            case TrackClassification::UNKNOWN:
                rowColor = cocos2d::Color4B(255, 180, 60, 220); break;
            default:
                rowColor = cocos2d::Color4B(180, 180, 180, 180); break;
        }

        // Highlight selected track
        if (track.trackId == selectedTrackId_) {
            dataPanelNode_->drawSolidRect(
                cocos2d::Vec2(textX - 2, rowY - 2),
                cocos2d::Vec2(panelX + pw - 12, rowY + lineH - 2),
                cocos2d::Color4F(1.0f, 1.0f, 0.0f, 0.1f));
            rowColor = cocos2d::Color4B(255, 255, 60, 255);
        }

        // Track ID
        auto* trkLabel = cocos2d::Label::createWithSystemFont(
            track.getTrackIdString(), "Courier", 7);
        trkLabel->setPosition(cocos2d::Vec2(textX + colX[0], rowY));
        trkLabel->setAnchorPoint(cocos2d::Vec2(0, 0));
        trkLabel->setTextColor(rowColor);
        dataLabelNode_->addChild(trkLabel);

        // Classification (truncated)
        std::string cls = track.getClassificationString().substr(0, 4);
        auto* clsLabel = cocos2d::Label::createWithSystemFont(cls, "Courier", 7);
        clsLabel->setPosition(cocos2d::Vec2(textX + colX[1], rowY));
        clsLabel->setAnchorPoint(cocos2d::Vec2(0, 0));
        clsLabel->setTextColor(rowColor);
        dataLabelNode_->addChild(clsLabel);

        // Range (NM)
        char rngBuf[16];
        snprintf(rngBuf, sizeof(rngBuf), "%.0f",
                 track.range * GameConstants::KM_TO_NM);
        auto* rngLabel = cocos2d::Label::createWithSystemFont(rngBuf, "Courier", 7);
        rngLabel->setPosition(cocos2d::Vec2(textX + colX[2], rowY));
        rngLabel->setAnchorPoint(cocos2d::Vec2(0, 0));
        rngLabel->setTextColor(rowColor);
        dataLabelNode_->addChild(rngLabel);

        // Azimuth
        char azmBuf[8];
        snprintf(azmBuf, sizeof(azmBuf), "%03d", (int)track.azimuth);
        auto* azmLabel = cocos2d::Label::createWithSystemFont(azmBuf, "Courier", 7);
        azmLabel->setPosition(cocos2d::Vec2(textX + colX[3], rowY));
        azmLabel->setAnchorPoint(cocos2d::Vec2(0, 0));
        azmLabel->setTextColor(rowColor);
        dataLabelNode_->addChild(azmLabel);
    }
}

// ============================================================================
// Right Panel: Battery Status + HQ + Threat
// ============================================================================

void ConsoleFrame::drawRightPanel()
{
    float r = scopeRadius_;
    float pw = panelWidth_;

    float panelX = r + 20.0f;
    float panelY = -r + 20.0f;
    float panelH = r * 2.0f - 40.0f;

    // Panel background
    dataPanelNode_->drawSolidRect(
        cocos2d::Vec2(panelX + 4, panelY),
        cocos2d::Vec2(panelX + pw - 4, panelY + panelH),
        cocos2d::Color4F(0.06f, 0.02f, 0.02f, 0.95f));

    dataPanelNode_->drawRect(
        cocos2d::Vec2(panelX + 4, panelY),
        cocos2d::Vec2(panelX + pw - 4, panelY + panelH),
        cocos2d::Color4F(0.16f, 0.06f, 0.06f, 1.0f));

    float textX = panelX + 10.0f;
    float topY = panelY + panelH - 10.0f;

    // Battery Status header
    auto* header = cocos2d::Label::createWithSystemFont(
        "BATTERY STATUS", "Courier", 9);
    header->setPosition(cocos2d::Vec2(textX, topY));
    header->setAnchorPoint(cocos2d::Vec2(0, 1));
    header->setTextColor(cocos2d::Color4B(255, 60, 60, 255));
    dataLabelNode_->addChild(header);

    float lineH = 11.0f;
    float rowY = topY - 14.0f;

    if (fireControl_) {
        auto batteries = fireControl_->getAllBatteryData();

        // Column headers
        const char* cols[] = { "UNIT", "MSL", "STS", "TGT" };
        float colX[] = { 0.0f, 60.0f, 90.0f, 120.0f };
        cocos2d::Color4B dimRed(180, 40, 40, 200);

        for (int c = 0; c < 4; c++) {
            auto* colLabel = cocos2d::Label::createWithSystemFont(
                cols[c], "Courier", 7);
            colLabel->setPosition(cocos2d::Vec2(textX + colX[c], rowY));
            colLabel->setAnchorPoint(cocos2d::Vec2(0, 1));
            colLabel->setTextColor(dimRed);
            dataLabelNode_->addChild(colLabel);
        }

        // Separator
        dataPanelNode_->drawLine(
            cocos2d::Vec2(textX, rowY - 4),
            cocos2d::Vec2(panelX + pw - 12, rowY - 4),
            cocos2d::Color4F(0.47f, 0.12f, 0.12f, 0.6f));

        rowY -= 8.0f;

        for (int i = 0; i < (int)batteries.size(); i++) {
            const auto& bat = batteries[i];
            float y = rowY - lineH * (i + 1);

            // Determine status string and color
            const char* statusStr;
            cocos2d::Color4B statusColor;
            switch (bat.status) {
                case BatteryStatus::READY:
                    statusStr = "RDY";
                    statusColor = cocos2d::Color4B(60, 255, 60, 255); break;
                case BatteryStatus::ENGAGED:
                    statusStr = "ENG";
                    statusColor = cocos2d::Color4B(255, 60, 60, 255); break;
                case BatteryStatus::RELOADING:
                    statusStr = "RLD";
                    statusColor = cocos2d::Color4B(255, 180, 60, 255); break;
                case BatteryStatus::OFFLINE:
                    statusStr = "OFF";
                    statusColor = cocos2d::Color4B(120, 120, 120, 200); break;
                case BatteryStatus::DESTROYED:
                    statusStr = "DES";
                    statusColor = cocos2d::Color4B(255, 0, 0, 255); break;
                default:
                    statusStr = "TRK";
                    statusColor = cocos2d::Color4B(255, 255, 60, 255); break;
            }

            cocos2d::Color4B unitColor(220, 80, 60, 240);

            // Designation
            auto* desLabel = cocos2d::Label::createWithSystemFont(
                bat.designation, "Courier", 7);
            desLabel->setPosition(cocos2d::Vec2(textX + colX[0], y));
            desLabel->setAnchorPoint(cocos2d::Vec2(0, 0));
            desLabel->setTextColor(unitColor);
            dataLabelNode_->addChild(desLabel);

            // Missiles
            char mslBuf[8];
            snprintf(mslBuf, sizeof(mslBuf), "%d/%d",
                     bat.missilesRemaining, bat.maxMissiles);
            auto* mslLabel = cocos2d::Label::createWithSystemFont(
                mslBuf, "Courier", 7);
            mslLabel->setPosition(cocos2d::Vec2(textX + colX[1], y));
            mslLabel->setAnchorPoint(cocos2d::Vec2(0, 0));
            mslLabel->setTextColor(unitColor);
            dataLabelNode_->addChild(mslLabel);

            // Status
            auto* stsLabel = cocos2d::Label::createWithSystemFont(
                statusStr, "Courier", 7);
            stsLabel->setPosition(cocos2d::Vec2(textX + colX[2], y));
            stsLabel->setAnchorPoint(cocos2d::Vec2(0, 0));
            stsLabel->setTextColor(statusColor);
            dataLabelNode_->addChild(stsLabel);

            // Target
            std::string tgt = bat.assignedTrackId >= 0
                ? "TK-" + std::to_string(bat.assignedTrackId)
                : "---";
            auto* tgtLabel = cocos2d::Label::createWithSystemFont(
                tgt, "Courier", 7);
            tgtLabel->setPosition(cocos2d::Vec2(textX + colX[3], y));
            tgtLabel->setAnchorPoint(cocos2d::Vec2(0, 0));
            tgtLabel->setTextColor(unitColor);
            dataLabelNode_->addChild(tgtLabel);
        }

        rowY -= lineH * ((int)batteries.size() + 2) + 8.0f;
    }

    // HQ / Threat section
    dataPanelNode_->drawLine(
        cocos2d::Vec2(textX, rowY + lineH),
        cocos2d::Vec2(panelX + pw - 12, rowY + lineH),
        cocos2d::Color4F(0.47f, 0.12f, 0.12f, 0.4f));

    auto* hqHeader = cocos2d::Label::createWithSystemFont(
        "HQ / THREAT", "Courier", 9);
    hqHeader->setPosition(cocos2d::Vec2(textX, rowY));
    hqHeader->setAnchorPoint(cocos2d::Vec2(0, 0));
    hqHeader->setTextColor(cocos2d::Color4B(255, 60, 60, 255));
    dataLabelNode_->addChild(hqHeader);

    rowY -= lineH + 4.0f;

    if (battalionHQ_) {
        auto hqData = battalionHQ_->getData();
        cocos2d::Color4B hqColor = hqData.radarOnline
            ? cocos2d::Color4B(60, 255, 60, 230)
            : cocos2d::Color4B(255, 60, 60, 230);

        char hqBuf[64];
        snprintf(hqBuf, sizeof(hqBuf), "HQ:%s", hqData.statusString.c_str());
        auto* hqLabel = cocos2d::Label::createWithSystemFont(hqBuf, "Courier", 7);
        hqLabel->setPosition(cocos2d::Vec2(textX, rowY));
        hqLabel->setAnchorPoint(cocos2d::Vec2(0, 0));
        hqLabel->setTextColor(hqColor);
        dataLabelNode_->addChild(hqLabel);

        rowY -= lineH;

        char sysBuf[48];
        snprintf(sysBuf, sizeof(sysBuf), "RDR:%s COM:%s",
                 hqData.radarOnline ? "ON" : "OFF",
                 hqData.commsOnline ? "ON" : "OFF");
        auto* sysLabel = cocos2d::Label::createWithSystemFont(sysBuf, "Courier", 7);
        sysLabel->setPosition(cocos2d::Vec2(textX, rowY));
        sysLabel->setAnchorPoint(cocos2d::Vec2(0, 0));
        sysLabel->setTextColor(hqColor);
        dataLabelNode_->addChild(sysLabel);

        rowY -= lineH;
    }

    // Threat count
    if (threatBoard_) {
        int threatCount = threatBoard_->getThreatCount();
        cocos2d::Color4B threatColor = threatCount > 0
            ? cocos2d::Color4B(255, 60, 60, 255)
            : cocos2d::Color4B(60, 255, 60, 200);

        char thrBuf[32];
        snprintf(thrBuf, sizeof(thrBuf), "THREATS: %d", threatCount);
        auto* thrLabel = cocos2d::Label::createWithSystemFont(thrBuf, "Courier", 8);
        thrLabel->setPosition(cocos2d::Vec2(textX, rowY));
        thrLabel->setAnchorPoint(cocos2d::Vec2(0, 0));
        thrLabel->setTextColor(threatColor);
        dataLabelNode_->addChild(thrLabel);
    }
}

// ============================================================================
// Bottom bar: Score, level, latest message
// ============================================================================

void ConsoleFrame::drawBottomBar()
{
    float r = scopeRadius_;
    float pw = panelWidth_;

    float barLeft  = -r - pw - 16.0f;
    float barRight =  r + pw + 16.0f;
    float barTop   = -r - 82.0f;
    float barBot   = -r - 110.0f;

    // Background
    dataPanelNode_->drawSolidRect(
        cocos2d::Vec2(barLeft, barBot),
        cocos2d::Vec2(barRight, barTop),
        cocos2d::Color4F(0.06f, 0.02f, 0.02f, 0.9f));

    dataPanelNode_->drawRect(
        cocos2d::Vec2(barLeft, barBot),
        cocos2d::Vec2(barRight, barTop),
        cocos2d::Color4F(0.16f, 0.06f, 0.06f, 0.6f));

    float midY = (barTop + barBot) * 0.5f;

    // Score
    char scoreBuf[32];
    snprintf(scoreBuf, sizeof(scoreBuf), "SCORE: %d", score_);
    auto* scoreLabel = cocos2d::Label::createWithSystemFont(
        scoreBuf, "Courier", 10);
    scoreLabel->setPosition(cocos2d::Vec2(barLeft + 12, midY));
    scoreLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
    scoreLabel->setTextColor(score_ < 0
        ? cocos2d::Color4B(255, 80, 80, 255)
        : cocos2d::Color4B(255, 100, 60, 255));
    dataLabelNode_->addChild(scoreLabel);

    // Level
    char lvlBuf[16];
    snprintf(lvlBuf, sizeof(lvlBuf), "LV:%d", level_);
    auto* lvlLabel = cocos2d::Label::createWithSystemFont(
        lvlBuf, "Courier", 10);
    lvlLabel->setPosition(cocos2d::Vec2(barLeft + 120, midY));
    lvlLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
    lvlLabel->setTextColor(cocos2d::Color4B(255, 180, 60, 255));
    dataLabelNode_->addChild(lvlLabel);

    // Latest message
    if (!messages_.empty()) {
        std::string msg = messages_.back().substr(0, 55);
        auto* msgLabel = cocos2d::Label::createWithSystemFont(
            msg, "Courier", 8);
        msgLabel->setPosition(cocos2d::Vec2(barLeft + 200, midY));
        msgLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
        msgLabel->setTextColor(cocos2d::Color4B(0, 220, 0, 200));
        dataLabelNode_->addChild(msgLabel);
    }
}

#else
// ============================================================================
// Stub ConsoleFrame (no cocos2d-x)
// ============================================================================

ConsoleFrame* ConsoleFrame::create(float scopeRadius, float panelWidth)
{
    auto* frame = new ConsoleFrame();
    if (frame->init(scopeRadius, panelWidth)) return frame;
    delete frame;
    return nullptr;
}

bool ConsoleFrame::init(float scopeRadius, float panelWidth)
{
    scopeRadius_ = scopeRadius;
    panelWidth_ = panelWidth;
    return true;
}

void ConsoleFrame::update(float dt) {}

void ConsoleFrame::addMessage(const std::string& msg)
{
    messages_.push_back(msg);
    if (messages_.size() > 6) {
        messages_.erase(messages_.begin());
    }
}
#endif
