#include "WelcomeScreen.h"
#include <iostream>

#if USE_COCOS2DX
// ============================================================================
// Cocos2d-x WelcomeScreen — Octagonal modal dialog over faded console
// ============================================================================

WelcomeScreen* WelcomeScreen::create(float screenW, float screenH)
{
    auto* ret = new (std::nothrow) WelcomeScreen();
    if (ret && ret->init(screenW, screenH)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool WelcomeScreen::init(float screenW, float screenH)
{
    if (!Node::init()) return false;

    screenW_ = screenW;
    screenH_ = screenH;
    dismissed_ = false;

    drawFadedOverlay();
    drawOctagonalDialog();
    drawTitleContent();
    drawPrompt();

    return true;
}

// --------------------------------------------------------------------------
// Octagon drawing helper (same algorithm as ConsoleFrame)
// --------------------------------------------------------------------------
void WelcomeScreen::drawOctagon(cocos2d::DrawNode* node,
                                 const cocos2d::Vec2& origin,
                                 const cocos2d::Vec2& dest,
                                 float chamfer,
                                 const cocos2d::Color4F& fill,
                                 const cocos2d::Color4F& border,
                                 float borderWidth)
{
    float left   = std::min(origin.x, dest.x);
    float right  = std::max(origin.x, dest.x);
    float bottom = std::min(origin.y, dest.y);
    float top    = std::max(origin.y, dest.y);

    float c = std::min(chamfer, std::min((right - left) * 0.4f, (top - bottom) * 0.4f));

    cocos2d::Vec2 verts[] = {
        { left,         bottom + c },
        { left + c,     bottom },
        { right - c,    bottom },
        { right,        bottom + c },
        { right,        top - c },
        { right - c,    top },
        { left + c,     top },
        { left,         top - c }
    };

    node->drawPolygon(verts, 8, fill, borderWidth, border);
}

// --------------------------------------------------------------------------
// Semi-transparent fullscreen overlay — dims the console behind the dialog
// --------------------------------------------------------------------------
void WelcomeScreen::drawFadedOverlay()
{
    auto* overlay = cocos2d::DrawNode::create();

    // Dark semi-transparent overlay so console is visible but faded
    overlay->drawSolidRect(
        cocos2d::Vec2(0, 0),
        cocos2d::Vec2(screenW_, screenH_),
        cocos2d::Color4F(0.0f, 0.0f, 0.0f, 0.70f));

    addChild(overlay, 0);
}

// --------------------------------------------------------------------------
// Octagonal framed dialog — military style bezel
// --------------------------------------------------------------------------
void WelcomeScreen::drawOctagonalDialog()
{
    auto* dialog = cocos2d::DrawNode::create();

    float cx = screenW_ * 0.5f;
    float cy = screenH_ * 0.5f;

    // Dialog dimensions — sized to comfortably hold title content
    float dialogW = 520.0f;
    float dialogH = 380.0f;
    float chamfer = 60.0f;   // 45-degree corner cuts

    // --- Outer shadow/glow ---
    drawOctagon(dialog,
        cocos2d::Vec2(cx - dialogW * 0.5f - 6, cy - dialogH * 0.5f - 6),
        cocos2d::Vec2(cx + dialogW * 0.5f + 6, cy + dialogH * 0.5f + 6),
        chamfer + 4,
        cocos2d::Color4F(0.0f, 0.08f, 0.0f, 0.50f),
        cocos2d::Color4F(0.10f, 0.25f, 0.12f, 0.40f),
        2.0f);

    // --- Outer bezel ring (dark metal frame) ---
    drawOctagon(dialog,
        cocos2d::Vec2(cx - dialogW * 0.5f - 3, cy - dialogH * 0.5f - 3),
        cocos2d::Vec2(cx + dialogW * 0.5f + 3, cy + dialogH * 0.5f + 3),
        chamfer + 2,
        cocos2d::Color4F(0.08f, 0.12f, 0.08f, 0.95f),
        cocos2d::Color4F(0.18f, 0.32f, 0.20f, 0.90f),
        2.5f);

    // --- Main octagonal dialog face ---
    drawOctagon(dialog,
        cocos2d::Vec2(cx - dialogW * 0.5f, cy - dialogH * 0.5f),
        cocos2d::Vec2(cx + dialogW * 0.5f, cy + dialogH * 0.5f),
        chamfer,
        cocos2d::Color4F(0.03f, 0.05f, 0.03f, 0.94f),
        cocos2d::Color4F(0.22f, 0.40f, 0.25f, 0.85f),
        2.0f);

    // --- Inner border line (inset detail) ---
    float inset = 14.0f;
    drawOctagon(dialog,
        cocos2d::Vec2(cx - dialogW * 0.5f + inset, cy - dialogH * 0.5f + inset),
        cocos2d::Vec2(cx + dialogW * 0.5f - inset, cy + dialogH * 0.5f - inset),
        chamfer - inset * 0.7f,
        cocos2d::Color4F(0.0f, 0.0f, 0.0f, 0.0f),  // transparent fill
        cocos2d::Color4F(0.18f, 0.30f, 0.20f, 0.50f),
        1.0f);

    // --- Top highlight line on bezel face ---
    float topY = cy + dialogH * 0.5f - 2;
    float topChamferX = chamfer * 0.65f;
    dialog->drawLine(
        cocos2d::Vec2(cx - dialogW * 0.5f + topChamferX + 4, topY),
        cocos2d::Vec2(cx + dialogW * 0.5f - topChamferX - 4, topY),
        cocos2d::Color4F(0.30f, 0.50f, 0.35f, 0.25f));

    // --- Corner rivets/screws on the 4 cardinal flat faces ---
    float screwR = 3.0f;
    cocos2d::Color4F screwColor(0.15f, 0.22f, 0.16f, 0.80f);
    cocos2d::Color4F screwHL(0.25f, 0.38f, 0.28f, 0.60f);

    // Top face screws
    float screwY = cy + dialogH * 0.5f - 8;
    dialog->drawDot(cocos2d::Vec2(cx - 60, screwY), screwR, screwColor);
    dialog->drawDot(cocos2d::Vec2(cx + 60, screwY), screwR, screwColor);
    dialog->drawDot(cocos2d::Vec2(cx - 60, screwY), screwR * 0.4f, screwHL);
    dialog->drawDot(cocos2d::Vec2(cx + 60, screwY), screwR * 0.4f, screwHL);

    // Bottom face screws
    screwY = cy - dialogH * 0.5f + 8;
    dialog->drawDot(cocos2d::Vec2(cx - 60, screwY), screwR, screwColor);
    dialog->drawDot(cocos2d::Vec2(cx + 60, screwY), screwR, screwColor);
    dialog->drawDot(cocos2d::Vec2(cx - 60, screwY), screwR * 0.4f, screwHL);
    dialog->drawDot(cocos2d::Vec2(cx + 60, screwY), screwR * 0.4f, screwHL);

    // Left face screws
    float screwX = cx - dialogW * 0.5f + 8;
    dialog->drawDot(cocos2d::Vec2(screwX, cy - 50), screwR, screwColor);
    dialog->drawDot(cocos2d::Vec2(screwX, cy + 50), screwR, screwColor);
    dialog->drawDot(cocos2d::Vec2(screwX, cy - 50), screwR * 0.4f, screwHL);
    dialog->drawDot(cocos2d::Vec2(screwX, cy + 50), screwR * 0.4f, screwHL);

    // Right face screws
    screwX = cx + dialogW * 0.5f - 8;
    dialog->drawDot(cocos2d::Vec2(screwX, cy - 50), screwR, screwColor);
    dialog->drawDot(cocos2d::Vec2(screwX, cy + 50), screwR, screwColor);
    dialog->drawDot(cocos2d::Vec2(screwX, cy - 50), screwR * 0.4f, screwHL);
    dialog->drawDot(cocos2d::Vec2(screwX, cy + 50), screwR * 0.4f, screwHL);

    addChild(dialog, 1);
}

// --------------------------------------------------------------------------
// Title text content within the octagonal dialog
// --------------------------------------------------------------------------
void WelcomeScreen::drawTitleContent()
{
    float cx = screenW_ * 0.5f;
    float cy = screenH_ * 0.5f;

    // Branch designation
    auto* designation = cocos2d::Label::createWithSystemFont(
        "U.S. ARMY AIR DEFENSE ARTILLERY", "Courier", 10);
    designation->setPosition(cocos2d::Vec2(cx, cy + 130));
    designation->setTextColor(cocos2d::Color4B(120, 180, 120, 180));
    addChild(designation, 2);

    // Main title: AN/TSQ-73
    auto* title1 = cocos2d::Label::createWithSystemFont(
        "AN/TSQ-73", "Courier", 32);
    title1->setPosition(cocos2d::Vec2(cx, cy + 85));
    title1->setTextColor(cocos2d::Color4B(60, 255, 60, 255));
    addChild(title1, 2);

    // Main title: MISSILE MINDER
    auto* title2 = cocos2d::Label::createWithSystemFont(
        "MISSILE MINDER", "Courier", 24);
    title2->setPosition(cocos2d::Vec2(cx, cy + 48));
    title2->setTextColor(cocos2d::Color4B(60, 255, 60, 240));
    addChild(title2, 2);

    // Subtitle
    auto* subtitle = cocos2d::Label::createWithSystemFont(
        "COMMAND SIMULATION", "Courier", 16);
    subtitle->setPosition(cocos2d::Vec2(cx, cy + 15));
    subtitle->setTextColor(cocos2d::Color4B(255, 220, 80, 220));
    addChild(subtitle, 2);

    // Decorative separator with diamond accent
    auto* sepNode = cocos2d::DrawNode::create();
    float sepW = 200.0f;
    float sepY = cy - 8;
    sepNode->drawLine(
        cocos2d::Vec2(cx - sepW, sepY),
        cocos2d::Vec2(cx + sepW, sepY),
        cocos2d::Color4F(0.25f, 0.45f, 0.30f, 0.6f));
    // Diamond accent at center
    cocos2d::Vec2 diamond[] = {
        { cx, sepY + 5 }, { cx + 5, sepY }, { cx, sepY - 5 }, { cx - 5, sepY }
    };
    sepNode->drawPolygon(diamond, 4,
        cocos2d::Color4F(0.30f, 0.55f, 0.35f, 0.8f),
        0.5f, cocos2d::Color4F(0.40f, 0.65f, 0.45f, 0.6f));
    addChild(sepNode, 2);

    // System description
    auto* desc1 = cocos2d::Label::createWithSystemFont(
        "Fire Distribution Center", "Courier", 10);
    desc1->setPosition(cocos2d::Vec2(cx, cy - 28));
    desc1->setTextColor(cocos2d::Color4B(180, 190, 170, 180));
    addChild(desc1, 2);

    auto* desc2 = cocos2d::Label::createWithSystemFont(
        "Composite Air Defense Battalion", "Courier", 10);
    desc2->setPosition(cocos2d::Vec2(cx, cy - 45));
    desc2->setTextColor(cocos2d::Color4B(180, 190, 170, 180));
    addChild(desc2, 2);

    auto* desc3 = cocos2d::Label::createWithSystemFont(
        "3x Patriot  |  3x Hawk  |  3x Javelin", "Courier", 9);
    desc3->setPosition(cocos2d::Vec2(cx, cy - 65));
    desc3->setTextColor(cocos2d::Color4B(140, 160, 140, 160));
    addChild(desc3, 2);

    // Copyright
    auto* copyright = cocos2d::Label::createWithSystemFont(
        "\xC2\xA9 2026 Imagery Business Systems, LLC", "Courier", 8);
    copyright->setPosition(cocos2d::Vec2(cx, cy - 100));
    copyright->setTextColor(cocos2d::Color4B(120, 130, 115, 160));
    addChild(copyright, 2);

    auto* rights = cocos2d::Label::createWithSystemFont(
        "All Rights Reserved", "Courier", 7);
    rights->setPosition(cocos2d::Vec2(cx, cy - 114));
    rights->setTextColor(cocos2d::Color4B(100, 110, 95, 140));
    addChild(rights, 2);
}

// --------------------------------------------------------------------------
// Blinking prompt at bottom of dialog
// --------------------------------------------------------------------------
void WelcomeScreen::drawPrompt()
{
    float cx = screenW_ * 0.5f;
    float cy = screenH_ * 0.5f;
    float promptY = cy - 145.0f;

    auto* prompt = cocos2d::Label::createWithSystemFont(
        "[ CLICK OR PRESS ANY KEY TO BEGIN ]", "Courier", 10);
    prompt->setPosition(cocos2d::Vec2(cx, promptY));
    prompt->setTextColor(cocos2d::Color4B(180, 200, 180, 200));
    prompt->setTag(100);
    addChild(prompt, 2);

    // Blink the prompt
    auto blink = cocos2d::RepeatForever::create(
        cocos2d::Sequence::create(
            cocos2d::FadeOut::create(0.8f),
            cocos2d::FadeIn::create(0.8f),
            nullptr));
    prompt->runAction(blink);
}

void WelcomeScreen::dismiss()
{
    if (dismissed_) return;
    dismissed_ = true;

    auto fadeOut = cocos2d::FadeOut::create(0.5f);
    auto callback = cocos2d::CallFunc::create([this]() {
        if (onDismiss_) onDismiss_();
        removeFromParent();
    });

    // Fade out all children
    for (auto* child : getChildren()) {
        child->runAction(cocos2d::FadeOut::create(0.5f));
    }

    runAction(cocos2d::Sequence::create(
        cocos2d::DelayTime::create(0.5f),
        callback,
        nullptr));
}

#else
// ============================================================================
// Stub WelcomeScreen (no cocos2d-x)
// ============================================================================

WelcomeScreen* WelcomeScreen::create(float screenW, float screenH)
{
    auto* ret = new WelcomeScreen();
    if (ret->init(screenW, screenH)) return ret;
    delete ret;
    return nullptr;
}

bool WelcomeScreen::init(float screenW, float screenH)
{
    screenW_ = screenW;
    screenH_ = screenH;
    dismissed_ = false;

    std::cout << "============================================" << std::endl;
    std::cout << "  U.S. ARMY AIR DEFENSE ARTILLERY" << std::endl;
    std::cout << std::endl;
    std::cout << "         AN/TSQ-73" << std::endl;
    std::cout << "      MISSILE MINDER" << std::endl;
    std::cout << "    COMMAND SIMULATION" << std::endl;
    std::cout << std::endl;
    std::cout << "  (c) 2026 Imagery Business Systems, LLC" << std::endl;
    std::cout << "         All Rights Reserved" << std::endl;
    std::cout << "============================================" << std::endl;

    return true;
}

void WelcomeScreen::dismiss()
{
    if (dismissed_) return;
    dismissed_ = true;
    if (onDismiss_) onDismiss_();
}

#endif
