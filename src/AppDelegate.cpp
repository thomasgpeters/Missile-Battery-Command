#include "AppDelegate.h"
#include "IntegratedConsoleScene.h"
#include "RadarScene.h"
#include <iostream>

#if USE_COCOS2DX
// ============================================================================
// Cocos2d-x Application Delegate
// ============================================================================

AppDelegate::AppDelegate() {}
AppDelegate::~AppDelegate() {}

void AppDelegate::initGLContextAttrs()
{
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8, 0};
    cocos2d::GLView::setGLContextAttrs(glContextAttrs);
}

bool AppDelegate::applicationDidFinishLaunching()
{
    auto director = cocos2d::Director::getInstance();
    auto glview = director->getOpenGLView();

    if (!glview) {
        glview = cocos2d::GLViewImpl::createWithRect(
            "Missile Battery Command - AN/TSQ-73",
            cocos2d::Rect(0, 0, 1280, 960));
        director->setOpenGLView(glview);
    }

    director->setDisplayStats(true);
    director->setAnimationInterval(1.0f / 60);

    // Use the integrated single-console scene
    auto scene = IntegratedConsoleScene::createScene();
    director->runWithScene(scene);

    return true;
}

void AppDelegate::applicationDidEnterBackground()
{
    cocos2d::Director::getInstance()->stopAnimation();
}

void AppDelegate::applicationWillEnterForeground()
{
    cocos2d::Director::getInstance()->startAnimation();
}

#else
// ============================================================================
// Stub Application (no cocos2d-x)
// ============================================================================

AppDelegate::AppDelegate() {}
AppDelegate::~AppDelegate() {}

void AppDelegate::initGLContextAttrs() {}

bool AppDelegate::applicationDidFinishLaunching()
{
    std::cout << "[AppDelegate] Application launching..." << std::endl;
    return true;
}

void AppDelegate::applicationDidEnterBackground()
{
    std::cout << "[AppDelegate] Entering background." << std::endl;
}

void AppDelegate::applicationWillEnterForeground()
{
    std::cout << "[AppDelegate] Entering foreground." << std::endl;
}

int AppDelegate::run()
{
    applicationDidFinishLaunching();

    std::cout << "[AppDelegate] Initializing game systems..." << std::endl;

    auto scene = IntegratedConsoleScene::create();
    if (scene && scene->init()) {
        std::cout << "[AppDelegate] IntegratedConsoleScene initialized." << std::endl;
        scene->runGameLoop();
    }

    delete scene;
    return 0;
}
#endif
