#include "WelcomeScreen.h"
#include <iostream>

#if USE_COCOS2DX
// ============================================================================
// Cocos2d-x WelcomeScreen
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

    drawBackground();
    drawTitleBanner();
    drawCopyright();
    drawPrompt();

    return true;
}

void WelcomeScreen::drawBackground()
{
    auto* bg = cocos2d::DrawNode::create();

    // Full screen dark overlay
    bg->drawSolidRect(
        cocos2d::Vec2(0, 0),
        cocos2d::Vec2(screenW_, screenH_),
        cocos2d::Color4F(0.02f, 0.03f, 0.02f, 0.96f));

    // Subtle military green gradient band across center
    float bandH = 320.0f;
    float bandY = screenH_ * 0.5f - bandH * 0.5f;
    bg->drawSolidRect(
        cocos2d::Vec2(0, bandY),
        cocos2d::Vec2(screenW_, bandY + bandH),
        cocos2d::Color4F(0.04f, 0.06f, 0.04f, 1.0f));

    // Top and bottom border lines (military style)
    cocos2d::Color4F borderColor(0.20f, 0.35f, 0.22f, 0.8f);
    bg->drawLine(
        cocos2d::Vec2(screenW_ * 0.1f, bandY + bandH),
        cocos2d::Vec2(screenW_ * 0.9f, bandY + bandH),
        borderColor);
    bg->drawLine(
        cocos2d::Vec2(screenW_ * 0.1f, bandY),
        cocos2d::Vec2(screenW_ * 0.9f, bandY),
        borderColor);

    // Double line detail
    cocos2d::Color4F thinLine(0.15f, 0.25f, 0.18f, 0.5f);
    bg->drawLine(
        cocos2d::Vec2(screenW_ * 0.1f, bandY + bandH + 3),
        cocos2d::Vec2(screenW_ * 0.9f, bandY + bandH + 3),
        thinLine);
    bg->drawLine(
        cocos2d::Vec2(screenW_ * 0.1f, bandY - 3),
        cocos2d::Vec2(screenW_ * 0.9f, bandY - 3),
        thinLine);

    // Corner accents
    float accentLen = 30.0f;
    cocos2d::Color4F accentColor(0.30f, 0.50f, 0.35f, 0.7f);
    // Top-left
    bg->drawLine(cocos2d::Vec2(screenW_ * 0.1f, bandY + bandH),
                 cocos2d::Vec2(screenW_ * 0.1f, bandY + bandH + accentLen), accentColor);
    bg->drawLine(cocos2d::Vec2(screenW_ * 0.1f, bandY + bandH),
                 cocos2d::Vec2(screenW_ * 0.1f + accentLen, bandY + bandH), accentColor);
    // Top-right
    bg->drawLine(cocos2d::Vec2(screenW_ * 0.9f, bandY + bandH),
                 cocos2d::Vec2(screenW_ * 0.9f, bandY + bandH + accentLen), accentColor);
    bg->drawLine(cocos2d::Vec2(screenW_ * 0.9f, bandY + bandH),
                 cocos2d::Vec2(screenW_ * 0.9f - accentLen, bandY + bandH), accentColor);
    // Bottom-left
    bg->drawLine(cocos2d::Vec2(screenW_ * 0.1f, bandY),
                 cocos2d::Vec2(screenW_ * 0.1f, bandY - accentLen), accentColor);
    bg->drawLine(cocos2d::Vec2(screenW_ * 0.1f, bandY),
                 cocos2d::Vec2(screenW_ * 0.1f + accentLen, bandY), accentColor);
    // Bottom-right
    bg->drawLine(cocos2d::Vec2(screenW_ * 0.9f, bandY),
                 cocos2d::Vec2(screenW_ * 0.9f, bandY - accentLen), accentColor);
    bg->drawLine(cocos2d::Vec2(screenW_ * 0.9f, bandY),
                 cocos2d::Vec2(screenW_ * 0.9f - accentLen, bandY), accentColor);

    addChild(bg, 0);
}

