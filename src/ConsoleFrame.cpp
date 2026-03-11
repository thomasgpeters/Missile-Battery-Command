#include "ConsoleFrame.h"
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <random>

#if USE_COCOS2DX
// ============================================================================
// Cocos2d-x ConsoleFrame — AN/TSQ-73 Analog Console
// ============================================================================

ConsoleFrame* ConsoleFrame::create(float scopeRadius)
{
    auto* ret = new (std::nothrow) ConsoleFrame();
    if (ret && ret->init(scopeRadius)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ConsoleFrame::init(float scopeRadius)
{
    if (!Node::init()) return false;

    scopeRadius_ = scopeRadius;
    trackManager_ = nullptr;
    fireControl_ = nullptr;
    threatBoard_ = nullptr;
    battalionHQ_ = nullptr;
    score_ = 0;
    level_ = 1;
    selectedTrackId_ = -1;

    // === AN/TSQ-73 layout: central portrait display + flanking control panels ===
    //
    // The real console has a portrait-format CRT display in the center with
    // rounded corners, flanked by tall narrow control panels on each side.
    // The PPI radar scope is rendered inside the portrait display.

    // Portrait display: sized to frame the circular scope with margin
    float scopeDia = scopeRadius * 2.0f;
    displayW_ = scopeDia + 50.0f;              // scope + small side margins
    displayH_ = displayW_ * 1.35f;             // portrait: 35% taller than wide
    displayCornerR_ = 55.0f;                    // large ~5-inch rounded corners on CRT
    displayCenterY_ = 0.0f;                     // scope centered in display

    // Side panels: wider for authentic control panel spacing
    panelW_ = 130.0f;
    panelH_ = displayH_ + 40.0f;               // slightly taller than display
    panelGap_ = 12.0f;                          // gap between panel and display

    // Overall console extents (wider for bezel room and control panels)
    bezelW_ = displayW_ + 2.0f * (panelGap_ + panelW_) + 40.0f;
    bezelH_ = panelH_ + 90.0f;                 // panels + top indicators + bottom bar

    // Create draw layers
    housingNode_ = cocos2d::DrawNode::create();
    displayNode_ = cocos2d::DrawNode::create();
    panelNode_   = cocos2d::DrawNode::create();
    buttonNode_  = cocos2d::DrawNode::create();
    dynamicNode_ = cocos2d::DrawNode::create();
    labelNode_   = cocos2d::Node::create();

    addChild(housingNode_,  0);
    addChild(displayNode_,  1);
    addChild(panelNode_,    2);
    addChild(buttonNode_,   3);
    addChild(dynamicNode_,  4);
    addChild(labelNode_,    5);

    // Draw all static elements once
    drawShelterBackground();
    drawChassis();
    drawPatina();
    drawPortraitDisplay();
    drawScopeRing();
    drawLeftPanel();
    drawRightPanel();
    drawBottomControls();
    drawTopIndicatorRow();
    drawManufacturerPlate();

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
    dynamicNode_->clear();
    labelNode_->removeAllChildren();

    drawDynamicIndicators();
    drawBottomBar();
}

// ============================================================================
// Rounded rectangle helper
// ============================================================================

void ConsoleFrame::drawRoundedRect(cocos2d::DrawNode* node,
                                   const cocos2d::Vec2& origin,
                                   const cocos2d::Vec2& dest,
                                   float radius,
                                   const cocos2d::Color4F& fill,
                                   const cocos2d::Color4F& border)
{
    float left   = std::min(origin.x, dest.x);
    float right  = std::max(origin.x, dest.x);
    float bottom = std::min(origin.y, dest.y);
    float top    = std::max(origin.y, dest.y);

    float r = std::min(radius, std::min((right - left) * 0.5f, (top - bottom) * 0.5f));

    // Build polygon with rounded corners
    const int arcSteps = 6;
    std::vector<cocos2d::Vec2> verts;

    // Bottom-left corner
    for (int i = 0; i <= arcSteps; i++) {
        float angle = M_PI + (M_PI * 0.5f) * ((float)i / arcSteps);
        verts.push_back(cocos2d::Vec2(left + r + r * cosf(angle),
                                       bottom + r + r * sinf(angle)));
    }
    // Bottom-right corner
    for (int i = 0; i <= arcSteps; i++) {
        float angle = M_PI * 1.5f + (M_PI * 0.5f) * ((float)i / arcSteps);
        verts.push_back(cocos2d::Vec2(right - r + r * cosf(angle),
                                       bottom + r + r * sinf(angle)));
    }
    // Top-right corner
    for (int i = 0; i <= arcSteps; i++) {
        float angle = 0.0f + (M_PI * 0.5f) * ((float)i / arcSteps);
        verts.push_back(cocos2d::Vec2(right - r + r * cosf(angle),
                                       top - r + r * sinf(angle)));
    }
    // Top-left corner
    for (int i = 0; i <= arcSteps; i++) {
        float angle = M_PI * 0.5f + (M_PI * 0.5f) * ((float)i / arcSteps);
        verts.push_back(cocos2d::Vec2(left + r + r * cosf(angle),
                                       top - r + r * sinf(angle)));
    }

    node->drawPolygon(verts.data(), (int)verts.size(), fill, 1.0f, border);
}

// ============================================================================
// Octagon helper — rectangle with 45° chamfered corners
// ============================================================================

void ConsoleFrame::drawOctagon(cocos2d::DrawNode* node,
                                const cocos2d::Vec2& origin,
                                const cocos2d::Vec2& dest,
                                float chamfer,
                                const cocos2d::Color4F& fill,
                                const cocos2d::Color4F& border)
{
    float left   = std::min(origin.x, dest.x);
    float right  = std::max(origin.x, dest.x);
    float bottom = std::min(origin.y, dest.y);
    float top    = std::max(origin.y, dest.y);

    float c = std::min(chamfer, std::min((right - left) * 0.4f, (top - bottom) * 0.4f));

    // 8 vertices, going clockwise from bottom-left chamfer
    cocos2d::Vec2 verts[] = {
        { left,         bottom + c },    // left side, bottom chamfer end
        { left + c,     bottom },        // bottom side, left chamfer end
        { right - c,    bottom },        // bottom side, right chamfer end
        { right,        bottom + c },    // right side, bottom chamfer end
        { right,        top - c },       // right side, top chamfer end
        { right - c,    top },           // top side, right chamfer end
        { left + c,     top },           // top side, left chamfer end
        { left,         top - c }        // left side, top chamfer end
    };

    node->drawPolygon(verts, 8, fill, 1.5f, border);
}

// ============================================================================
// Button drawing helpers
// ============================================================================

void ConsoleFrame::drawButtonRow(cocos2d::DrawNode* node,
                                  float x, float y, int count,
                                  float btnW, float btnH, float gap,
                                  const cocos2d::Color4F& btnColor,
                                  const cocos2d::Color4F& litColor,
                                  bool vertical)
{
    for (int i = 0; i < count; i++) {
        float bx, by;
        if (vertical) {
            bx = x;
            by = y - i * (btnH + gap);
        } else {
            bx = x + i * (btnW + gap);
            by = y;
        }

        // Button housing (recessed)
        node->drawSolidRect(
            cocos2d::Vec2(bx - 1, by - 1),
            cocos2d::Vec2(bx + btnW + 1, by + btnH + 1),
            cocos2d::Color4F(0.08f, 0.08f, 0.06f, 1.0f));

        // Button face
        node->drawSolidRect(
            cocos2d::Vec2(bx, by),
            cocos2d::Vec2(bx + btnW, by + btnH),
            btnColor);

        // Lit indicator on button (small illuminated window)
        float indW = btnW * 0.6f;
        float indH = btnH * 0.3f;
        float indX = bx + (btnW - indW) * 0.5f;
        float indY = by + btnH * 0.55f;
        node->drawSolidRect(
            cocos2d::Vec2(indX, indY),
            cocos2d::Vec2(indX + indW, indY + indH),
            litColor);

        // Top bevel highlight
        node->drawLine(
            cocos2d::Vec2(bx + 1, by + btnH),
            cocos2d::Vec2(bx + btnW - 1, by + btnH),
            cocos2d::Color4F(btnColor.r + 0.08f, btnColor.g + 0.08f,
                             btnColor.b + 0.06f, 1.0f));
    }
}

void ConsoleFrame::drawLabeledButton(cocos2d::DrawNode* node,
                                      float cx, float cy,
                                      float w, float h,
                                      const cocos2d::Color4F& color,
                                      const char* label,
                                      float fontSize)
{
    float x0 = cx - w * 0.5f;
    float y0 = cy - h * 0.5f;

    // Recessed housing
    node->drawSolidRect(
        cocos2d::Vec2(x0 - 1, y0 - 1),
        cocos2d::Vec2(x0 + w + 1, y0 + h + 1),
        cocos2d::Color4F(0.06f, 0.06f, 0.05f, 1.0f));

    // Button face
    node->drawSolidRect(
        cocos2d::Vec2(x0, y0),
        cocos2d::Vec2(x0 + w, y0 + h),
        color);

    // Label
    if (label && label[0]) {
        auto* lbl = cocos2d::Label::createWithSystemFont(label, "Courier", fontSize);
        lbl->setPosition(cocos2d::Vec2(cx, cy));
        lbl->setTextColor(cocos2d::Color4B(20, 20, 18, 220));
        node->addChild(lbl);
    }
}

// ============================================================================
// S-280 shelter background
// ============================================================================

void ConsoleFrame::drawShelterBackground()
{
    housingNode_->drawSolidRect(
        cocos2d::Vec2(-800, -600),
        cocos2d::Vec2(800, 600),
        cocos2d::Color4F(0.06f, 0.07f, 0.05f, 1.0f));

    // Ceiling
    housingNode_->drawSolidRect(
        cocos2d::Vec2(-800, 400),
        cocos2d::Vec2(800, 600),
        cocos2d::Color4F(0.08f, 0.09f, 0.07f, 1.0f));

    // Red night ops light strips
    for (int lx = -400; lx <= 400; lx += 200) {
        housingNode_->drawSolidRect(
            cocos2d::Vec2((float)lx - 20.0f, 520.0f),
            cocos2d::Vec2((float)lx + 20.0f, 524.0f),
            cocos2d::Color4F(0.25f, 0.06f, 0.06f, 0.5f));
        housingNode_->drawSolidRect(
            cocos2d::Vec2((float)lx - 35.0f, 500.0f),
            cocos2d::Vec2((float)lx + 35.0f, 520.0f),
            cocos2d::Color4F(0.15f, 0.04f, 0.04f, 0.2f));
    }

    // Floor
    housingNode_->drawSolidRect(
        cocos2d::Vec2(-800, -600),
        cocos2d::Vec2(800, -450),
        cocos2d::Color4F(0.04f, 0.05f, 0.03f, 1.0f));
}

// ============================================================================
// Chassis — the console housing that holds display + panels
// ============================================================================

void ConsoleFrame::drawChassis()
{
    float hw = bezelW_ * 0.5f;
    float hh = bezelH_ * 0.5f;

    // Outer chassis — AN/TSQ-73 seafoam green painted metal (FS 24491)
    cocos2d::Color4F chassisColor(0.52f, 0.68f, 0.60f, 1.0f);
    cocos2d::Color4F chassisBorder(0.38f, 0.52f, 0.45f, 1.0f);

    drawRoundedRect(housingNode_,
                    cocos2d::Vec2(-hw, -hh),
                    cocos2d::Vec2(hw, hh),
                    12.0f,
                    chassisColor, chassisBorder);

    // Inner recessed work surface (slightly darker seafoam)
    float inset = 8.0f;
    cocos2d::Color4F recessColor(0.42f, 0.56f, 0.49f, 1.0f);
    cocos2d::Color4F recessBorder(0.32f, 0.44f, 0.38f, 1.0f);

    drawRoundedRect(housingNode_,
                    cocos2d::Vec2(-hw + inset, -hh + inset),
                    cocos2d::Vec2(hw - inset, hh - inset),
                    10.0f,
                    recessColor, recessBorder);

    // Top highlight bevel (seafoam lighter)
    housingNode_->drawLine(
        cocos2d::Vec2(-hw + 12, hh - 1),
        cocos2d::Vec2(hw - 12, hh - 1),
        cocos2d::Color4F(0.62f, 0.76f, 0.68f, 0.6f));
    // Bottom shadow bevel
    housingNode_->drawLine(
        cocos2d::Vec2(-hw + 12, -hh + 1),
        cocos2d::Vec2(hw - 12, -hh + 1),
        cocos2d::Color4F(0.28f, 0.38f, 0.32f, 0.8f));
}

// ============================================================================
// Patina — weathering, scuffs, and paint wear on the chassis
// Gives the console a battle-hardened, field-deployed appearance
// ============================================================================

void ConsoleFrame::drawPatina()
{
    float hw = bezelW_ * 0.5f;
    float hh = bezelH_ * 0.5f;

    // Use a deterministic seed for consistent weathering pattern
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> xDist(-hw + 15, hw - 15);
    std::uniform_real_distribution<float> yDist(-hh + 15, hh - 15);
    std::uniform_real_distribution<float> sizeDist(2.0f, 12.0f);
    std::uniform_real_distribution<float> alphaDist(0.03f, 0.09f);
    std::uniform_real_distribution<float> angleDist(0.0f, 6.28f);
    std::uniform_real_distribution<float> lenDist(8.0f, 35.0f);

    // --- Paint wear spots (darker patches where paint has worn thin) ---
    for (int i = 0; i < 25; i++) {
        float x = xDist(rng);
        float y = yDist(rng);
        float size = sizeDist(rng);
        float alpha = alphaDist(rng);

        housingNode_->drawSolidCircle(
            cocos2d::Vec2(x, y), size, 0, 8,
            cocos2d::Color4F(0.0f, 0.0f, 0.0f, alpha));
    }

    // --- Lighter oxidation patches (faded seafoam green spots) ---
    for (int i = 0; i < 15; i++) {
        float x = xDist(rng);
        float y = yDist(rng);
        float size = sizeDist(rng) * 1.5f;
        float alpha = alphaDist(rng) * 0.7f;

        housingNode_->drawSolidCircle(
            cocos2d::Vec2(x, y), size, 0, 8,
            cocos2d::Color4F(0.65f, 0.75f, 0.68f, alpha));
    }

    // --- Scratch marks (thin lines from equipment handling) ---
    for (int i = 0; i < 12; i++) {
        float x = xDist(rng);
        float y = yDist(rng);
        float angle = angleDist(rng);
        float len = lenDist(rng);
        float alpha = alphaDist(rng) * 0.8f;

        cocos2d::Vec2 start(x, y);
        cocos2d::Vec2 end(x + cosf(angle) * len, y + sinf(angle) * len);

        housingNode_->drawLine(start, end,
            cocos2d::Color4F(0.30f, 0.38f, 0.32f, alpha));
    }

    // --- Edge wear (brighter metal showing through at corners and edges) ---
    // Top edge wear
    for (int i = 0; i < 8; i++) {
        float x = xDist(rng);
        float len = sizeDist(rng) * 2.0f;
        float alpha = alphaDist(rng) * 0.6f;
        housingNode_->drawLine(
            cocos2d::Vec2(x, hh - 4), cocos2d::Vec2(x + len, hh - 5),
            cocos2d::Color4F(0.55f, 0.60f, 0.53f, alpha));
    }
    // Bottom edge wear
    for (int i = 0; i < 6; i++) {
        float x = xDist(rng);
        float len = sizeDist(rng) * 2.0f;
        float alpha = alphaDist(rng) * 0.6f;
        housingNode_->drawLine(
            cocos2d::Vec2(x, -hh + 4), cocos2d::Vec2(x + len, -hh + 5),
            cocos2d::Color4F(0.55f, 0.60f, 0.53f, alpha));
    }

    // --- Subtle dust/grime accumulation in recessed areas ---
    float inset = 10.0f;
    housingNode_->drawLine(
        cocos2d::Vec2(-hw + inset, -hh + inset + 1),
        cocos2d::Vec2(hw - inset, -hh + inset + 1),
        cocos2d::Color4F(0.0f, 0.0f, 0.0f, 0.06f));
    housingNode_->drawLine(
        cocos2d::Vec2(-hw + inset, -hh + inset + 2),
        cocos2d::Vec2(hw - inset, -hh + inset + 2),
        cocos2d::Color4F(0.0f, 0.0f, 0.0f, 0.04f));
}

// ============================================================================
// Portrait display — rounded-corner CRT viewport containing the PPI scope
// ============================================================================

void ConsoleFrame::drawPortraitDisplay()
{
    float hdw = displayW_ * 0.5f;
    float hdh = displayH_ * 0.5f;
    float cy = displayCenterY_;

    // === Octagonal outer bezel — seafoam green to match console housing ===
    float bezelThick = 18.0f;
    float chamfer = 45.0f;  // 45° cut on each corner of the octagon

    // Outer octagon shadow/border (slightly darker)
    drawOctagon(displayNode_,
                cocos2d::Vec2(-hdw - bezelThick - 2, cy - hdh - bezelThick - 2),
                cocos2d::Vec2(hdw + bezelThick + 2, cy + hdh + bezelThick + 2),
                chamfer + 4.0f,
                cocos2d::Color4F(0.32f, 0.42f, 0.36f, 1.0f),
                cocos2d::Color4F(0.25f, 0.34f, 0.28f, 1.0f));

    // Main octagonal bezel face — seafoam green painted cast metal
    drawOctagon(displayNode_,
                cocos2d::Vec2(-hdw - bezelThick, cy - hdh - bezelThick),
                cocos2d::Vec2(hdw + bezelThick, cy + hdh + bezelThick),
                chamfer,
                cocos2d::Color4F(0.48f, 0.63f, 0.55f, 1.0f),
                cocos2d::Color4F(0.36f, 0.48f, 0.40f, 1.0f));

    // Inner lip / bevel ring around CRT opening (darker edge)
    drawRoundedRect(displayNode_,
                    cocos2d::Vec2(-hdw - 3, cy - hdh - 3),
                    cocos2d::Vec2(hdw + 3, cy + hdh + 3),
                    displayCornerR_ + 4.0f,
                    cocos2d::Color4F(0.12f, 0.14f, 0.11f, 1.0f),
                    cocos2d::Color4F(0.20f, 0.26f, 0.22f, 1.0f));

    // === CRT glass surface — portrait rounded rectangle with large radius ===
    drawRoundedRect(displayNode_,
                    cocos2d::Vec2(-hdw, cy - hdh),
                    cocos2d::Vec2(hdw, cy + hdh),
                    displayCornerR_,
                    cocos2d::Color4F(0.02f, 0.04f, 0.02f, 1.0f),
                    cocos2d::Color4F(0.06f, 0.08f, 0.05f, 1.0f));

    // Subtle CRT glass reflection (top-left highlight arc)
    displayNode_->drawLine(
        cocos2d::Vec2(-hdw + displayCornerR_ + 10, cy + hdh - 10),
        cocos2d::Vec2(-hdw + 10, cy + hdh - displayCornerR_ - 10),
        cocos2d::Color4F(0.12f, 0.16f, 0.10f, 0.25f));

    // === Mounting screws on the octagonal bezel flat faces ===
    // Place screws on the 4 cardinal flat faces of the octagon
    float screwDist = 12.0f;  // distance from inner edge of bezel
    cocos2d::Vec2 screwPositions[] = {
        // Top face (2 screws)
        { -hdw * 0.35f, cy + hdh + screwDist },
        {  hdw * 0.35f, cy + hdh + screwDist },
        // Bottom face (2 screws)
        { -hdw * 0.35f, cy - hdh - screwDist },
        {  hdw * 0.35f, cy - hdh - screwDist },
        // Left face (2 screws)
        { -hdw - screwDist, cy + hdh * 0.25f },
        { -hdw - screwDist, cy - hdh * 0.25f },
        // Right face (2 screws)
        {  hdw + screwDist, cy + hdh * 0.25f },
        {  hdw + screwDist, cy - hdh * 0.25f },
    };
    for (const auto& sp : screwPositions) {
        // Screw recess
        displayNode_->drawSolidCircle(sp, 3.5f, 0, 10,
            cocos2d::Color4F(0.30f, 0.38f, 0.32f, 1.0f));
        // Screw head
        displayNode_->drawSolidCircle(sp, 2.5f, 0, 10,
            cocos2d::Color4F(0.42f, 0.50f, 0.44f, 1.0f));
        displayNode_->drawCircle(sp, 3.5f, 0, 10, false,
            cocos2d::Color4F(0.25f, 0.32f, 0.27f, 1.0f));
        // Phillips cross
        displayNode_->drawLine(sp + cocos2d::Vec2(-1.5f, 0), sp + cocos2d::Vec2(1.5f, 0),
            cocos2d::Color4F(0.28f, 0.34f, 0.30f, 1.0f));
        displayNode_->drawLine(sp + cocos2d::Vec2(0, -1.5f), sp + cocos2d::Vec2(0, 1.5f),
            cocos2d::Color4F(0.28f, 0.34f, 0.30f, 1.0f));
    }

    // Bezel highlight (top face of octagon catches light)
    float topFaceLeft = -hdw + chamfer * 0.3f;
    float topFaceRight = hdw - chamfer * 0.3f;
    float topFaceY = cy + hdh + bezelThick - 2;
    displayNode_->drawLine(
        cocos2d::Vec2(topFaceLeft, topFaceY),
        cocos2d::Vec2(topFaceRight, topFaceY),
        cocos2d::Color4F(0.58f, 0.72f, 0.62f, 0.4f));
}

// ============================================================================
// Scope ring — rubber hood and metal bezel around PPI (centered in display)
// ============================================================================

void ConsoleFrame::drawScopeRing()
{
    float r = scopeRadius_;
    float cy = displayCenterY_;  // scope centered in portrait display

    // Rubber hood ring (dark gray, outermost ring)
    displayNode_->drawCircle(
        cocos2d::Vec2(0, cy), r + 8.0f, 0, 72, false,
        cocos2d::Color4F(0.12f, 0.12f, 0.10f, 1.0f));
    displayNode_->drawCircle(
        cocos2d::Vec2(0, cy), r + 5.0f, 0, 72, false,
        cocos2d::Color4F(0.10f, 0.10f, 0.08f, 1.0f));

    // Inner metal bezel ring
    displayNode_->drawCircle(
        cocos2d::Vec2(0, cy), r + 2.0f, 0, 72, false,
        cocos2d::Color4F(0.45f, 0.43f, 0.38f, 1.0f));
    displayNode_->drawCircle(
        cocos2d::Vec2(0, cy), r + 1.0f, 0, 72, false,
        cocos2d::Color4F(0.35f, 0.33f, 0.28f, 1.0f));

    // Range ring etchings on the CRT glass (below and above scope area)
    // These are the faint markings visible on the portrait display outside the scope circle
    float hdh = displayH_ * 0.5f;

    // Azimuth scale marks along the scope circumference
    for (int deg = 0; deg < 360; deg += 10) {
        float angle = deg * M_PI / 180.0f;
        float innerR = r + 2.0f;
        float outerR = r + ((deg % 30 == 0) ? 10.0f : 5.0f);
        cocos2d::Vec2 inner(sinf(angle) * innerR, cy + cosf(angle) * innerR);
        cocos2d::Vec2 outer(sinf(angle) * outerR, cy + cosf(angle) * outerR);
        float alpha = (deg % 30 == 0) ? 0.35f : 0.18f;
        displayNode_->drawLine(inner, outer,
            cocos2d::Color4F(0.0f, 0.4f, 0.0f, alpha));
    }

    // Cardinal direction labels on the display glass
    const char* cardinals[] = { "N", "E", "S", "W" };
    float cardAngles[] = { 0, 90, 180, 270 };
    for (int i = 0; i < 4; i++) {
        float angle = cardAngles[i] * M_PI / 180.0f;
        float labelR = r + 16.0f;
        cocos2d::Vec2 pos(sinf(angle) * labelR, cy + cosf(angle) * labelR);
        auto* lbl = cocos2d::Label::createWithSystemFont(cardinals[i], "Courier", 7);
        lbl->setPosition(pos);
        lbl->setTextColor(cocos2d::Color4B(0, 120, 0, 140));
        displayNode_->addChild(lbl);
    }
}

// ============================================================================
// Left panel — tall narrow control panel (track management, keypad)
// Rendered separately from the display, to the left
// ============================================================================

void ConsoleFrame::drawLeftPanel()
{
    float panelCX = -(displayW_ * 0.5f + panelGap_ + panelW_ * 0.5f);
    float hpw = panelW_ * 0.5f;
    float hph = panelH_ * 0.5f;

    // Panel housing — seafoam green to match chassis, worn/weathered
    drawRoundedRect(panelNode_,
                    cocos2d::Vec2(panelCX - hpw, -hph),
                    cocos2d::Vec2(panelCX + hpw, hph),
                    6.0f,
                    cocos2d::Color4F(0.40f, 0.52f, 0.44f, 1.0f),
                    cocos2d::Color4F(0.30f, 0.40f, 0.34f, 1.0f));

    // Inner recess (dark recessed panel area)
    float inset = 6.0f;
    panelNode_->drawSolidRect(
        cocos2d::Vec2(panelCX - hpw + inset, -hph + inset),
        cocos2d::Vec2(panelCX + hpw - inset, hph - inset),
        cocos2d::Color4F(0.14f, 0.16f, 0.13f, 1.0f));

    // --- Layout buttons top to bottom ---
    // Larger buttons with generous spacing for gloved-hand operation
    float btnW = 22.0f;
    float btnH = 16.0f;
    float gap = 6.0f;
    float padding = 10.0f;
    float pLeft = panelCX - hpw + inset + padding;
    float usableW = panelW_ - 2.0f * inset - 2.0f * padding;
    int cols = std::max(1, (int)(usableW / (btnW + gap)));
    float startX = pLeft + (usableW - cols * (btnW + gap) + gap) * 0.5f;

    cocos2d::Color4F btnFace(0.62f, 0.60f, 0.52f, 1.0f);
    cocos2d::Color4F dimLit(0.15f, 0.30f, 0.10f, 0.4f);
    cocos2d::Color4F amberLit(0.50f, 0.35f, 0.05f, 0.5f);

    // Section label
    auto* trkLabel = cocos2d::Label::createWithSystemFont("TRACK MGMT", "Courier", 6);
    trkLabel->setPosition(cocos2d::Vec2(panelCX, hph - 16));
    trkLabel->setTextColor(cocos2d::Color4B(180, 190, 170, 200));
    panelNode_->addChild(trkLabel);

    // Upper illuminated button array: 7 rows
    float rowTop = hph - 28.0f;
    for (int row = 0; row < 7; row++) {
        float rowY = rowTop - row * (btnH + gap + 3.0f);
        cocos2d::Color4F litColor = (row < 2) ? amberLit : dimLit;
        drawButtonRow(buttonNode_, startX, rowY, cols, btnW, btnH, gap,
                      btnFace, litColor);
    }

    // --- Numeric keypad ---
    float keypadTop = rowTop - 7 * (btnH + gap + 3.0f) - 14.0f;
    float keyW = 20.0f;
    float keyH = 18.0f;
    float keyGap = 5.0f;

    float kpLeft = panelCX - (3 * (keyW + keyGap) - keyGap) * 0.5f;

    // Keypad background with border
    buttonNode_->drawSolidRect(
        cocos2d::Vec2(kpLeft - 6, keypadTop - 4 * (keyH + keyGap) - 6),
        cocos2d::Vec2(kpLeft + 3 * (keyW + keyGap) + 6, keypadTop + 6),
        cocos2d::Color4F(0.10f, 0.12f, 0.09f, 1.0f));
    buttonNode_->drawRect(
        cocos2d::Vec2(kpLeft - 6, keypadTop - 4 * (keyH + keyGap) - 6),
        cocos2d::Vec2(kpLeft + 3 * (keyW + keyGap) + 6, keypadTop + 6),
        cocos2d::Color4F(0.28f, 0.36f, 0.30f, 0.6f));

    const char* keyLabels[] = {
        "7", "8", "9",
        "4", "5", "6",
        "1", "2", "3",
        "CLR", "0", "ENT"
    };
    cocos2d::Color4F keyColor(0.58f, 0.56f, 0.50f, 1.0f);
    cocos2d::Color4F entColor(0.40f, 0.52f, 0.38f, 1.0f);
    cocos2d::Color4F clrColor(0.52f, 0.38f, 0.34f, 1.0f);

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 3; col++) {
            int idx = row * 3 + col;
            float kx = kpLeft + col * (keyW + keyGap) + keyW * 0.5f;
            float ky = keypadTop - row * (keyH + keyGap) - keyH * 0.5f;

            cocos2d::Color4F kc = keyColor;
            if (idx == 9) kc = clrColor;
            if (idx == 11) kc = entColor;

            drawLabeledButton(buttonNode_, kx, ky, keyW, keyH, kc, keyLabels[idx], 6.0f);
        }
    }

    // --- Lower function buttons ---
    float funcTop = keypadTop - 4 * (keyH + keyGap) - 18.0f;
    cocos2d::Color4F funcFace(0.58f, 0.56f, 0.50f, 1.0f);
    cocos2d::Color4F funcLit(0.10f, 0.35f, 0.10f, 0.5f);

    auto* dispLabel = cocos2d::Label::createWithSystemFont("DISPLAY", "Courier", 6);
    dispLabel->setPosition(cocos2d::Vec2(panelCX, funcTop + 12));
    dispLabel->setTextColor(cocos2d::Color4B(180, 190, 170, 200));
    panelNode_->addChild(dispLabel);

    for (int row = 0; row < 5; row++) {
        float rowY = funcTop - row * (btnH + gap + 3.0f);
        drawButtonRow(buttonNode_, startX, rowY, cols, btnW, btnH, gap,
                      funcFace, funcLit);
    }
}

