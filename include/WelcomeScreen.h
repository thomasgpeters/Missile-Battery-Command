#ifndef __WELCOME_SCREEN_H__
#define __WELCOME_SCREEN_H__

#include "GameTypes.h"
#include <functional>

// ============================================================================
// WelcomeScreen — Title screen overlay
//
// Displays on game start with the AN/TSQ-73 title banner and copyright.
// Click or press any key to dismiss and begin the game.
// ============================================================================

#if USE_COCOS2DX
#include "cocos2d.h"

class WelcomeScreen : public cocos2d::Node {
public:
    static WelcomeScreen* create(float screenW, float screenH);
    virtual bool init(float screenW, float screenH);

    void setOnDismiss(std::function<void()> cb) { onDismiss_ = cb; }
    void dismiss();

private:
    float screenW_;
    float screenH_;
    std::function<void()> onDismiss_;
    bool dismissed_;

    void drawBackground();
    void drawTitleBanner();
    void drawCopyright();
    void drawPrompt();
};

#else
// ============================================================================
// Stub WelcomeScreen (no cocos2d-x)
// ============================================================================
class WelcomeScreen {
public:
    static WelcomeScreen* create(float screenW, float screenH);
    bool init(float screenW, float screenH);

    void setOnDismiss(std::function<void()> cb) { onDismiss_ = cb; }
    void dismiss();

private:
    float screenW_ = 0;
    float screenH_ = 0;
    std::function<void()> onDismiss_;
    bool dismissed_ = false;
};
#endif

#endif // __WELCOME_SCREEN_H__