void WelcomeScreen::drawTitleBanner()
{
    float centerX = screenW_ * 0.5f;
    float centerY = screenH_ * 0.5f;

    // System designation
    auto* designation = cocos2d::Label::createWithSystemFont(
        "U.S. ARMY AIR DEFENSE ARTILLERY", "Courier", 10);
    designation->setPosition(cocos2d::Vec2(centerX, centerY + 110));
    designation->setTextColor(cocos2d::Color4B(120, 180, 120, 180));
    addChild(designation, 1);

    // Main title line 1
    auto* title1 = cocos2d::Label::createWithSystemFont(
        "AN/TSQ-73", "Courier", 32);
    title1->setPosition(cocos2d::Vec2(centerX, centerY + 70));
    title1->setTextColor(cocos2d::Color4B(60, 255, 60, 255));
    addChild(title1, 1);

    // Main title line 2
    auto* title2 = cocos2d::Label::createWithSystemFont(
        "MISSILE MINDER", "Courier", 24);
    title2->setPosition(cocos2d::Vec2(centerX, centerY + 35));
    title2->setTextColor(cocos2d::Color4B(60, 255, 60, 240));
    addChild(title2, 1);

    // Subtitle
    auto* subtitle = cocos2d::Label::createWithSystemFont(
        "COMMAND SIMULATION", "Courier", 16);
    subtitle->setPosition(cocos2d::Vec2(centerX, centerY + 5));
    subtitle->setTextColor(cocos2d::Color4B(255, 220, 80, 220));
    addChild(subtitle, 1);

    // Decorative separator
    auto* sepNode = cocos2d::DrawNode::create();
    float sepW = 280.0f;
    sepNode->drawLine(
        cocos2d::Vec2(centerX - sepW, centerY - 18),
        cocos2d::Vec2(centerX + sepW, centerY - 18),
        cocos2d::Color4F(0.25f, 0.45f, 0.30f, 0.6f));
    // Diamond center accent
    float dy = centerY - 18;
    sepNode->drawSolidRect(
        cocos2d::Vec2(centerX - 4, dy - 4),
        cocos2d::Vec2(centerX + 4, dy + 4),
        cocos2d::Color4F(0.30f, 0.55f, 0.35f, 0.8f));
    addChild(sepNode, 1);

    // System description
    auto* desc1 = cocos2d::Label::createWithSystemFont(
        "Fire Distribution Center", "Courier", 10);
    desc1->setPosition(cocos2d::Vec2(centerX, centerY - 35));
    desc1->setTextColor(cocos2d::Color4B(180, 190, 170, 180));
    addChild(desc1, 1);

    auto* desc2 = cocos2d::Label::createWithSystemFont(
        "Composite Air Defense Battalion", "Courier", 10);
    desc2->setPosition(cocos2d::Vec2(centerX, centerY - 52));
    desc2->setTextColor(cocos2d::Color4B(180, 190, 170, 180));
    addChild(desc2, 1);

    auto* desc3 = cocos2d::Label::createWithSystemFont(
        "3x Patriot  |  3x Hawk  |  3x Javelin", "Courier", 9);
    desc3->setPosition(cocos2d::Vec2(centerX, centerY - 72));
    desc3->setTextColor(cocos2d::Color4B(140, 160, 140, 160));
    addChild(desc3, 1);
}

void WelcomeScreen::drawCopyright()
{
    float centerX = screenW_ * 0.5f;
    float bottomY = screenH_ * 0.5f - 130.0f;

    auto* copyright = cocos2d::Label::createWithSystemFont(
        "\xC2\xA9 2026 Imagery Business Systems, LLC", "Courier", 8);
    copyright->setPosition(cocos2d::Vec2(centerX, bottomY));
    copyright->setTextColor(cocos2d::Color4B(120, 130, 115, 160));
    addChild(copyright, 1);

    auto* rights = cocos2d::Label::createWithSystemFont(
        "All Rights Reserved", "Courier", 7);
    rights->setPosition(cocos2d::Vec2(centerX, bottomY - 14));
    rights->setTextColor(cocos2d::Color4B(100, 110, 95, 140));
    addChild(rights, 1);
}

void WelcomeScreen::drawPrompt()
{
    float centerX = screenW_ * 0.5f;
    float promptY = screenH_ * 0.5f - 100.0f;

    auto* prompt = cocos2d::Label::createWithSystemFont(
        "[ CLICK OR PRESS ANY KEY TO BEGIN ]", "Courier", 10);
    prompt->setPosition(cocos2d::Vec2(centerX, promptY));
    prompt->setTextColor(cocos2d::Color4B(180, 200, 180, 200));
    prompt->setTag(100);
    addChild(prompt, 1);

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