// ============================================================================
// Right panel — tall narrow control panel (console settings, rotary knobs)
// Rendered separately from the display, to the right
// ============================================================================

void ConsoleFrame::drawRightPanel()
{
    float panelCX = displayW_ * 0.5f + panelGap_ + panelW_ * 0.5f;
    float hpw = panelW_ * 0.5f;
    float hph = panelH_ * 0.5f;

    // Panel housing — seafoam green to match chassis
    drawRoundedRect(panelNode_,
                    cocos2d::Vec2(panelCX - hpw, -hph),
                    cocos2d::Vec2(panelCX + hpw, hph),
                    6.0f,
                    cocos2d::Color4F(0.40f, 0.52f, 0.44f, 1.0f),
                    cocos2d::Color4F(0.30f, 0.40f, 0.34f, 1.0f));

    // Inner recess
    float inset = 6.0f;
    panelNode_->drawSolidRect(
        cocos2d::Vec2(panelCX - hpw + inset, -hph + inset),
        cocos2d::Vec2(panelCX + hpw - inset, hph - inset),
        cocos2d::Color4F(0.14f, 0.16f, 0.13f, 1.0f));

    // Larger buttons with generous spacing
    float btnW = 22.0f;
    float btnH = 16.0f;
    float gap = 6.0f;
    float padding = 10.0f;
    float pLeft = panelCX - hpw + inset + padding;
    float usableW = panelW_ - 2.0f * inset - 2.0f * padding;
    int cols = std::max(1, (int)(usableW / (btnW + gap)));
    float startX = pLeft + (usableW - cols * (btnW + gap) + gap) * 0.5f;

    cocos2d::Color4F btnFace(0.62f, 0.60f, 0.52f, 1.0f);
    cocos2d::Color4F greenLit(0.10f, 0.40f, 0.10f, 0.5f);
    cocos2d::Color4F amberLit(0.50f, 0.35f, 0.05f, 0.5f);

    // Section label
    auto* settLabel = cocos2d::Label::createWithSystemFont("SETTINGS", "Courier", 6);
    settLabel->setPosition(cocos2d::Vec2(panelCX, hph - 16));
    settLabel->setTextColor(cocos2d::Color4B(180, 190, 170, 200));
    panelNode_->addChild(settLabel);

    // Upper button array: 6 rows
    float rowTop = hph - 28.0f;
    for (int row = 0; row < 6; row++) {
        float rowY = rowTop - row * (btnH + gap + 3.0f);
        cocos2d::Color4F litColor = (row == 0) ? amberLit : greenLit;
        drawButtonRow(buttonNode_, startX, rowY, cols, btnW, btnH, gap,
                      btnFace, litColor);
    }

    // --- Rotary controls (knobs) ---
    float knobSectionTop = rowTop - 6 * (btnH + gap + 3.0f) - 16.0f;

    auto* knobLabel = cocos2d::Label::createWithSystemFont("GAIN/VIDEO", "Courier", 5);
    knobLabel->setPosition(cocos2d::Vec2(panelCX, knobSectionTop + 14));
    knobLabel->setTextColor(cocos2d::Color4B(180, 190, 170, 180));
    panelNode_->addChild(knobLabel);

    float knobR = 11.0f;
    float knobGap = 32.0f;
    int numKnobs = 2;
    float knobStartX = panelCX - (numKnobs - 1) * knobGap * 0.5f;

    for (int i = 0; i < numKnobs; i++) {
        cocos2d::Vec2 kc(knobStartX + i * knobGap, knobSectionTop);

        buttonNode_->drawSolidCircle(kc, knobR + 2, 0, 16,
            cocos2d::Color4F(0.14f, 0.14f, 0.12f, 1.0f));
        buttonNode_->drawSolidCircle(kc, knobR, 0, 16,
            cocos2d::Color4F(0.22f, 0.22f, 0.19f, 1.0f));
        buttonNode_->drawSolidCircle(kc, knobR - 3, 0, 12,
            cocos2d::Color4F(0.30f, 0.30f, 0.26f, 1.0f));
        float angle = 0.4f + i * 0.7f;
        buttonNode_->drawLine(
            kc, kc + cocos2d::Vec2(sinf(angle) * (knobR - 2),
                                    cosf(angle) * (knobR - 2)),
            cocos2d::Color4F(0.7f, 0.7f, 0.66f, 1.0f));
        buttonNode_->drawCircle(kc, knobR, 0, 16, false,
            cocos2d::Color4F(0.38f, 0.38f, 0.33f, 1.0f));
    }

    // Second knob row
    float knob2Y = knobSectionTop - knobGap;
    for (int i = 0; i < numKnobs; i++) {
        cocos2d::Vec2 kc(knobStartX + i * knobGap, knob2Y);

        buttonNode_->drawSolidCircle(kc, knobR + 2, 0, 16,
            cocos2d::Color4F(0.14f, 0.14f, 0.12f, 1.0f));
        buttonNode_->drawSolidCircle(kc, knobR, 0, 16,
            cocos2d::Color4F(0.22f, 0.22f, 0.19f, 1.0f));
        buttonNode_->drawSolidCircle(kc, knobR - 3, 0, 12,
            cocos2d::Color4F(0.30f, 0.30f, 0.26f, 1.0f));
        float angle = 1.2f + i * 0.5f;
        buttonNode_->drawLine(
            kc, kc + cocos2d::Vec2(sinf(angle) * (knobR - 2),
                                    cosf(angle) * (knobR - 2)),
            cocos2d::Color4F(0.7f, 0.7f, 0.66f, 1.0f));
        buttonNode_->drawCircle(kc, knobR, 0, 16, false,
            cocos2d::Color4F(0.38f, 0.38f, 0.33f, 1.0f));
    }

    // Lower button rows
    float lowerTop = knob2Y - knobR - 20.0f;

    auto* rngLabel = cocos2d::Label::createWithSystemFont("RANGE/MTI", "Courier", 5);
    rngLabel->setPosition(cocos2d::Vec2(panelCX, lowerTop + 12));
    rngLabel->setTextColor(cocos2d::Color4B(180, 190, 170, 180));
    panelNode_->addChild(rngLabel);

    for (int row = 0; row < 5; row++) {
        float rowY = lowerTop - row * (btnH + gap + 3.0f);
        drawButtonRow(buttonNode_, startX, rowY, cols, btnW, btnH, gap,
                      btnFace, greenLit);
    }
}

