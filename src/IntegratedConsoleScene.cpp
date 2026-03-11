#include "IntegratedConsoleScene.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <thread>
#include <chrono>

#if USE_COCOS2DX
// ============================================================================
// Cocos2d-x Integrated Console Scene
// ============================================================================

IntegratedConsoleScene::IntegratedConsoleScene()
    : gameConfig_(GameConfig::getInstance())
    , radarDisplay_(nullptr)
    , consoleFrame_(nullptr)
    , score_(0)
    , gameOver_(false)
{
}

cocos2d::Scene* IntegratedConsoleScene::createScene()
{
    return IntegratedConsoleScene::create();
}

IntegratedConsoleScene* IntegratedConsoleScene::create()
{
    auto* ret = new (std::nothrow) IntegratedConsoleScene();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool IntegratedConsoleScene::init()
{
    if (!Scene::init()) return false;

    // Initialize game config
    gameConfig_.setLevel(1);

    // Initialize subsystems
    aircraftGenerator_.init(gameConfig_.getLevel());
    fireControlSystem_.init();
    battalionHQ_.init(0.5f, 0.0f);
    trackManager_.getIFFSystem().setErrorRate(gameConfig_.getIFFErrorRate());

    // Create the integrated console display
    initConsole();
    initInputHandlers();

    // Start game loop
    scheduleUpdate();
    return true;
}

void IntegratedConsoleScene::initConsole()
{
    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
    auto center = cocos2d::Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.5f);

    // AN/TSQ-72 portrait console: bezel is 25% taller than wide.
    // Size the radar scope so the full bezel fits on screen.
    // bezelW = 2*r + 240, bezelH = bezelW * 1.25
    float maxBezelW = visibleSize.width * 0.85f;
    float maxBezelH = visibleSize.height * 0.88f;
    // From bezelH = (2r+240)*1.25, solve for r:
    float rFromH = (maxBezelH / 1.25f - 240.0f) * 0.5f;
    float rFromW = (maxBezelW - 240.0f) * 0.5f;
    float radarRadius = std::max(80.0f, std::min(rFromH, rFromW));

    // Console frame — AN/TSQ-72 analog console housing
    consoleFrame_ = ConsoleFrame::create(radarRadius);
    consoleFrame_->setPosition(center);
    consoleFrame_->setTrackManager(&trackManager_);
    consoleFrame_->setFireControlSystem(&fireControlSystem_);
    consoleFrame_->setThreatBoard(&threatBoard_);
    consoleFrame_->setBattalionHQ(&battalionHQ_);
    consoleFrame_->setLevel(gameConfig_.getLevel());
    addChild(consoleFrame_, 0);

    // Radar display — PPI scope, offset to upper portion of bezel
    float scopeCenterY = consoleFrame_->getBezelHeight() * 0.15f;
    radarDisplay_ = RadarDisplay::create(radarRadius);
    radarDisplay_->setPosition(center + cocos2d::Vec2(0, scopeCenterY));
    radarDisplay_->setTrackManager(&trackManager_);
    radarDisplay_->setFireControlSystem(&fireControlSystem_);
    addChild(radarDisplay_, 1);
}

