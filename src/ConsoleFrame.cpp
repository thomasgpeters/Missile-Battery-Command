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

    // === AN/TSQ-73 layout: landscape console, wider than tall ===
    //
    // The real console is landscape-format: a portrait CRT display centered
    // inside a wide console housing with generous control panels on each side,
    // a writing shelf below, and status indicators along the top.

    // Portrait CRT display: sized to frame the circular scope with margin.
    // The glass opening is smaller than the radar sweep — the bezel hides the
    // outer ring of the sweep, so only the inner field of play is visible.
    float scopeDia = scopeRadius * 2.0f;
    float bezelInset = 35.0f;                    // extra bezel width on each side
    displayW_ = scopeDia + 50.0f - bezelInset * 2.0f; // smaller glass opening
    displayH_ = displayW_ * 1.25f;              // portrait: 25% taller than wide
    displayCornerR_ = 48.0f;                     // rounded corners on CRT
    displayCenterY_ = 15.0f;                     // scope slightly above center (room for shelf)
    bezelThick_ = 52.0f;                         // wide octagonal bezel around glass

    // Side panels: positioned outside the bezel so side thickness matches top
    panelW_ = 160.0f;
    panelH_ = (displayW_ + bezelThick_ * 2.0f) * 1.25f + 30.0f;  // match bezel outer height
    panelGap_ = 14.0f;                          // gap between bezel edge and panel

    // Overall console extents — landscape: wider than tall
    // Panels positioned from bezel outer edge outward
    float bezelOuterW = displayW_ + bezelThick_ * 2.0f;
    bezelW_ = bezelOuterW + 2.0f * (panelGap_ + panelW_) + 60.0f;
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
    // Note: drawScopeRing() removed — the wider bezel now covers the scope
    // rim area, and RadarDisplay has its own azimuth ticks/labels.
    drawLeftPanel();
    drawRightPanel();
    drawBottomControls();
    drawWritingBench();
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
// Shelter background — steel equipment cabinets behind the console
// The AN/TSQ-73 sits inside an S-529/S-280 shelter filled with
// 19" relay racks, power distribution, comm equipment behind the operator.
// ============================================================================