// ============================================================================
// Bottom controls — fire command buttons (left) + joystick (right)
// Below the portrait display, centered in the console
// ============================================================================

void ConsoleFrame::drawBottomControls()
{
    float hh = bezelH_ * 0.5f;
    float displayBot = displayCenterY_ - displayH_ * 0.5f;

    // === Fire command buttons (bottom-left of center) ===
    float fireTop = displayBot - 14.0f;
    float fireCX = -60.0f;
    float fireW = 110.0f;
    float fireH = 100.0f;

    // Dark recessed panel background
    drawRoundedRect(buttonNode_,
                    cocos2d::Vec2(fireCX - fireW * 0.5f, fireTop - fireH),
                    cocos2d::Vec2(fireCX + fireW * 0.5f, fireTop),
                    4.0f,
                    cocos2d::Color4F(0.14f, 0.16f, 0.13f, 1.0f),
                    cocos2d::Color4F(0.30f, 0.38f, 0.32f, 0.6f));

    float fbtnW = 28.0f;
    float fbtnH = 18.0f;
    float fbtnGap = 6.0f;
    float row1StartX = fireCX - (3 * (fbtnW + fbtnGap) - fbtnGap) * 0.5f + fbtnW * 0.5f;

    // Row 1: ENGAGE, CEASE FIRE, HOLD FIRE
    const char* fireLabels1[] = { "ENG", "C/F", "HLD" };
    cocos2d::Color4F fireColors1[] = {
        cocos2d::Color4F(0.55f, 0.20f, 0.15f, 1.0f),
        cocos2d::Color4F(0.55f, 0.50f, 0.15f, 1.0f),
        cocos2d::Color4F(0.20f, 0.50f, 0.20f, 1.0f),
    };
    float row1Y = fireTop - 14.0f;
    for (int i = 0; i < 3; i++) {
        float bx = row1StartX + i * (fbtnW + fbtnGap);
        drawLabeledButton(buttonNode_, bx, row1Y, fbtnW, fbtnH,
                          fireColors1[i], fireLabels1[i], 6.0f);
    }

    // Row 2: ARM, SAFE, IFF
    const char* fireLabels2[] = { "ARM", "SAFE", "IFF" };
    cocos2d::Color4F fireColors2[] = {
        cocos2d::Color4F(0.55f, 0.20f, 0.15f, 1.0f),
        cocos2d::Color4F(0.20f, 0.50f, 0.20f, 1.0f),
        cocos2d::Color4F(0.40f, 0.45f, 0.55f, 1.0f),
    };
    float row2Y = row1Y - fbtnH - fbtnGap - 2.0f;
    for (int i = 0; i < 3; i++) {
        float bx = row1StartX + i * (fbtnW + fbtnGap);
        drawLabeledButton(buttonNode_, bx, row2Y, fbtnW, fbtnH,
                          fireColors2[i], fireLabels2[i], 6.0f);
    }

    // Row 3: SEL, DRP, ALT
    const char* fireLabels3[] = { "SEL", "DRP", "ALT" };
    cocos2d::Color4F fireColor3(0.60f, 0.58f, 0.50f, 1.0f);
    float row3Y = row2Y - fbtnH - fbtnGap - 2.0f;
    for (int i = 0; i < 3; i++) {
        float bx = row1StartX + i * (fbtnW + fbtnGap);
        drawLabeledButton(buttonNode_, bx, row3Y, fbtnW, fbtnH,
                          fireColor3, fireLabels3[i], 6.0f);
    }

    // Row 4: RNG, AZM, ALT
    const char* fireLabels4[] = { "RNG", "AZM", "ALT" };
    float row4Y = row3Y - fbtnH - fbtnGap - 2.0f;
    for (int i = 0; i < 3; i++) {
        float bx = row1StartX + i * (fbtnW + fbtnGap);
        drawLabeledButton(buttonNode_, bx, row4Y, fbtnW, fbtnH,
                          fireColor3, fireLabels4[i], 6.0f);
    }

    // === Joystick (bottom-right of center) ===
    float joyX = 70.0f;
    float joyY = fireTop - fireH * 0.5f;

    // Base plate
    buttonNode_->drawSolidCircle(
        cocos2d::Vec2(joyX, joyY), 22.0f, 0, 24,
        cocos2d::Color4F(0.16f, 0.16f, 0.14f, 1.0f));
    buttonNode_->drawCircle(
        cocos2d::Vec2(joyX, joyY), 22.0f, 0, 24, false,
        cocos2d::Color4F(0.38f, 0.36f, 0.30f, 1.0f));

    // Boot
    buttonNode_->drawSolidCircle(
        cocos2d::Vec2(joyX, joyY), 15.0f, 0, 20,
        cocos2d::Color4F(0.10f, 0.10f, 0.08f, 1.0f));

    // Shaft + knob
    buttonNode_->drawSolidCircle(
        cocos2d::Vec2(joyX, joyY), 6.0f, 0, 12,
        cocos2d::Color4F(0.25f, 0.25f, 0.22f, 1.0f));
    buttonNode_->drawSolidCircle(
        cocos2d::Vec2(joyX, joyY + 2), 5.0f, 0, 12,
        cocos2d::Color4F(0.35f, 0.35f, 0.30f, 1.0f));
    buttonNode_->drawCircle(
        cocos2d::Vec2(joyX, joyY + 2), 5.0f, 0, 12, false,
        cocos2d::Color4F(0.45f, 0.43f, 0.38f, 1.0f));

    // Directional marks
    float markDist = 18.0f;
    cocos2d::Color4F markColor(0.45f, 0.43f, 0.38f, 0.6f);
    buttonNode_->drawLine(cocos2d::Vec2(joyX, joyY + markDist - 2),
                          cocos2d::Vec2(joyX, joyY + markDist + 2), markColor);
    buttonNode_->drawLine(cocos2d::Vec2(joyX, joyY - markDist - 2),
                          cocos2d::Vec2(joyX, joyY - markDist + 2), markColor);
    buttonNode_->drawLine(cocos2d::Vec2(joyX + markDist - 2, joyY),
                          cocos2d::Vec2(joyX + markDist + 2, joyY), markColor);
    buttonNode_->drawLine(cocos2d::Vec2(joyX - markDist - 2, joyY),
                          cocos2d::Vec2(joyX - markDist + 2, joyY), markColor);

    auto* joyLabel = cocos2d::Label::createWithSystemFont("CURSOR", "Courier", 6);
    joyLabel->setPosition(cocos2d::Vec2(joyX, joyY - 28));
    joyLabel->setTextColor(cocos2d::Color4B(130, 130, 115, 180));
    buttonNode_->addChild(joyLabel);

    // Thumb buttons
    float tbX = joyX + 35.0f;
    cocos2d::Color4F tbColor(0.55f, 0.53f, 0.46f, 1.0f);
    drawLabeledButton(buttonNode_, tbX, joyY + 10, 16, 12, tbColor, "ACQ", 5.0f);
    drawLabeledButton(buttonNode_, tbX, joyY - 10, 16, 12, tbColor, "TGT", 5.0f);
}

