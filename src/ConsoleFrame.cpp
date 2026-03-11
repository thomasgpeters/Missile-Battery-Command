#include "ConsoleFrame.h"
#include <cstdio>
#include <cmath>
#include <algorithm>

#if USE_COCOS2DX
// ============================================================================
// Cocos2d-x ConsoleFrame — AN/TSQ-72 Analog Console
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

    // AN/TSQ-72 bezel: portrait format, ~25% taller than wide
    // The scope sits in the upper portion; buttons fill the sides and bottom
    bezelW_ = scopeRadius * 2.0f + 240.0f;   // scope + side button panels
    bezelH_ = bezelW_ * 1.25f;                // 25% taller than wide
    cornerRadius_ = 12.0f;                     // ~2-3 inch radius at scale

    // Create draw layers
    housingNode_ = cocos2d::DrawNode::create();
    buttonNode_  = cocos2d::DrawNode::create();
    dynamicNode_ = cocos2d::DrawNode::create();
    labelNode_   = cocos2d::Node::create();

    addChild(housingNode_,  0);
    addChild(buttonNode_,   1);
    addChild(dynamicNode_,  2);
    addChild(labelNode_,    3);

    // Draw all static elements once
    drawShelterBackground();
    drawBezel();
    drawScopeRing();
    drawLeftButtonPanel();
    drawRightButtonPanel();
    drawBottomLeftFireButtons();
    drawBottomRightJoystick();
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
// Main bezel — portrait rectangle with rounded corners
// ============================================================================

void ConsoleFrame::drawBezel()
{
    float hw = bezelW_ * 0.5f;
    float hh = bezelH_ * 0.5f;

    // Outer bezel frame (cream/tan metal — matches the photo)
    cocos2d::Color4F bezelColor(0.72f, 0.70f, 0.62f, 1.0f);
    cocos2d::Color4F bezelBorder(0.55f, 0.53f, 0.47f, 1.0f);

    drawRoundedRect(housingNode_,
                    cocos2d::Vec2(-hw, -hh),
                    cocos2d::Vec2(hw, hh),
                    cornerRadius_,
                    bezelColor, bezelBorder);

    // Inner recessed area (darker, where scope + buttons sit)
    float inset = 8.0f;
    cocos2d::Color4F recessColor(0.58f, 0.56f, 0.50f, 1.0f);
    cocos2d::Color4F recessBorder(0.45f, 0.43f, 0.38f, 1.0f);

    drawRoundedRect(housingNode_,
                    cocos2d::Vec2(-hw + inset, -hh + inset),
                    cocos2d::Vec2(hw - inset, hh - inset),
                    cornerRadius_ - 2.0f,
                    recessColor, recessBorder);

    // Shadow/bevel lines for depth
    // Top highlight
    housingNode_->drawLine(
        cocos2d::Vec2(-hw + cornerRadius_, hh - 1),
        cocos2d::Vec2(hw - cornerRadius_, hh - 1),
        cocos2d::Color4F(0.80f, 0.78f, 0.70f, 0.6f));
    // Bottom shadow
    housingNode_->drawLine(
        cocos2d::Vec2(-hw + cornerRadius_, -hh + 1),
        cocos2d::Vec2(hw - cornerRadius_, -hh + 1),
        cocos2d::Color4F(0.40f, 0.38f, 0.33f, 0.8f));
}

// ============================================================================
// Scope ring — rubber hood and metal bezel around PPI
// ============================================================================