void ConsoleFrame::drawShelterBackground()
{
    float sceneW = 900.0f;
    float sceneH = 600.0f;

    // Overall dark interior
    housingNode_->drawSolidRect(
        cocos2d::Vec2(-sceneW, -sceneH),
        cocos2d::Vec2(sceneW, sceneH),
        cocos2d::Color4F(0.04f, 0.05f, 0.04f, 1.0f));

    // === Steel equipment cabinet wall (behind console) ===
    // Full-width row of 19" relay rack cabinets
    float cabTop = sceneH * 0.85f;
    float cabBot = -sceneH * 0.55f;
    float cabW = 85.0f;     // each cabinet panel width
    float cabGap = 3.0f;    // gap between cabinets

    int numCabs = (int)(2.0f * sceneW / (cabW + cabGap)) + 2;
    float startX = -sceneW;

    cocos2d::Color4F cabDark(0.07f, 0.08f, 0.07f, 1.0f);
    cocos2d::Color4F cabMid(0.10f, 0.11f, 0.10f, 1.0f);
    cocos2d::Color4F cabEdge(0.14f, 0.15f, 0.13f, 1.0f);
    cocos2d::Color4F cabBorder(0.06f, 0.06f, 0.05f, 1.0f);

    for (int i = 0; i < numCabs; i++) {
        float cx = startX + i * (cabW + cabGap);

        // Cabinet face — dark steel
        housingNode_->drawSolidRect(
            cocos2d::Vec2(cx, cabBot),
            cocos2d::Vec2(cx + cabW, cabTop),
            (i % 3 == 0) ? cabDark : cabMid);

        // Cabinet border lines (recessed seams between panels)
        housingNode_->drawLine(
            cocos2d::Vec2(cx, cabBot), cocos2d::Vec2(cx, cabTop), cabBorder);
        housingNode_->drawLine(
            cocos2d::Vec2(cx + cabW, cabBot), cocos2d::Vec2(cx + cabW, cabTop), cabBorder);

        // Vertical edge highlight (left side catches faint light)
        housingNode_->drawLine(
            cocos2d::Vec2(cx + 1, cabBot + 10), cocos2d::Vec2(cx + 1, cabTop - 10),
            cocos2d::Color4F(0.14f, 0.16f, 0.14f, 0.3f));

        // Horizontal panel dividers (3 sections per cabinet)
        float sectionH = (cabTop - cabBot) / 3.0f;
        for (int s = 1; s < 3; s++) {
            float divY = cabBot + s * sectionH;
            housingNode_->drawLine(
                cocos2d::Vec2(cx + 4, divY),
                cocos2d::Vec2(cx + cabW - 4, divY),
                cabBorder);
            // Small handle/latch detail on each section
            float handleY = divY - sectionH * 0.5f;
            housingNode_->drawSolidRect(
                cocos2d::Vec2(cx + cabW * 0.35f, handleY - 2),
                cocos2d::Vec2(cx + cabW * 0.65f, handleY + 2),
                cocos2d::Color4F(0.16f, 0.17f, 0.15f, 0.5f));
        }

        // Ventilation louvers on some panels (every other cabinet)
        if (i % 2 == 1) {
            float ventTop = cabTop - 15.0f;
            for (int v = 0; v < 4; v++) {
                float vy = ventTop - v * 6.0f;
                housingNode_->drawLine(
                    cocos2d::Vec2(cx + 10, vy),
                    cocos2d::Vec2(cx + cabW - 10, vy),
                    cocos2d::Color4F(0.03f, 0.03f, 0.03f, 0.6f));
            }
        }
    }

    // === Ceiling ===
    housingNode_->drawSolidRect(
        cocos2d::Vec2(-sceneW, cabTop),
        cocos2d::Vec2(sceneW, sceneH),
        cocos2d::Color4F(0.06f, 0.07f, 0.06f, 1.0f));

    // Ceiling structural ribs
    for (int cx = -700; cx <= 700; cx += 200) {
        housingNode_->drawSolidRect(
            cocos2d::Vec2((float)cx - 3, cabTop),
            cocos2d::Vec2((float)cx + 3, sceneH),
            cocos2d::Color4F(0.08f, 0.09f, 0.08f, 1.0f));
    }

    // Red night ops light strips (dimmer, more subtle)
    for (int lx = -500; lx <= 500; lx += 250) {
        housingNode_->drawSolidRect(
            cocos2d::Vec2((float)lx - 15.0f, sceneH - 30.0f),
            cocos2d::Vec2((float)lx + 15.0f, sceneH - 26.0f),
            cocos2d::Color4F(0.22f, 0.04f, 0.04f, 0.5f));
        // Red light glow cone downward
        housingNode_->drawSolidRect(
            cocos2d::Vec2((float)lx - 40.0f, sceneH - 60.0f),
            cocos2d::Vec2((float)lx + 40.0f, sceneH - 30.0f),
            cocos2d::Color4F(0.12f, 0.03f, 0.03f, 0.15f));
    }

    // === Floor ===
    housingNode_->drawSolidRect(
        cocos2d::Vec2(-sceneW, -sceneH),
        cocos2d::Vec2(sceneW, cabBot),
        cocos2d::Color4F(0.04f, 0.04f, 0.03f, 1.0f));

    // Floor highlight (rubber matting sheen)
    housingNode_->drawSolidRect(
        cocos2d::Vec2(-sceneW, cabBot - 2),
        cocos2d::Vec2(sceneW, cabBot),
        cocos2d::Color4F(0.08f, 0.08f, 0.07f, 0.5f));
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
    // Wide bezel: the radar sweep extends behind this, visible only through the glass
    float bezelThick = 52.0f;
    float chamfer = 55.0f;  // 45° cut on each corner of the octagon (wider for larger bezel)

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
    // Panel positioned outside the bezel outer edge so side thickness matches top
    float panelCX = -(displayW_ * 0.5f + bezelThick_ + panelGap_ + panelW_ * 0.5f);
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
    float btnW = 22.0f;
    float btnH = 16.0f;
    float gap = 6.0f;
    float sectionGap = 22.0f;           // generous spacing between button groups
    float rowStep = btnH + gap + 5.0f;  // more vertical padding per row
    float padding = 10.0f;
    float pLeft = panelCX - hpw + inset + padding;
    float usableW = panelW_ - 2.0f * inset - 2.0f * padding;
    int cols = std::max(1, (int)(usableW / (btnW + gap)));
    float startX = pLeft + (usableW - cols * (btnW + gap) + gap) * 0.5f;

    cocos2d::Color4F btnFace(0.62f, 0.60f, 0.52f, 1.0f);
    cocos2d::Color4F dimLit(0.15f, 0.30f, 0.10f, 0.4f);
    cocos2d::Color4F amberLit(0.50f, 0.35f, 0.05f, 0.5f);

    // === TRACK MGMT section (top) ===
    auto* trkLabel = cocos2d::Label::createWithSystemFont("TRACK MGMT", "Courier", 6);
    trkLabel->setPosition(cocos2d::Vec2(panelCX, hph - 16));
    trkLabel->setTextColor(cocos2d::Color4B(180, 190, 170, 200));
    panelNode_->addChild(trkLabel);

    float rowTop = hph - 28.0f;
    for (int row = 0; row < 4; row++) {
        float rowY = rowTop - row * rowStep;
        cocos2d::Color4F litColor = (row < 2) ? amberLit : dimLit;
        drawButtonRow(buttonNode_, startX, rowY, cols, btnW, btnH, gap,
                      btnFace, litColor);
    }

    // === NUMERIC KEYPAD section ===
    float keypadTop = rowTop - 4 * rowStep - sectionGap;
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

    // === FIRE CONTROL section (anchored to bottom of panel, centered) ===
    float fbtnW = 28.0f;
    float fbtnH = 18.0f;
    float fbtnGap = 6.0f;
    float fireRowStep = fbtnH + fbtnGap + 2.0f;
    float fireSectionH = 4 * fireRowStep + 18.0f;  // 4 rows + label + padding
    float panelBot = -hph + inset + 16.0f;          // bottom of inner recess + padding
    float fireBot = panelBot;
    float fireTop = fireBot + fireSectionH;

    auto* fireLabel = cocos2d::Label::createWithSystemFont("FIRE CONTROL", "Courier", 6);
    fireLabel->setPosition(cocos2d::Vec2(panelCX, fireTop));
    fireLabel->setTextColor(cocos2d::Color4B(180, 190, 170, 200));
    panelNode_->addChild(fireLabel);

    // Dark recessed background for fire section
    drawRoundedRect(buttonNode_,
                    cocos2d::Vec2(panelCX - hpw + inset + 4, fireBot),
                    cocos2d::Vec2(panelCX + hpw - inset - 4, fireTop - 6),
                    4.0f,
                    cocos2d::Color4F(0.10f, 0.12f, 0.09f, 1.0f),
                    cocos2d::Color4F(0.28f, 0.36f, 0.30f, 0.6f));

    float row1StartX = panelCX - (3 * (fbtnW + fbtnGap) - fbtnGap) * 0.5f + fbtnW * 0.5f;

    // Row 1 (top of fire section): ENGAGE, CEASE FIRE, HOLD FIRE
    const char* fireLabels1[] = { "ENG", "C/F", "HLD" };
    cocos2d::Color4F fireColors1[] = {
        cocos2d::Color4F(0.55f, 0.20f, 0.15f, 1.0f),
        cocos2d::Color4F(0.55f, 0.50f, 0.15f, 1.0f),
        cocos2d::Color4F(0.20f, 0.50f, 0.20f, 1.0f),
    };
    float row1Y = fireTop - 18.0f;
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
    float row2Y = row1Y - fireRowStep;
    for (int i = 0; i < 3; i++) {
        float bx = row1StartX + i * (fbtnW + fbtnGap);
        drawLabeledButton(buttonNode_, bx, row2Y, fbtnW, fbtnH,
                          fireColors2[i], fireLabels2[i], 6.0f);
    }

    // Row 3: SEL, DRP, ALT
    const char* fireLabels3[] = { "SEL", "DRP", "ALT" };
    cocos2d::Color4F fireColor3(0.60f, 0.58f, 0.50f, 1.0f);
    float row3Y = row2Y - fireRowStep;
    for (int i = 0; i < 3; i++) {
        float bx = row1StartX + i * (fbtnW + fbtnGap);
        drawLabeledButton(buttonNode_, bx, row3Y, fbtnW, fbtnH,
                          fireColor3, fireLabels3[i], 6.0f);
    }

    // Row 4: RNG, AZM, ALT
    const char* fireLabels4[] = { "RNG", "AZM", "ALT" };
    float row4Y = row3Y - fireRowStep;
    for (int i = 0; i < 3; i++) {
        float bx = row1StartX + i * (fbtnW + fbtnGap);
        drawLabeledButton(buttonNode_, bx, row4Y, fbtnW, fbtnH,
                          fireColor3, fireLabels4[i], 6.0f);
    }
}