// ============================================================================
// Top indicator row — status LEDs above the portrait display
// ============================================================================

void ConsoleFrame::drawTopIndicatorRow()
{
    float displayTop = displayCenterY_ + displayH_ * 0.5f;
    float hh = bezelH_ * 0.5f;

    float indicatorY = displayTop + 12.0f;
    float indicatorTop = hh - 10.0f;

    // Background strip for indicators (spans across display width)
    float stripHW = displayW_ * 0.55f;
    housingNode_->drawSolidRect(
        cocos2d::Vec2(-stripHW, indicatorY),
        cocos2d::Vec2(stripHW, indicatorTop),
        cocos2d::Color4F(0.18f, 0.22f, 0.18f, 1.0f));

    struct Indicator { float x; float cr, cg, cb; const char* label; };
    Indicator indicators[] = {
        { -120.0f, 0.0f, 0.7f, 0.0f, "PWR" },
        {  -60.0f, 0.7f, 0.7f, 0.0f, "RDR" },
        {    0.0f, 0.0f, 0.7f, 0.0f, "IFF" },
        {   60.0f, 0.7f, 0.0f, 0.0f, "WPN" },
        {  120.0f, 0.0f, 0.7f, 0.0f, "COM" }
    };

    float midY = (indicatorY + indicatorTop) * 0.5f;

    for (const auto& ind : indicators) {
        cocos2d::Vec2 pos(ind.x, midY);

        housingNode_->drawSolidCircle(pos, 5.0f, 0, 10,
            cocos2d::Color4F(0.10f, 0.10f, 0.08f, 1.0f));
        housingNode_->drawSolidCircle(pos, 3.5f, 0, 10,
            cocos2d::Color4F(ind.cr, ind.cg, ind.cb, 0.85f));
        housingNode_->drawSolidCircle(pos, 7.0f, 0, 12,
            cocos2d::Color4F(ind.cr, ind.cg, ind.cb, 0.12f));

        auto* label = cocos2d::Label::createWithSystemFont(ind.label, "Courier", 6);
        label->setPosition(pos + cocos2d::Vec2(0, -10));
        label->setTextColor(cocos2d::Color4B(160, 160, 140, 200));
        housingNode_->addChild(label);
    }
}