void ConsoleFrame::drawScopeRing()
{
    float r = scopeRadius_;

    // Scope sits in upper half of bezel
    float scopeCenterY = bezelH_ * 0.15f;

    // Dark scope well (black circle behind the PPI)
    housingNode_->drawSolidCircle(
        cocos2d::Vec2(0, scopeCenterY), r + 6.0f, 0, 72,
        cocos2d::Color4F(0.03f, 0.03f, 0.02f, 1.0f));

    // Rubber hood ring (dark gray)
    housingNode_->drawCircle(
        cocos2d::Vec2(0, scopeCenterY), r + 8.0f, 0, 72, false,
        cocos2d::Color4F(0.12f, 0.12f, 0.10f, 1.0f));
    housingNode_->drawCircle(
        cocos2d::Vec2(0, scopeCenterY), r + 5.0f, 0, 72, false,
        cocos2d::Color4F(0.10f, 0.10f, 0.08f, 1.0f));

    // Inner metal bezel ring
    housingNode_->drawCircle(
        cocos2d::Vec2(0, scopeCenterY), r + 2.0f, 0, 72, false,
        cocos2d::Color4F(0.45f, 0.43f, 0.38f, 1.0f));
    housingNode_->drawCircle(
        cocos2d::Vec2(0, scopeCenterY), r + 1.0f, 0, 72, false,
        cocos2d::Color4F(0.35f, 0.33f, 0.28f, 1.0f));

    // Mounting screws at 45-degree positions
    float screwDist = r + 12.0f;
    float positions[][2] = {
        { -0.707f,  0.707f },
        {  0.707f,  0.707f },
        {  0.707f, -0.707f },
        { -0.707f, -0.707f }
    };
    for (const auto& pos : positions) {
        cocos2d::Vec2 screwPos(screwDist * pos[0], scopeCenterY + screwDist * pos[1]);
        housingNode_->drawSolidCircle(screwPos, 3.5f, 0, 8,
            cocos2d::Color4F(0.50f, 0.48f, 0.42f, 1.0f));
        housingNode_->drawCircle(screwPos, 3.5f, 0, 8, false,
            cocos2d::Color4F(0.38f, 0.36f, 0.30f, 1.0f));
        // Phillips cross
        housingNode_->drawLine(
            screwPos + cocos2d::Vec2(-1.5f, 0), screwPos + cocos2d::Vec2(1.5f, 0),
            cocos2d::Color4F(0.40f, 0.38f, 0.33f, 1.0f));
        housingNode_->drawLine(
            screwPos + cocos2d::Vec2(0, -1.5f), screwPos + cocos2d::Vec2(0, 1.5f),
            cocos2d::Color4F(0.40f, 0.38f, 0.33f, 1.0f));
    }
}

// ============================================================================
// Left button panel — illuminated button arrays + numeric keypad
// (Track management, IFF interrogation, display controls)
// ============================================================================