void IntegratedConsoleScene::initInputHandlers()
{
    // Touch/click to select tracks on radar
    auto listener = cocos2d::EventListenerTouchOneByOne::create();
    listener->onTouchBegan = [this](cocos2d::Touch* touch, cocos2d::Event* event) {
        auto touchPos = touch->getLocation();
        auto localPos = radarDisplay_->convertToNodeSpace(touchPos);

        int nearestTrackId = radarDisplay_->findNearestTrack(localPos, 20.0f);

        if (nearestTrackId >= 0) {
            onTrackSelected(nearestTrackId);
        } else {
            onTrackSelected(-1);
        }
        return true;
    };

    getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);

    // Keyboard for battery assignment and fire control
    auto keyListener = cocos2d::EventListenerKeyboard::create();
    keyListener->onKeyPressed = [this](cocos2d::EventKeyboard::KeyCode keyCode,
                                        cocos2d::Event* event) {
        switch (keyCode) {
            case cocos2d::EventKeyboard::KeyCode::KEY_1:
                onBatteryAssign("PATRIOT-1"); break;
            case cocos2d::EventKeyboard::KeyCode::KEY_2:
                onBatteryAssign("PATRIOT-2"); break;
            case cocos2d::EventKeyboard::KeyCode::KEY_3:
                onBatteryAssign("PATRIOT-3"); break;
            case cocos2d::EventKeyboard::KeyCode::KEY_4:
                onBatteryAssign("HAWK-1"); break;
            case cocos2d::EventKeyboard::KeyCode::KEY_5:
                onBatteryAssign("HAWK-2"); break;
            case cocos2d::EventKeyboard::KeyCode::KEY_6:
                onBatteryAssign("HAWK-3"); break;
            case cocos2d::EventKeyboard::KeyCode::KEY_7:
                onBatteryAssign("JAVELIN-1"); break;
            case cocos2d::EventKeyboard::KeyCode::KEY_8:
                onBatteryAssign("JAVELIN-2"); break;
            case cocos2d::EventKeyboard::KeyCode::KEY_9:
                onBatteryAssign("JAVELIN-3"); break;
            case cocos2d::EventKeyboard::KeyCode::KEY_F:
                onFireAuthorized(); break;
            case cocos2d::EventKeyboard::KeyCode::KEY_A:
                onAbortEngagement(); break;
            default:
                break;
        }
    };

    getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyListener, this);
}

void IntegratedConsoleScene::update(float dt)
{
    if (gameOver_) return;

    spawnAircraft(dt);
    updateAircraft(dt);
    trackManager_.update(dt);
    fireControlSystem_.update(dt);
    threatBoard_.update(trackManager_);
    battalionHQ_.update(dt);
    checkEngagements(dt);
    cleanupDestroyedAircraft();
    checkGameOver();
}

void IntegratedConsoleScene::spawnAircraft(float dt)
{
    int currentCount = 0;
    for (const auto& ac : aircraft_) {
        if (ac->isAlive()) currentCount++;
    }

    Aircraft* newAircraft = aircraftGenerator_.trySpawn(dt, currentCount);
    if (newAircraft) {
        trackManager_.addTrack(newAircraft);
        aircraft_.push_back(std::unique_ptr<Aircraft>(newAircraft));

        auto data = newAircraft->getTrackData();
        std::string msg = "NEW: " + data.getTrackIdString() +
                         " AZ:" + std::to_string((int)data.azimuth) +
                         " RNG:" + std::to_string((int)data.range) + "km";
        if (consoleFrame_) consoleFrame_->addMessage(msg);
    }
}

void IntegratedConsoleScene::updateAircraft(float dt)
{
    for (auto& ac : aircraft_) {
        if (ac->isAlive()) {
            ac->update(dt);

            if (ac->hasReachedTerritory() && !ac->isFriendly()) {
                onHostilePenetrated(ac.get());
                ac->destroy();
            }
        }
    }
}

void IntegratedConsoleScene::checkEngagements(float dt)
{
    auto results = fireControlSystem_.getRecentResults();
    for (const auto& result : results) {
        auto* aircraft = trackManager_.getAircraftByTrackId(result.trackId);
        std::string msg;

        if (result.result == EngagementResult::HIT) {
            msg = result.batteryDesignation + " -> " +
                  (aircraft ? aircraft->getTrackData().getTrackIdString() : "TK-???") +
                  " ** SPLASH **";
            if (aircraft) {
                if (aircraft->isFriendly()) {
                    onFriendlyDestroyed(aircraft);
                } else {
                    onHostileDestroyed(aircraft);
                }
            }
        } else if (result.result == EngagementResult::MISS) {
            msg = result.batteryDesignation + " -> MISS";
            addScore(GameConstants::SCORE_MISSILE_WASTED);
        }

        if (consoleFrame_ && !msg.empty()) {
            consoleFrame_->addMessage(msg);
        }
    }
}