// ============================================================================
// Manufacturer plate
// ============================================================================

void ConsoleFrame::drawManufacturerPlate()
{
    float hh = bezelH_ * 0.5f;

    // Small plate at very bottom of bezel (brushed metal)
    float plateY = -hh + 4.0f;
    housingNode_->drawSolidRect(
        cocos2d::Vec2(-55, plateY),
        cocos2d::Vec2(55, plateY + 12),
        cocos2d::Color4F(0.48f, 0.55f, 0.48f, 1.0f));
    housingNode_->drawRect(
        cocos2d::Vec2(-55, plateY),
        cocos2d::Vec2(55, plateY + 12),
        cocos2d::Color4F(0.38f, 0.46f, 0.40f, 1.0f));

    auto* mfgLabel = cocos2d::Label::createWithSystemFont(
        "HUGHES AIRCRAFT CO", "Courier", 5);
    mfgLabel->setPosition(cocos2d::Vec2(0, plateY + 6));
    mfgLabel->setTextColor(cocos2d::Color4B(40, 40, 35, 200));
    housingNode_->addChild(mfgLabel);

    auto* modelLabel = cocos2d::Label::createWithSystemFont(
        "AN/TSQ-73", "Courier", 5);
    modelLabel->setPosition(cocos2d::Vec2(0, plateY + 1));
    modelLabel->setTextColor(cocos2d::Color4B(40, 40, 35, 180));
    housingNode_->addChild(modelLabel);
}