void ConsoleFrame::drawLeftButtonPanel()
{
    float hw = bezelW_ * 0.5f;
    float scopeCenterY = bezelH_ * 0.15f;
    float r = scopeRadius_;

    // Panel area: left side of bezel, beside and below the scope
    float panelLeft = -hw + 14.0f;
    float panelRight = -r - 16.0f;
    float panelW = panelRight - panelLeft;

    // --- Upper button array (beside scope) ---
    // These are the illuminated pushbuttons visible in the photo
    float upperTop = scopeCenterY + r * 0.7f;
    float btnW = 20.0f;
    float btnH = 14.0f;
    float gap = 3.0f;

    // Calculate how many columns fit
    int cols = std::max(1, (int)((panelW - 4.0f) / (btnW + gap)));
    float startX = panelLeft + (panelW - cols * (btnW + gap) + gap) * 0.5f;

    // 5 rows of illuminated buttons (track/display controls)
    cocos2d::Color4F btnFace(0.68f, 0.66f, 0.58f, 1.0f);
    cocos2d::Color4F dimLit(0.15f, 0.30f, 0.10f, 0.4f);
    cocos2d::Color4F amberLit(0.50f, 0.35f, 0.05f, 0.5f);

    for (int row = 0; row < 5; row++) {
        float rowY = upperTop - row * (btnH + gap + 2.0f);
        cocos2d::Color4F litColor = (row < 2) ? amberLit : dimLit;
        drawButtonRow(buttonNode_, startX, rowY, cols, btnW, btnH, gap,
                      btnFace, litColor);
    }

    // --- Numeric keypad (below the button arrays) ---
    float keypadTop = upperTop - 5 * (btnH + gap + 2.0f) - 12.0f;
    float keyW = 18.0f;
    float keyH = 16.0f;
    float keyGap = 3.0f;

    // Keypad background
    float kpLeft = panelLeft + 4.0f;
    float kpRight = kpLeft + 3 * (keyW + keyGap) + keyGap;
    buttonNode_->drawSolidRect(
        cocos2d::Vec2(kpLeft - 4, keypadTop - 4 * (keyH + keyGap) - 8),
        cocos2d::Vec2(kpRight + 4, keypadTop + 4),
        cocos2d::Color4F(0.14f, 0.14f, 0.12f, 1.0f));

    const char* keyLabels[] = {
        "7", "8", "9",
        "4", "5", "6",
        "1", "2", "3",
        "CLR", "0", "ENT"
    };
    cocos2d::Color4F keyColor(0.62f, 0.60f, 0.54f, 1.0f);
    cocos2d::Color4F entColor(0.45f, 0.55f, 0.40f, 1.0f);
    cocos2d::Color4F clrColor(0.55f, 0.40f, 0.35f, 1.0f);

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 3; col++) {
            int idx = row * 3 + col;
            float kx = kpLeft + col * (keyW + keyGap) + keyW * 0.5f + keyGap;
            float ky = keypadTop - row * (keyH + keyGap) - keyH * 0.5f;

            cocos2d::Color4F kc = keyColor;
            if (idx == 9) kc = clrColor;   // CLR
            if (idx == 11) kc = entColor;   // ENT

            drawLabeledButton(buttonNode_, kx, ky, keyW, keyH, kc, keyLabels[idx], 6.0f);
        }
    }

    // --- Lower function buttons (below keypad) ---
    float funcTop = keypadTop - 4 * (keyH + keyGap) - 20.0f;
    int funcRows = 3;
    int funcCols = cols;
    cocos2d::Color4F funcFace(0.65f, 0.63f, 0.56f, 1.0f);
    cocos2d::Color4F funcLit(0.10f, 0.35f, 0.10f, 0.5f);

    for (int row = 0; row < funcRows; row++) {
        float rowY = funcTop - row * (btnH + gap + 2.0f);
        drawButtonRow(buttonNode_, startX, rowY, funcCols, btnW, btnH, gap,
                      funcFace, funcLit);
    }
}

// ============================================================================
// Right button panel — console settings button arrays
// (Brightness, gain, MTI, video, range controls)
// ============================================================================

