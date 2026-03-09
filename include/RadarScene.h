#ifndef __RADAR_SCENE_H__
#define __RADAR_SCENE_H__

#include "GameTypes.h"
#include "RadarDisplay.h"
#include "AircraftGenerator.h"
#include "TrackManager.h"
#include "FireControlSystem.h"
#include "GameHUD.h"
#include "GameConfig.h"
#include <vector>
#include <memory>

#if USE_COCOS2DX
#include "cocos2d.h"

// ============================================================================
// RadarScene - Main game scene containing the AN/TSQ-73 console
// ============================================================================

class RadarScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    static RadarScene* create();
    virtual bool init() override;

    void update(float dt) override;

private:
    // Core systems
    AircraftGenerator aircraftGenerator_;
    TrackManager trackManager_;
    FireControlSystem fireControlSystem_;
    GameConfig& gameConfig_;

    // Display elements
    RadarDisplay* radarDisplay_;
    GameHUD* gameHUD_;

    // Game state
    int score_;
    bool gameOver_;
    std::vector<std::unique_ptr<Aircraft>> aircraft_;

    // Initialization
    void initRadarDisplay();
    void initHUD();
    void initFireControl();
    void initInputHandlers();

    // Game loop
    void updateAircraft(float dt);
    void updateRadarSweep(float dt);
    void checkEngagements(float dt);
    void spawnAircraft(float dt);
    void cleanupDestroyedAircraft();
    void checkGameOver();

    // Input handling
    void onTrackSelected(int trackId);
    void onBatteryAssign(const std::string& batteryDesignation);
    void onFireAuthorized();
    void onAbortEngagement();

    // Scoring
    void addScore(int points);
    void onHostileDestroyed(Aircraft* aircraft);
    void onFriendlyDestroyed(Aircraft* aircraft);
    void onHostilePenetrated(Aircraft* aircraft);

    // Constructor helper to get GameConfig ref
    RadarScene();
};

#else
// Stub for development without cocos2d-x
class RadarScene {
public:
    static RadarScene* create();
    bool init();
    void update(float dt);

    // Stub run loop for testing without cocos2d-x
    void runGameLoop();

private:
    AircraftGenerator aircraftGenerator_;
    TrackManager trackManager_;
    FireControlSystem fireControlSystem_;

    int score_ = 0;
    bool gameOver_ = false;
    std::vector<std::unique_ptr<Aircraft>> aircraft_;

    RadarScene() = default;

    void spawnAircraft(float dt);
    void updateAircraft(float dt);
    void cleanupDestroyedAircraft();
};
#endif

#endif // __RADAR_SCENE_H__
