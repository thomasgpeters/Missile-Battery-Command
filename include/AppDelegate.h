#ifndef __APP_DELEGATE_H__
#define __APP_DELEGATE_H__

#if USE_COCOS2DX
#include "cocos2d.h"

class AppDelegate : private cocos2d::Application {
public:
    AppDelegate();
    virtual ~AppDelegate();

    virtual void initGLContextAttrs();
    virtual bool applicationDidFinishLaunching();
    virtual void applicationDidEnterBackground();
    virtual void applicationWillEnterForeground();
};

#else
// Stub for development without cocos2d-x
class AppDelegate {
public:
    AppDelegate();
    ~AppDelegate();

    void initGLContextAttrs();
    bool applicationDidFinishLaunching();
    void applicationDidEnterBackground();
    void applicationWillEnterForeground();

    // Stub run loop
    int run();
};
#endif

#endif // __APP_DELEGATE_H__