void ConsoleFrame::drawRightButtonPanel()
{
    float hw = bezelW_ * 0.5f;
    float scopeCenterY = bezelH_ * 0.15f;
    float r = scopeRadius_;

    float panelLeft = r + 16.0f;
    float panelRight = hw - 14.0f;
    float panelW = panelRight - panelLeft;

    float upperTop = scopeCenterY + r * 0.7f;
    float btnW = 20.0f;
    float btnH = 14.0f;
    float gap = 3.0f;

    int cols = std::max(1, (int)((panelW - 4.0f) / (btnW + gap)));
    float startX = panelLeft + (panelW - cols * (btnW + gap) + gap) * 0.5f;

    // Upper button array — 5 rows (console settings)
    cocos2d::Color4F btnFace(0.68f, 0.66f, 0.58f, 1.0f);
    cocos2d::Color4F greenLit(0.10f, 0.40f, 0.10f, 0.5f);
    cocos2d::Color4F amberLit(0.50f, 0.35f, 0.05f, 0.5f);

    for (int row = 0; row < 5; row++) {
        float rowY = upperTop - row * (btnH + gap + 2.0f);
        cocos2d::Color4F litColor = (row == 0) ? amberLit : greenLit;
        drawButtonRow(buttonNode_, startX, rowY, cols, btnW, btnH, gap,
                      btnFace, litColor);
    }

    // Middle section: rotary controls (drawn as knobs with settings)
    float knobSectionTop = upperTop - 5 * (btnH + gap + 2.0f) - 15.0f;
    float knobR = 12.0f;
    float knobGap = 36.0f;
    int numKnobs = std::max(1, (int)(panelW / knobGap));
    float knobStartX = panelLeft + (panelW - (numKnobs - 1) * knobGap) * 0.5f;

    for (int i = 0; i < numKnobs; i++) {
        cocos2d::Vec2 kc(knobStartX + i * knobGap, knobSectionTop);

        // Knob base recess
        buttonNode_->drawSolidCircle(kc, knobR + 2, 0, 16,
            cocos2d::Color4F(0.14f, 0.14f, 0.12f, 1.0f));
        // Knob body
        buttonNode_->drawSolidCircle(kc, knobR, 0, 16,
            cocos2d::Color4F(0.22f, 0.22f, 0.19f, 1.0f));
        // Knob cap
        buttonNode_->drawSolidCircle(kc, knobR - 3, 0, 12,
            cocos2d::Color4F(0.30f, 0.30f, 0.26f, 1.0f));
        // Indicator line
        float angle = 0.4f + i * 0.7f;  // varied positions
        buttonNode_->drawLine(
            kc, kc + cocos2d::Vec2(sinf(angle) * (knobR - 2),
                                    cosf(angle) * (knobR - 2)),
            cocos2d::Color4F(0.7f, 0.7f, 0.66f, 1.0f));
        // Ring
        buttonNode_->drawCircle(kc, knobR, 0, 16, false,
            cocos2d::Color4F(0.38f, 0.38f, 0.33f, 1.0f));
    }

    // Lower button rows (more settings)
    float lowerTop = knobSectionTop - knobR - 20.0f;
    for (int row = 0; row < 6; row++) {
        float rowY = lowerTop - row * (btnH + gap + 2.0f);
        drawButtonRow(buttonNode_, startX, rowY, cols, btnW, btnH, gap,
                      btnFace, greenLit);
    }
}

// ============================================================================
// Bottom-left fire command buttons
// ============================================================================