void IntegratedConsoleScene::cleanupDestroyedAircraft()
{
    aircraft_.erase(
        std::remove_if(aircraft_.begin(), aircraft_.end(),
            [](const std::unique_ptr<Aircraft>& ac) {
                return !ac->isAlive() ||
                       ac->getRange() > GameConstants::RADAR_MAX_RANGE_KM * 1.1f;
            }),
        aircraft_.end());
}

void IntegratedConsoleScene::checkGameOver()
{
    if (score_ < -2000) {
        gameOver_ = true;
        if (consoleFrame_) {
            consoleFrame_->addMessage("=== GAME OVER - DEFENSE FAILED ===");
        }
    }
}

void IntegratedConsoleScene::onTrackSelected(int trackId)
{
    radarDisplay_->setSelectedTrack(trackId);
    if (consoleFrame_) consoleFrame_->setSelectedTrack(trackId);
}

void IntegratedConsoleScene::onBatteryAssign(const std::string& batteryDesignation)
{
    if (radarDisplay_->getSelectedTrack() < 0) return;

    auto* battery = fireControlSystem_.getBattery(batteryDesignation);
    auto* aircraft = trackManager_.getAircraftByTrackId(
        radarDisplay_->getSelectedTrack());

    if (battery && aircraft && battery->canEngage(aircraft)) {
        if (battery->engage(aircraft)) {
            std::string msg = batteryDesignation + " ENGAGING " +
                             aircraft->getTrackData().getTrackIdString();
            if (consoleFrame_) consoleFrame_->addMessage(msg);
        }
    }
}

void IntegratedConsoleScene::onFireAuthorized() {}
void IntegratedConsoleScene::onAbortEngagement() {}

void IntegratedConsoleScene::addScore(int points)
{
    score_ += points;
    if (consoleFrame_) consoleFrame_->setScore(score_);
}

void IntegratedConsoleScene::onHostileDestroyed(Aircraft* aircraft)
{
    int score = gameConfig_.getHostileDestroyedScore(aircraft->getType());
    addScore(score);
}

void IntegratedConsoleScene::onFriendlyDestroyed(Aircraft* aircraft)
{
    addScore(GameConstants::SCORE_FRIENDLY_DESTROYED);
    if (consoleFrame_) {
        consoleFrame_->addMessage("*** WARNING: FRIENDLY FIRE ***");
    }
}

void IntegratedConsoleScene::onHostilePenetrated(Aircraft* aircraft)
{
    addScore(GameConstants::SCORE_HOSTILE_PENETRATED);
    if (consoleFrame_) {
        consoleFrame_->addMessage("ALERT: HOSTILE PENETRATED DEFENSE ZONE");
    }
}

#else
// ============================================================================
// Stub Integrated Console Scene (no cocos2d-x) — Console-based simulation
// ============================================================================

IntegratedConsoleScene* IntegratedConsoleScene::create()
{
    return new IntegratedConsoleScene();
}

bool IntegratedConsoleScene::init()
{
    GameConfig::getInstance().setLevel(1);
    aircraftGenerator_.init(1);
    fireControlSystem_.init();
    battalionHQ_.init(0.5f, 0.0f);
    trackManager_.getIFFSystem().setErrorRate(0.0f);

    std::cout << "[IntegratedConsoleScene] AN/TSQ-73 Console initialized." << std::endl;
    std::cout << "  - Single integrated console view" << std::endl;
    std::cout << "  - Radar: AN/TPS-43E (" << (int)GameConstants::RADAR_MAX_RANGE_NM
              << " NM)" << std::endl;
    std::cout << "  - 3x Patriot, 3x Hawk, 3x Javelin" << std::endl;
    std::cout << std::endl;

    return true;
}

