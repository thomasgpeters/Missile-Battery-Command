#include "RadarScene.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <thread>
#include <chrono>

#if USE_COCOS2DX
// ============================================================================
// Cocos2d-x Radar Scene
// ============================================================================

RadarScene::RadarScene()
    : gameConfig_(GameConfig::getInstance())
    , radarDisplay_(nullptr)
    , gameHUD_(nullptr)
    , score_(0)
    , gameOver_(false)
{
}

cocos2d::Scene* RadarScene::createScene()
{
    return RadarScene::create();
}

RadarScene* RadarScene::create()
{
    auto* ret = new (std::nothrow) RadarScene();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool RadarScene::init()
{
    if (!Scene::init()) return false;

    // Initialize game config
    gameConfig_.setLevel(1);

    // Initialize subsystems
    aircraftGenerator_.init(gameConfig_.getLevel());
    fireControlSystem_.init();

    // Set up IFF error rate from level config
    trackManager_.getIFFSystem().setErrorRate(gameConfig_.getIFFErrorRate());

    // Create display elements
    initRadarDisplay();
    initHUD();
    initInputHandlers();

    // Start game loop
    scheduleUpdate();
    return true;
}

void RadarScene::initRadarDisplay()
{
    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();

    // Radar takes up the left/center portion of the screen
    float radarRadius = std::min(visibleSize.width * 0.45f,
                                  visibleSize.height * 0.45f);

    radarDisplay_ = RadarDisplay::create(radarRadius);
    radarDisplay_->setPosition(cocos2d::Vec2(
        visibleSize.width * 0.4f, visibleSize.height * 0.5f));
    radarDisplay_->setTrackManager(&trackManager_);
    radarDisplay_->setFireControlSystem(&fireControlSystem_);
    addChild(radarDisplay_, 1);
}

void RadarScene::initHUD()
{
    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();

    gameHUD_ = GameHUD::create();
    gameHUD_->setPosition(cocos2d::Vec2(visibleSize.width * 0.75f,
                                         visibleSize.height * 0.95f));
    gameHUD_->setTrackManager(&trackManager_);
    gameHUD_->setFireControlSystem(&fireControlSystem_);
    gameHUD_->setLevel(gameConfig_.getLevel());
    addChild(gameHUD_, 2);
}

void RadarScene::initFireControl()
{
    fireControlSystem_.init();
}

void RadarScene::initInputHandlers()
{
    // Mouse/touch listener for selecting tracks on radar
    auto listener = cocos2d::EventListenerTouchOneByOne::create();
    listener->onTouchBegan = [this](cocos2d::Touch* touch, cocos2d::Event* event) {
        // Convert touch position to radar-local coordinates
        auto touchPos = touch->getLocation();
        auto localPos = radarDisplay_->convertToNodeSpace(touchPos);

        // Use RadarDisplay's findNearestTrack for accurate hit detection
        int nearestTrackId = radarDisplay_->findNearestTrack(localPos, 20.0f);

        if (nearestTrackId >= 0) {
            onTrackSelected(nearestTrackId);
        } else {
            // Clicked empty space — deselect
            onTrackSelected(-1);
        }
        return true;
    };

    getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);

    // Keyboard listener for fire control commands
    auto keyListener = cocos2d::EventListenerKeyboard::create();
    keyListener->onKeyPressed = [this](cocos2d::EventKeyboard::KeyCode keyCode,
                                        cocos2d::Event* event) {
        switch (keyCode) {
            // Number keys 1-6 for battery selection
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

void RadarScene::update(float dt)
{
    if (gameOver_) return;

    spawnAircraft(dt);
    updateAircraft(dt);
    trackManager_.update(dt);
    fireControlSystem_.update(dt);
    checkEngagements(dt);
    cleanupDestroyedAircraft();
    checkGameOver();
}

void RadarScene::spawnAircraft(float dt)
{
    int currentCount = 0;
    for (const auto& ac : aircraft_) {
        if (ac->isAlive()) currentCount++;
    }

    Aircraft* newAircraft = aircraftGenerator_.trySpawn(dt, currentCount);
    if (newAircraft) {
        int trackId = trackManager_.addTrack(newAircraft);
        aircraft_.push_back(std::unique_ptr<Aircraft>(newAircraft));

        std::string msg = "NEW CONTACT: " + newAircraft->getTrackData().getTrackIdString() +
                         " AZ:" + std::to_string((int)newAircraft->getAzimuth()) +
                         " RNG:" + std::to_string((int)newAircraft->getRange()) + "km";
        if (gameHUD_) gameHUD_->addMessage(msg);
    }
}

void RadarScene::updateAircraft(float dt)
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

void RadarScene::checkEngagements(float dt)
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

        if (gameHUD_ && !msg.empty()) {
            gameHUD_->addMessage(msg);
        }
    }
}

void RadarScene::cleanupDestroyedAircraft()
{
    aircraft_.erase(
        std::remove_if(aircraft_.begin(), aircraft_.end(),
            [](const std::unique_ptr<Aircraft>& ac) {
                return !ac->isAlive() ||
                       ac->getRange() > GameConstants::RADAR_MAX_RANGE_KM * 1.1f;
            }),
        aircraft_.end());
}

void RadarScene::checkGameOver()
{
    // Game over if score drops too low
    if (score_ < -2000) {
        gameOver_ = true;
        if (gameHUD_) {
            gameHUD_->addMessage("=== GAME OVER - DEFENSE FAILED ===");
        }
    }
}

void RadarScene::onTrackSelected(int trackId)
{
    radarDisplay_->setSelectedTrack(trackId);
    if (gameHUD_) gameHUD_->setSelectedTrack(trackId);
}

void RadarScene::onBatteryAssign(const std::string& batteryDesignation)
{
    if (radarDisplay_->getSelectedTrack() < 0) return;

    auto* battery = fireControlSystem_.getBattery(batteryDesignation);
    auto* aircraft = trackManager_.getAircraftByTrackId(
        radarDisplay_->getSelectedTrack());

    if (battery && aircraft && battery->canEngage(aircraft)) {
        if (battery->engage(aircraft)) {
            std::string msg = batteryDesignation + " ENGAGING " +
                             aircraft->getTrackData().getTrackIdString();
            if (gameHUD_) gameHUD_->addMessage(msg);
        }
    }
}

void RadarScene::onFireAuthorized() {}
void RadarScene::onAbortEngagement() {}

void RadarScene::addScore(int points)
{
    score_ += points;
    if (gameHUD_) gameHUD_->setScore(score_);
}

void RadarScene::onHostileDestroyed(Aircraft* aircraft)
{
    int score = gameConfig_.getHostileDestroyedScore(aircraft->getType());
    addScore(score);
}

void RadarScene::onFriendlyDestroyed(Aircraft* aircraft)
{
    addScore(GameConstants::SCORE_FRIENDLY_DESTROYED);
    if (gameHUD_) {
        gameHUD_->addMessage("*** WARNING: FRIENDLY FIRE ***");
    }
}

void RadarScene::onHostilePenetrated(Aircraft* aircraft)
{
    addScore(GameConstants::SCORE_HOSTILE_PENETRATED);
    if (gameHUD_) {
        gameHUD_->addMessage("ALERT: HOSTILE PENETRATED DEFENSE ZONE");
    }
}

#else
// ============================================================================
// Stub Radar Scene (no cocos2d-x) — Console-based simulation
// ============================================================================

RadarScene* RadarScene::create()
{
    return new RadarScene();
}

bool RadarScene::init()
{
    GameConfig::getInstance().setLevel(1);
    aircraftGenerator_.init(1);
    fireControlSystem_.init();
    trackManager_.getIFFSystem().setErrorRate(0.0f);

    std::cout << "[RadarScene] Systems initialized." << std::endl;
    std::cout << "  - Radar: Raytheon AN/TPS-43E long-range surveillance" << std::endl;
    std::cout << "  - Range: " << (int)GameConstants::RADAR_MAX_RANGE_NM
              << " NM (" << (int)GameConstants::RADAR_MAX_RANGE_KM << " km)" << std::endl;
    std::cout << "  - 3x Patriot Missile Batteries (MPMB) @ 160 km max" << std::endl;
    std::cout << "  - 3x Hawk SAM Batteries (HSAMB) @ 45 km max" << std::endl;
    std::cout << "  - Sweep rate: " << GameConstants::RADAR_SWEEP_RATE_RPM << " RPM" << std::endl;
    std::cout << std::endl;

    return true;
}

void RadarScene::update(float dt)
{
    spawnAircraft(dt);
    updateAircraft(dt);
    trackManager_.update(dt);
    fireControlSystem_.update(dt);
    cleanupDestroyedAircraft();
}

void RadarScene::spawnAircraft(float dt)
{
    int currentCount = 0;
    for (const auto& ac : aircraft_) {
        if (ac->isAlive()) currentCount++;
    }

    Aircraft* newAircraft = aircraftGenerator_.trySpawn(dt, currentCount);
    if (newAircraft) {
        int trackId = trackManager_.addTrack(newAircraft);
        aircraft_.push_back(std::unique_ptr<Aircraft>(newAircraft));

        auto data = newAircraft->getTrackData();
        std::cout << "[NEW CONTACT] " << data.getTrackIdString()
                  << " | Type: " << newAircraft->getTypeName()
                  << " | AZM: " << std::setw(3) << (int)data.azimuth << "\xC2\xB0"
                  << " | RNG: " << std::setw(3) << (int)(data.range * GameConstants::KM_TO_NM)
                  << " NM"
                  << " | ALT: " << data.getAltitudeString()
                  << " | SPD: " << (int)data.speed << " kts"
                  << " | IFF: " << data.getClassificationString()
                  << std::endl;
    }
}

void RadarScene::updateAircraft(float dt)
{
    for (auto& ac : aircraft_) {
        if (ac->isAlive()) {
            ac->update(dt);
        }
    }
}

void RadarScene::cleanupDestroyedAircraft()
{
    aircraft_.erase(
        std::remove_if(aircraft_.begin(), aircraft_.end(),
            [](const std::unique_ptr<Aircraft>& ac) {
                return !ac->isAlive() ||
                       ac->getRange() > GameConstants::RADAR_MAX_RANGE_KM * 1.1f;
            }),
        aircraft_.end());
}

void RadarScene::runGameLoop()
{
    std::cout << "=== AN/TSQ-73 RADAR CONSOLE ACTIVE ===" << std::endl;
    std::cout << "Running 60-second simulation..." << std::endl;
    std::cout << std::endl;

    const float dt = 1.0f;  // 1-second time steps for console output
    const float totalTime = 60.0f;

    for (float t = 0; t < totalTime; t += dt) {
        update(dt);

        // Print track table every 5 seconds
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

        // Small delay for readable console output
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // Final battery status
    std::cout << "\n--- BATTERY STATUS ---" << std::endl;
    auto batteries = fireControlSystem_.getAllBatteryData();
    for (const auto& bat : batteries) {
        std::cout << bat.designation << " | Missiles: "
                  << bat.missilesRemaining << "/" << bat.maxMissiles
                  << " | Status: READY" << std::endl;
    }

    std::cout << "\n=== SIMULATION COMPLETE ===" << std::endl;
    std::cout << "Final Score: " << score_ << std::endl;
    std::cout << "Tracks processed: " << trackManager_.getActiveTrackCount()
              << " active" << std::endl;
}
#endif