void ConsoleFrame::drawBottomLeftFireButtons()
{
    float hw = bezelW_ * 0.5f;
    float hh = bezelH_ * 0.5f;
    float scopeCenterY = bezelH_ * 0.15f;
    float r = scopeRadius_;

    // Fire buttons below and to the left of the scope
    float fireTop = scopeCenterY - r - 20.0f;
    float fireLeft = -hw + 14.0f;
    float fireRight = -20.0f;
    float fireW = fireRight - fireLeft;

    // Dark recessed panel background
    buttonNode_->drawSolidRect(
        cocos2d::Vec2(fireLeft - 2, -hh + 14.0f),
        cocos2d::Vec2(fireRight + 2, fireTop + 4),
        cocos2d::Color4F(0.18f, 0.18f, 0.15f, 1.0f));
    buttonNode_->drawRect(
        cocos2d::Vec2(fireLeft - 2, -hh + 14.0f),
        cocos2d::Vec2(fireRight + 2, fireTop + 4),
        cocos2d::Color4F(0.40f, 0.38f, 0.33f, 0.6f));

    // Fire command buttons — larger, prominent
    float fbtnW = 28.0f;
    float fbtnH = 18.0f;
    float fbtnGap = 4.0f;

    // Top row: ENGAGE, CEASE FIRE, HOLD FIRE
    const char* fireLabels1[] = { "ENG", "C/F", "HLD" };
    cocos2d::Color4F fireColors1[] = {
        cocos2d::Color4F(0.55f, 0.20f, 0.15f, 1.0f),  // red-ish
        cocos2d::Color4F(0.55f, 0.50f, 0.15f, 1.0f),  // amber
        cocos2d::Color4F(0.20f, 0.50f, 0.20f, 1.0f),  // green
    };

    float row1Y = fireTop - 10.0f;
    float row1StartX = fireLeft + (fireW - 3 * (fbtnW + fbtnGap) + fbtnGap) * 0.5f + fbtnW * 0.5f;

    for (int i = 0; i < 3; i++) {
        float bx = row1StartX + i * (fbtnW + fbtnGap);
        drawLabeledButton(buttonNode_, bx, row1Y, fbtnW, fbtnH,
                          fireColors1[i], fireLabels1[i], 6.0f);
    }

    // Second row: ARM, SAFE, IFF
    const char* fireLabels2[] = { "ARM", "SAFE", "IFF" };
    cocos2d::Color4F fireColors2[] = {
        cocos2d::Color4F(0.55f, 0.20f, 0.15f, 1.0f),
        cocos2d::Color4F(0.20f, 0.50f, 0.20f, 1.0f),
        cocos2d::Color4F(0.40f, 0.45f, 0.55f, 1.0f),
    };

    float row2Y = row1Y - fbtnH - fbtnGap - 4.0f;
    for (int i = 0; i < 3; i++) {
        float bx = row1StartX + i * (fbtnW + fbtnGap);
        drawLabeledButton(buttonNode_, bx, row2Y, fbtnW, fbtnH,
                          fireColors2[i], fireLabels2[i], 6.0f);
    }

    // Third row: TRK SEL, TRK DROP, ALERT
    const char* fireLabels3[] = { "SEL", "DRP", "ALT" };
    cocos2d::Color4F fireColor3(0.60f, 0.58f, 0.50f, 1.0f);

    float row3Y = row2Y - fbtnH - fbtnGap - 4.0f;
    for (int i = 0; i < 3; i++) {
        float bx = row1StartX + i * (fbtnW + fbtnGap);
        drawLabeledButton(buttonNode_, bx, row3Y, fbtnW, fbtnH,
                          fireColor3, fireLabels3[i], 6.0f);
    }

    // Fourth row: More function buttons
    const char* fireLabels4[] = { "RNG", "AZM", "ALT" };
    float row4Y = row3Y - fbtnH - fbtnGap - 4.0f;
    for (int i = 0; i < 3; i++) {
        float bx = row1StartX + i * (fbtnW + fbtnGap);
        drawLabeledButton(buttonNode_, bx, row4Y, fbtnW, fbtnH,
                          fireColor3, fireLabels4[i], 6.0f);
    }

    // Additional small button array below
    float smallTop = row4Y - fbtnH * 0.5f - 12.0f;
    float smallBtnW = 16.0f;
    float smallBtnH = 12.0f;
    int smallCols = std::max(1, (int)((fireW - 8) / (smallBtnW + 3)));
    float smallStartX = fireLeft + (fireW - smallCols * (smallBtnW + 3) + 3) * 0.5f;

    for (int row = 0; row < 3; row++) {
        float rowY = smallTop - row * (smallBtnH + 3.0f);
        drawButtonRow(buttonNode_, smallStartX, rowY, smallCols,
                      smallBtnW, smallBtnH, 3.0f,
                      cocos2d::Color4F(0.62f, 0.60f, 0.52f, 1.0f),
                      cocos2d::Color4F(0.40f, 0.25f, 0.08f, 0.4f));
    }
}

// ============================================================================
// Bottom-right joystick
// ============================================================================