// ============================================================================
// Dynamic indicators — lit buttons, status readouts (updated each frame)
// ============================================================================

void ConsoleFrame::drawDynamicIndicators()
{
    float displayBot = displayCenterY_ - displayH_ * 0.5f;

    // Fire button glow when engaged
    if (fireControl_) {
        auto batteries = fireControl_->getAllBatteryData();
        bool anyEngaged = false;
        for (const auto& bat : batteries) {
            if (bat.status == BatteryStatus::ENGAGED) anyEngaged = true;
        }

        if (anyEngaged) {
            // Pulse the ENGAGE button area (bottom-left fire panel)
            float fireCX = -60.0f;
            float fireTop = displayBot - 14.0f;
            float fbtnW = 28.0f;
            float fbtnH = 18.0f;
            float fbtnGap = 4.0f;
            float row1StartX = fireCX - (3 * (fbtnW + fbtnGap) - fbtnGap) * 0.5f + fbtnW * 0.5f;
            float row1Y = fireTop - 14.0f;

            float pulse = 0.5f + 0.5f * sinf((float)clock() / 200.0f);
            dynamicNode_->drawSolidRect(
                cocos2d::Vec2(row1StartX - fbtnW * 0.5f - 2, row1Y - fbtnH * 0.5f - 2),
                cocos2d::Vec2(row1StartX + fbtnW * 0.5f + 2, row1Y + fbtnH * 0.5f + 2),
                cocos2d::Color4F(0.8f, 0.15f, 0.1f, 0.25f * pulse));
        }
    }

    // Threat indicator — flash the top of the right panel
    if (threatBoard_ && threatBoard_->getThreatCount() > 0) {
        float rpCX = displayW_ * 0.5f + panelGap_ + panelW_ * 0.5f;
        float hph = panelH_ * 0.5f;

        float pulse = 0.5f + 0.5f * sinf((float)clock() / 150.0f);
        dynamicNode_->drawSolidRect(
            cocos2d::Vec2(rpCX - panelW_ * 0.4f, hph - 30),
            cocos2d::Vec2(rpCX + panelW_ * 0.4f, hph - 10),
            cocos2d::Color4F(0.6f, 0.35f, 0.05f, 0.2f * pulse));
    }
}