// ============================================================================
// Right panel — tall narrow control panel (console settings, rotary knobs)
// Rendered separately from the display, to the right
// ============================================================================

void ConsoleFrame::drawRightPanel()
{
    // Panel positioned outside the bezel outer edge so side thickness matches top
    float panelCX = displayW_ * 0.5f + bezelThick_ + panelGap_ + panelW_ * 0.5f;
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

    float btnW = 22.0f;
    float btnH = 16.0f;
    float gap = 6.0f;
    float sectionGap = 22.0f;           // generous spacing between sections
    float rowStep = btnH + gap + 5.0f;  // more vertical padding per row
    float padding = 10.0f;
    float pLeft = panelCX - hpw + inset + padding;
    float usableW = panelW_ - 2.0f * inset - 2.0f * padding;
    int cols = std::max(1, (int)(usableW / (btnW + gap)));
    float startX = pLeft + (usableW - cols * (btnW + gap) + gap) * 0.5f;

    cocos2d::Color4F btnFace(0.62f, 0.60f, 0.52f, 1.0f);
    cocos2d::Color4F greenLit(0.10f, 0.40f, 0.10f, 0.5f);
    cocos2d::Color4F amberLit(0.50f, 0.35f, 0.05f, 0.5f);

    // === SETTINGS section (top) ===
    auto* settLabel = cocos2d::Label::createWithSystemFont("SETTINGS", "Courier", 6);
    settLabel->setPosition(cocos2d::Vec2(panelCX, hph - 16));
    settLabel->setTextColor(cocos2d::Color4B(180, 190, 170, 200));
    panelNode_->addChild(settLabel);

    float rowTop = hph - 28.0f;
    for (int row = 0; row < 4; row++) {
        float rowY = rowTop - row * rowStep;
        cocos2d::Color4F litColor = (row == 0) ? amberLit : greenLit;
        drawButtonRow(buttonNode_, startX, rowY, cols, btnW, btnH, gap,
                      btnFace, litColor);
    }

    // === GAIN/VIDEO rotary controls ===
    float knobSectionTop = rowTop - 4 * rowStep - sectionGap;

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

    // === RANGE/MTI section ===
    float lowerTop = knob2Y - knobR - sectionGap;

    auto* rngLabel = cocos2d::Label::createWithSystemFont("RANGE/MTI", "Courier", 5);
    rngLabel->setPosition(cocos2d::Vec2(panelCX, lowerTop + 12));
    rngLabel->setTextColor(cocos2d::Color4B(180, 190, 170, 180));
    panelNode_->addChild(rngLabel);

    for (int row = 0; row < 3; row++) {
        float rowY = lowerTop - row * rowStep;
        drawButtonRow(buttonNode_, startX, rowY, cols, btnW, btnH, gap,
                      btnFace, greenLit);
    }

    // === CURSOR JOYSTICK (anchored to bottom of right panel) ===
    float panelBot = -hph + inset + 16.0f;          // bottom of inner recess + padding
    float joyY = panelBot + 22.0f;                   // base plate radius up from bottom

    auto* joyLabel = cocos2d::Label::createWithSystemFont("CURSOR", "Courier", 6);
    joyLabel->setPosition(cocos2d::Vec2(panelCX, joyY + 30));
    joyLabel->setTextColor(cocos2d::Color4B(130, 130, 115, 180));
    buttonNode_->addChild(joyLabel);

    // Base plate
    buttonNode_->drawSolidCircle(
        cocos2d::Vec2(panelCX, joyY), 22.0f, 0, 24,
        cocos2d::Color4F(0.16f, 0.16f, 0.14f, 1.0f));
    buttonNode_->drawCircle(
        cocos2d::Vec2(panelCX, joyY), 22.0f, 0, 24, false,
        cocos2d::Color4F(0.38f, 0.36f, 0.30f, 1.0f));

    // Boot
    buttonNode_->drawSolidCircle(
        cocos2d::Vec2(panelCX, joyY), 15.0f, 0, 20,
        cocos2d::Color4F(0.10f, 0.10f, 0.08f, 1.0f));

    // Shaft + knob
    buttonNode_->drawSolidCircle(
        cocos2d::Vec2(panelCX, joyY), 6.0f, 0, 12,
        cocos2d::Color4F(0.25f, 0.25f, 0.22f, 1.0f));
    buttonNode_->drawSolidCircle(
        cocos2d::Vec2(panelCX, joyY + 2), 5.0f, 0, 12,
        cocos2d::Color4F(0.35f, 0.35f, 0.30f, 1.0f));
    buttonNode_->drawCircle(
        cocos2d::Vec2(panelCX, joyY + 2), 5.0f, 0, 12, false,
        cocos2d::Color4F(0.45f, 0.43f, 0.38f, 1.0f));

    // Directional marks
    float markDist = 18.0f;
    cocos2d::Color4F markColor(0.45f, 0.43f, 0.38f, 0.6f);
    buttonNode_->drawLine(cocos2d::Vec2(panelCX, joyY + markDist - 2),
                          cocos2d::Vec2(panelCX, joyY + markDist + 2), markColor);
    buttonNode_->drawLine(cocos2d::Vec2(panelCX, joyY - markDist - 2),
                          cocos2d::Vec2(panelCX, joyY - markDist + 2), markColor);
    buttonNode_->drawLine(cocos2d::Vec2(panelCX + markDist - 2, joyY),
                          cocos2d::Vec2(panelCX + markDist + 2, joyY), markColor);
    buttonNode_->drawLine(cocos2d::Vec2(panelCX - markDist - 2, joyY),
                          cocos2d::Vec2(panelCX - markDist + 2, joyY), markColor);

    // Thumb buttons beside joystick
    cocos2d::Color4F tbColor(0.55f, 0.53f, 0.46f, 1.0f);
    drawLabeledButton(buttonNode_, panelCX + 35.0f, joyY + 10, 16, 12, tbColor, "ACQ", 5.0f);
    drawLabeledButton(buttonNode_, panelCX + 35.0f, joyY - 10, 16, 12, tbColor, "TGT", 5.0f);
}