void ConsoleFrame::drawBottomRightJoystick()
{
    float hw = bezelW_ * 0.5f;
    float hh = bezelH_ * 0.5f;

    // Joystick position: bottom-right of bezel
    float joyX = hw * 0.5f;
    float joyY = -hh + 55.0f;

    // Base plate
    buttonNode_->drawSolidCircle(
        cocos2d::Vec2(joyX, joyY), 22.0f, 0, 24,
        cocos2d::Color4F(0.16f, 0.16f, 0.14f, 1.0f));
    buttonNode_->drawCircle(
        cocos2d::Vec2(joyX, joyY), 22.0f, 0, 24, false,
        cocos2d::Color4F(0.38f, 0.36f, 0.30f, 1.0f));

    // Boot (rubber ring)
    buttonNode_->drawSolidCircle(
        cocos2d::Vec2(joyX, joyY), 15.0f, 0, 20,
        cocos2d::Color4F(0.10f, 0.10f, 0.08f, 1.0f));

    // Stick shaft
    buttonNode_->drawSolidCircle(
        cocos2d::Vec2(joyX, joyY), 6.0f, 0, 12,
        cocos2d::Color4F(0.25f, 0.25f, 0.22f, 1.0f));

    // Stick top (knob)
    buttonNode_->drawSolidCircle(
        cocos2d::Vec2(joyX, joyY + 2), 5.0f, 0, 12,
        cocos2d::Color4F(0.35f, 0.35f, 0.30f, 1.0f));
    buttonNode_->drawCircle(
        cocos2d::Vec2(joyX, joyY + 2), 5.0f, 0, 12, false,
        cocos2d::Color4F(0.45f, 0.43f, 0.38f, 1.0f));

    // Directional marks
    float markDist = 18.0f;
    cocos2d::Color4F markColor(0.45f, 0.43f, 0.38f, 0.6f);
    buttonNode_->drawLine(
        cocos2d::Vec2(joyX, joyY + markDist - 2),
        cocos2d::Vec2(joyX, joyY + markDist + 2), markColor);
    buttonNode_->drawLine(
        cocos2d::Vec2(joyX, joyY - markDist - 2),
        cocos2d::Vec2(joyX, joyY - markDist + 2), markColor);
    buttonNode_->drawLine(
        cocos2d::Vec2(joyX + markDist - 2, joyY),
        cocos2d::Vec2(joyX + markDist + 2, joyY), markColor);
    buttonNode_->drawLine(
        cocos2d::Vec2(joyX - markDist - 2, joyY),
        cocos2d::Vec2(joyX - markDist + 2, joyY), markColor);

    // Label
    auto* joyLabel = cocos2d::Label::createWithSystemFont("CURSOR", "Courier", 6);
    joyLabel->setPosition(cocos2d::Vec2(joyX, joyY - 28));
    joyLabel->setTextColor(cocos2d::Color4B(130, 130, 115, 180));
    buttonNode_->addChild(joyLabel);

    // Some small buttons next to joystick (thumb buttons)
    float tbX = joyX + 35.0f;
    cocos2d::Color4F tbColor(0.55f, 0.53f, 0.46f, 1.0f);
    drawLabeledButton(buttonNode_, tbX, joyY + 10, 16, 12, tbColor, "ACQ", 5.0f);
    drawLabeledButton(buttonNode_, tbX, joyY - 10, 16, 12, tbColor, "TGT", 5.0f);
}

// ============================================================================
// Top indicator row — status LEDs above the scope
// ============================================================================

void ConsoleFrame::drawTopIndicatorRow()
{
    float hh = bezelH_ * 0.5f;
    float scopeCenterY = bezelH_ * 0.15f;
    float r = scopeRadius_;

    float indicatorY = scopeCenterY + r + 12.0f;
    float indicatorTop = hh - 10.0f;

    // Background strip for indicators
    float stripLeft = -bezelW_ * 0.35f;
    float stripRight = bezelW_ * 0.35f;
    housingNode_->drawSolidRect(
        cocos2d::Vec2(stripLeft, indicatorY),
        cocos2d::Vec2(stripRight, indicatorTop),
        cocos2d::Color4F(0.20f, 0.20f, 0.17f, 1.0f));

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

        // LED housing
        housingNode_->drawSolidCircle(pos, 5.0f, 0, 10,
            cocos2d::Color4F(0.10f, 0.10f, 0.08f, 1.0f));

        // LED
        housingNode_->drawSolidCircle(pos, 3.5f, 0, 10,
            cocos2d::Color4F(ind.cr, ind.cg, ind.cb, 0.85f));

        // Glow
        housingNode_->drawSolidCircle(pos, 7.0f, 0, 12,
            cocos2d::Color4F(ind.cr, ind.cg, ind.cb, 0.12f));

        // Label below
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

    // Small plate at very bottom of bezel
    float plateY = -hh + 4.0f;
    housingNode_->drawSolidRect(
        cocos2d::Vec2(-55, plateY),
        cocos2d::Vec2(55, plateY + 12),
        cocos2d::Color4F(0.55f, 0.53f, 0.47f, 1.0f));
    housingNode_->drawRect(
        cocos2d::Vec2(-55, plateY),
        cocos2d::Vec2(55, plateY + 12),
        cocos2d::Color4F(0.45f, 0.43f, 0.38f, 1.0f));

    auto* mfgLabel = cocos2d::Label::createWithSystemFont(
        "HUGHES AIRCRAFT CO", "Courier", 5);
    mfgLabel->setPosition(cocos2d::Vec2(0, plateY + 6));
    mfgLabel->setTextColor(cocos2d::Color4B(40, 40, 35, 200));
    housingNode_->addChild(mfgLabel);

    auto* modelLabel = cocos2d::Label::createWithSystemFont(
        "AN/TSQ-72", "Courier", 5);
    modelLabel->setPosition(cocos2d::Vec2(0, plateY + 1));
    modelLabel->setTextColor(cocos2d::Color4B(40, 40, 35, 180));
    housingNode_->addChild(modelLabel);
}

