#include "AppDelegate.h"

#if USE_COCOS2DX
#include "cocos2d.h"

int main(int argc, char** argv)
{
    AppDelegate app;
    return cocos2d::Application::getInstance()->run();
}

#else
// Stub main for development/testing without cocos2d-x
#include <iostream>
#include "RadarScene.h"

int main(int argc, char** argv)
{
    std::cout << "========================================" << std::endl;
    std::cout << "  MISSILE BATTERY COMMAND" << std::endl;
    std::cout << "  AN/TSQ-73 Air Defense Console" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Running in STUB mode (no cocos2d-x)" << std::endl;
    std::cout << "Build with cocos2d-x for full graphics." << std::endl;
    std::cout << std::endl;

    AppDelegate app;
    return app.run();
}
#endif
