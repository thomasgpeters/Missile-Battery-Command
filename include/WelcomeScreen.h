#ifndef __WELCOME_SCREEN_H__
#define __WELCOME_SCREEN_H__

#include "GameTypes.h"
#include <functional>

// ============================================================================
// WelcomeScreen — Octagonal modal dialog overlay
//
// Displays on game start as a framed octagonal dialog with the console
// visible but faded behind it. Click or press any key to dismiss.
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

    void drawFadedOverlay();
    void drawOctagonalDialog();
    void drawTitleContent();
    void drawPrompt();

    // Octagon helper — rectangle with 45-degree chamfered corners
    void drawOctagon(cocos2d::DrawNode* node,
                     const cocos2d::Vec2& origin,
                     const cocos2d::Vec2& dest,
                     float chamfer,
                     const cocos2d::Color4F& fill,
                     const cocos2d::Color4F& border,
                     float borderWidth = 1.5f);
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