// ============================================================================
// Dynamic indicators — lit buttons, status readouts (updated each frame)
// ============================================================================

void ConsoleFrame::drawDynamicIndicators()
{
    // Illuminate specific fire command buttons based on game state
    float hw = bezelW_ * 0.5f;
    float hh = bezelH_ * 0.5f;
    float scopeCenterY = bezelH_ * 0.15f;
    float r = scopeRadius_;

    // Fire button glow when engaged
    if (fireControl_) {
        auto batteries = fireControl_->getAllBatteryData();
        bool anyEngaged = false;
        bool anyReady = false;
        for (const auto& bat : batteries) {
            if (bat.status == BatteryStatus::ENGAGED) anyEngaged = true;
            if (bat.status == BatteryStatus::READY) anyReady = true;
        }

        // Glow the ENG button when batteries are firing
        float fireTop = scopeCenterY - r - 20.0f;
        float fireLeft = -hw + 14.0f;
        float fireRight = -20.0f;
        float fireW = fireRight - fireLeft;
        float fbtnW = 28.0f;
        float fbtnH = 18.0f;
        float fbtnGap = 4.0f;
        float row1StartX = fireLeft + (fireW - 3 * (fbtnW + fbtnGap) + fbtnGap) * 0.5f + fbtnW * 0.5f;
        float row1Y = fireTop - 10.0f;

        if (anyEngaged) {
            // Pulse the ENGAGE button
            float pulse = 0.5f + 0.5f * sinf((float)clock() / 200.0f);
            dynamicNode_->drawSolidRect(
                cocos2d::Vec2(row1StartX - fbtnW * 0.5f - 2, row1Y - fbtnH * 0.5f - 2),
                cocos2d::Vec2(row1StartX + fbtnW * 0.5f + 2, row1Y + fbtnH * 0.5f + 2),
                cocos2d::Color4F(0.8f, 0.15f, 0.1f, 0.25f * pulse));
        }
    }

    // Threat indicator on the right panel — light up some buttons
    if (threatBoard_ && threatBoard_->getThreatCount() > 0) {
        float panelLeft = r + 16.0f;
        float upperTop = scopeCenterY + r * 0.7f;
        float btnH = 14.0f;
        float gap = 3.0f;

        // Flash the top-right button row amber
        float pulse = 0.5f + 0.5f * sinf((float)clock() / 150.0f);
        float rowY = upperTop;
        dynamicNode_->drawSolidRect(
            cocos2d::Vec2(panelLeft, rowY - 2),
            cocos2d::Vec2(panelLeft + 100, rowY + btnH + 2),
            cocos2d::Color4F(0.6f, 0.35f, 0.05f, 0.15f * pulse));
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
    bezelW_ = scopeRadius * 2.0f + 240.0f;
    bezelH_ = bezelW_ * 1.25f;
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
