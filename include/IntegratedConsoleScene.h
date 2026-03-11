#ifndef __INTEGRATED_CONSOLE_SCENE_H__
#define __INTEGRATED_CONSOLE_SCENE_H__

#include "GameTypes.h"
#include "RadarDisplay.h"
#include "ConsoleFrame.h"
#include "AircraftGenerator.h"
#include "TrackManager.h"
#include "FireControlSystem.h"
#include "GameConfig.h"
#include "ThreatBoard.h"
#include "BattalionHQ.h"
#include <vector>
#include <memory>

#if USE_COCOS2DX
#include "cocos2d.h"

// ============================================================================
// IntegratedConsoleScene — single AN/TSQ-73 console with all information
// combined into one view.
//
// Layout:
//   - Center: PPI radar scope (RadarDisplay)
//   - Console housing: bezel, knobs, indicator LEDs (ConsoleFrame)
//   - Left panel:  Live track table
//   - Right panel: Battery status, HQ status, threat count
//   - Bottom bar:  Score, level, latest message
//
// Node hierarchy:
//   IntegratedConsoleScene (Scene)
//     └─ ConsoleFrame (z:0) — housing, background, live data panels
//     └─ RadarDisplay (z:1) — PPI scope, blips, sweep, trails
//
// This replaces both RadarScene (radar + separate HUD) and any dual-console
// scene. Everything is integrated into a single console view.
// ============================================================================

class IntegratedConsoleScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    static IntegratedConsoleScene* create();
    virtual bool init() override;

    void update(float dt) override;

private:
    // Core systems
    AircraftGenerator aircraftGenerator_;
    TrackManager trackManager_;
    FireControlSystem fireControlSystem_;
    ThreatBoard threatBoard_;
    BattalionHQ battalionHQ_;
    GameConfig& gameConfig_;

    // Display elements
    RadarDisplay* radarDisplay_;
    ConsoleFrame* consoleFrame_;

    // Game state
    int score_;
    bool gameOver_;
    std::vector<std::unique_ptr<Aircraft>> aircraft_;

    // Initialization
    void initConsole();
    void initInputHandlers();

    // Game loop
    void spawnAircraft(float dt);
    void updateAircraft(float dt);
    void checkEngagements(float dt);
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

    IntegratedConsoleScene();
};

#else
// ============================================================================
// Stub IntegratedConsoleScene (no cocos2d-x)
// ============================================================================
class IntegratedConsoleScene {
public:
    static IntegratedConsoleScene* create();
    bool init();
    void update(float dt);
    void runGameLoop();

private:
    AircraftGenerator aircraftGenerator_;
    TrackManager trackManager_;
    FireControlSystem fireControlSystem_;
    ThreatBoard threatBoard_;
    BattalionHQ battalionHQ_;

    int score_ = 0;
    bool gameOver_ = false;
    std::vector<std::unique_ptr<Aircraft>> aircraft_;

    IntegratedConsoleScene() = default;

    void spawnAircraft(float dt);
    void updateAircraft(float dt);
    void cleanupDestroyedAircraft();
};
#endif

#endif // __INTEGRATED_CONSOLE_SCENE_H__