// ============================================================================
// Bottom bar — score, level, latest message (below the bezel)
// ============================================================================

void ConsoleFrame::drawBottomBar()
{
    float hh = bezelH_ * 0.5f;
    float hw = bezelW_ * 0.5f;

    float barTop = -hh - 6.0f;
    float barBot = -hh - 28.0f;
    float barLeft = -hw;
    float barRight = hw;

    // Background
    dynamicNode_->drawSolidRect(
        cocos2d::Vec2(barLeft, barBot),
        cocos2d::Vec2(barRight, barTop),
        cocos2d::Color4F(0.06f, 0.02f, 0.02f, 0.9f));

    dynamicNode_->drawRect(
        cocos2d::Vec2(barLeft, barBot),
        cocos2d::Vec2(barRight, barTop),
        cocos2d::Color4F(0.16f, 0.06f, 0.06f, 0.6f));

    float midY = (barTop + barBot) * 0.5f;

    // Score
    char scoreBuf[32];
    snprintf(scoreBuf, sizeof(scoreBuf), "SCORE: %d", score_);
    auto* scoreLabel = cocos2d::Label::createWithSystemFont(
        scoreBuf, "Courier", 9);
    scoreLabel->setPosition(cocos2d::Vec2(barLeft + 10, midY));
    scoreLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
    scoreLabel->setTextColor(score_ < 0
        ? cocos2d::Color4B(255, 80, 80, 255)
        : cocos2d::Color4B(255, 100, 60, 255));
    labelNode_->addChild(scoreLabel);

    // Level
    char lvlBuf[16];
    snprintf(lvlBuf, sizeof(lvlBuf), "LV:%d", level_);
    auto* lvlLabel = cocos2d::Label::createWithSystemFont(
        lvlBuf, "Courier", 9);
    lvlLabel->setPosition(cocos2d::Vec2(barLeft + 100, midY));
    lvlLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
    lvlLabel->setTextColor(cocos2d::Color4B(255, 180, 60, 255));
    labelNode_->addChild(lvlLabel);

    // Latest message
    if (!messages_.empty()) {
        std::string msg = messages_.back().substr(0, 60);
        auto* msgLabel = cocos2d::Label::createWithSystemFont(
            msg, "Courier", 7);
        msgLabel->setPosition(cocos2d::Vec2(barLeft + 170, midY));
        msgLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
        msgLabel->setTextColor(cocos2d::Color4B(0, 220, 0, 200));
        labelNode_->addChild(msgLabel);
    }
}

#else
// ============================================================================
// Stub ConsoleFrame (no cocos2d-x)
// ============================================================================

ConsoleFrame* ConsoleFrame::create(float scopeRadius)
{
    auto* frame = new ConsoleFrame();
    if (frame->init(scopeRadius)) return frame;
    delete frame;
    return nullptr;
}

bool ConsoleFrame::init(float scopeRadius)
{
    scopeRadius_ = scopeRadius;
    float scopeDia = scopeRadius * 2.0f;
    displayW_ = scopeDia + 50.0f;
    displayH_ = displayW_ * 1.35f;
    panelW_ = 130.0f;
    panelGap_ = 12.0f;
    panelH_ = displayH_ + 40.0f;
    bezelW_ = displayW_ + 2.0f * (panelGap_ + panelW_) + 40.0f;
    bezelH_ = panelH_ + 90.0f;
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