// ============================================================================
// Bottom controls — fire command buttons (left) + joystick (right)
// Below the portrait display, centered in the console
// ============================================================================

void ConsoleFrame::drawBottomControls()
{
    // Fire controls moved to bottom of left panel (drawLeftPanel)
    // Joystick moved to bottom of right panel (drawRightPanel)
}

// ============================================================================
// Writing bench — flat shelf/desk in front of the console
// The AN/TSQ-73 has a writing surface extending toward the operator
// ============================================================================

void ConsoleFrame::drawWritingBench()
{
    float hw = bezelW_ * 0.5f;
    float hh = bezelH_ * 0.5f;

    float benchTop = -hh - 4.0f;         // just below the console
    float benchH = 50.0f;                 // depth of the writing shelf
    float benchBot = benchTop - benchH;
    float benchOverhang = 20.0f;          // extends slightly wider than console

    // Shelf underside shadow
    housingNode_->drawSolidRect(
        cocos2d::Vec2(-hw - benchOverhang, benchBot - 4),
        cocos2d::Vec2(hw + benchOverhang, benchBot),
        cocos2d::Color4F(0.03f, 0.03f, 0.03f, 0.7f));

    // Shelf body — seafoam green metal to match console
    drawRoundedRect(housingNode_,
                    cocos2d::Vec2(-hw - benchOverhang, benchBot),
                    cocos2d::Vec2(hw + benchOverhang, benchTop),
                    4.0f,
                    cocos2d::Color4F(0.44f, 0.58f, 0.50f, 1.0f),
                    cocos2d::Color4F(0.32f, 0.44f, 0.38f, 1.0f));

    // Writing surface (slightly lighter, worn smooth from use)
    float surfInset = 4.0f;
    housingNode_->drawSolidRect(
        cocos2d::Vec2(-hw - benchOverhang + surfInset, benchBot + surfInset),
        cocos2d::Vec2(hw + benchOverhang - surfInset, benchTop - 2),
        cocos2d::Color4F(0.48f, 0.62f, 0.54f, 1.0f));

    // Surface highlight (top edge catches light)
    housingNode_->drawLine(
        cocos2d::Vec2(-hw - benchOverhang + 6, benchTop - 1),
        cocos2d::Vec2(hw + benchOverhang - 6, benchTop - 1),
        cocos2d::Color4F(0.56f, 0.70f, 0.60f, 0.4f));

    // Front lip (rolled edge)
    housingNode_->drawSolidRect(
        cocos2d::Vec2(-hw - benchOverhang + 2, benchBot),
        cocos2d::Vec2(hw + benchOverhang - 2, benchBot + 3),
        cocos2d::Color4F(0.38f, 0.50f, 0.42f, 1.0f));

    // Pen groove / channel along front
    housingNode_->drawLine(
        cocos2d::Vec2(-hw * 0.3f, benchBot + 8),
        cocos2d::Vec2(hw * 0.3f, benchBot + 8),
        cocos2d::Color4F(0.35f, 0.46f, 0.40f, 0.6f));
    housingNode_->drawLine(
        cocos2d::Vec2(-hw * 0.3f, benchBot + 9),
        cocos2d::Vec2(hw * 0.3f, benchBot + 9),
        cocos2d::Color4F(0.50f, 0.64f, 0.55f, 0.3f));
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
    float bezelInset = 35.0f;
    displayW_ = scopeDia + 50.0f - bezelInset * 2.0f;
    displayH_ = displayW_ * 1.25f;
    displayCornerR_ = 48.0f;
    bezelThick_ = 52.0f;
    panelW_ = 160.0f;
    panelGap_ = 14.0f;
    panelH_ = (displayW_ + bezelThick_ * 2.0f) * 1.25f + 30.0f;
    float bezelOuterW = displayW_ + bezelThick_ * 2.0f;
    bezelW_ = bezelOuterW + 2.0f * (panelGap_ + panelW_) + 60.0f;
    bezelH_ = panelH_ + 90.0f;
    displayCenterY_ = 15.0f;
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
