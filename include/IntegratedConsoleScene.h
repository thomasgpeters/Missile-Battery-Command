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
#include "DrawerBook.h"
#include "WelcomeScreen.h"
#include <vector>
#include <memory>

#if USE_COCOS2DX
#include "cocos2d.h"

// ============================================================================
// Game state machine
// ============================================================================
enum class GameState {
    WELCOME,        // Title/welcome screen displayed
    RUNNING,        // Game in progress
    GAME_OVER,      // Game ended (loss condition)
    LEVEL_COMPLETE  // Current level cleared
};

// ============================================================================
// IntegratedConsoleScene — single AN/TSQ-73 console with all information
// combined into one view.
//
// Layout:
//   - Center: PPI radar scope (RadarDisplay)
//   - Console housing: bezel, knobs, indicator LEDs (ConsoleFrame)
//   - Left drawer:  Fire control reference book
//   - Right drawer: Settings/instructions reference book
//   - Bottom bar:  Score, level, timer, latest message
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

    // Drawer reference books
    DrawerBook* leftDrawer_;
    DrawerBook* rightDrawer_;

    // Welcome screen
    WelcomeScreen* welcomeScreen_;

    // Game state
    GameState gameState_;
    int score_;
    bool gameOver_;
    float gameTimer_;           // Elapsed game time in seconds
    float levelTimer_;          // Time remaining in current level
    int hostilesPenetrated_;    // Count for game-over condition
    int hostilesDestroyed_;     // Count for level-complete condition
    std::vector<std::unique_ptr<Aircraft>> aircraft_;

    // Initialization
    void initConsole();
    void initDrawers();
    void initInputHandlers();
    void initWelcomeScreen();
    void populateFireControlBook();
    void populateSettingsBook();

    // Game flow
    void startGame();
    void restartGame();
    void advanceLevel();
    float getLevelDuration(int level) const;
    int getLevelHostileTarget(int level) const;

    // Game loop
    void spawnAircraft(float dt);
    void updateAircraft(float dt);
    void checkEngagements(float dt);
    void cleanupDestroyedAircraft();
    void checkGameOver();
    void checkLevelComplete();
    void updateGameTimer(float dt);

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
enum class GameState { WELCOME, RUNNING, GAME_OVER, LEVEL_COMPLETE };

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

    GameState gameState_ = GameState::WELCOME;
    int score_ = 0;
    bool gameOver_ = false;
    float gameTimer_ = 0;
    float levelTimer_ = 0;
    int hostilesPenetrated_ = 0;
    int hostilesDestroyed_ = 0;
    std::vector<std::unique_ptr<Aircraft>> aircraft_;

    IntegratedConsoleScene() = default;

    void spawnAircraft(float dt);
    void updateAircraft(float dt);
    void cleanupDestroyedAircraft();
};
#endif

#endif // __INTEGRATED_CONSOLE_SCENE_H__