void IntegratedConsoleScene::update(float dt)
{
    spawnAircraft(dt);
    updateAircraft(dt);
    trackManager_.update(dt);
    fireControlSystem_.update(dt);
    threatBoard_.update(trackManager_);
    battalionHQ_.update(dt);
    cleanupDestroyedAircraft();
}

void IntegratedConsoleScene::spawnAircraft(float dt)
{
    int currentCount = 0;
    for (const auto& ac : aircraft_) {
        if (ac->isAlive()) currentCount++;
    }

    Aircraft* newAircraft = aircraftGenerator_.trySpawn(dt, currentCount);
    if (newAircraft) {
        trackManager_.addTrack(newAircraft);
        aircraft_.push_back(std::unique_ptr<Aircraft>(newAircraft));

        auto data = newAircraft->getTrackData();
        std::cout << "[NEW CONTACT] " << data.getTrackIdString()
                  << " | " << newAircraft->getTypeName()
                  << " | AZM:" << (int)data.azimuth << "\xC2\xB0"
                  << " | RNG:" << (int)(data.range * GameConstants::KM_TO_NM) << "NM"
                  << " | ALT:" << data.getAltitudeString()
                  << " | IFF:" << data.getClassificationString()
                  << std::endl;
    }
}

void IntegratedConsoleScene::updateAircraft(float dt)
{
    for (auto& ac : aircraft_) {
        if (ac->isAlive()) {
            ac->update(dt);
        }
    }
}

void IntegratedConsoleScene::cleanupDestroyedAircraft()
{
    aircraft_.erase(
        std::remove_if(aircraft_.begin(), aircraft_.end(),
            [](const std::unique_ptr<Aircraft>& ac) {
                return !ac->isAlive() ||
                       ac->getRange() > GameConstants::RADAR_MAX_RANGE_KM * 1.1f;
            }),
        aircraft_.end());
}

void IntegratedConsoleScene::runGameLoop()
{
    std::cout << "=== AN/TSQ-73 INTEGRATED CONSOLE ===" << std::endl;
    std::cout << "Running 60-second simulation..." << std::endl;
    std::cout << std::endl;

    const float dt = 1.0f;
    const float totalTime = 60.0f;

    for (float t = 0; t < totalTime; t += dt) {
        update(dt);

        if ((int)t % 5 == 0 && trackManager_.getActiveTrackCount() > 0) {
            std::cout << "\n--- TRACK TABLE [T+" << (int)t << "s] ---" << std::endl;
            std::cout << std::left
                      << std::setw(8) << "TRACK"
                      << std::setw(10) << "CLASS"
                      << std::setw(8) << "ALT"
                      << std::setw(8) << "AZM"
                      << std::setw(10) << "RNG(NM)"
                      << std::setw(10) << "SPD(kts)"
                      << std::setw(8) << "HDG"
                      << std::endl;

            auto tracks = trackManager_.getAllTracks();
            for (const auto& track : tracks) {
                std::cout << std::left
                          << std::setw(8) << track.getTrackIdString()
                          << std::setw(10) << track.getClassificationString()
                          << std::setw(8) << track.getAltitudeString()
                          << std::setw(8) << (int)track.azimuth
                          << std::setw(10) << std::fixed << std::setprecision(1)
                          << (track.range * GameConstants::KM_TO_NM)
                          << std::setw(10) << (int)track.speed
                          << std::setw(8) << (int)track.heading
                          << std::endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::cout << "\n--- BATTERY STATUS ---" << std::endl;
    auto batteries = fireControlSystem_.getAllBatteryData();
    for (const auto& bat : batteries) {
        std::cout << bat.designation << " | Missiles: "
                  << bat.missilesRemaining << "/" << bat.maxMissiles
                  << " | Status: READY" << std::endl;
    }

    std::cout << "\n=== SIMULATION COMPLETE ===" << std::endl;
    std::cout << "Final Score: " << score_ << std::endl;
}
#endif
