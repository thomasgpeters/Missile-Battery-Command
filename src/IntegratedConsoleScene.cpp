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
    , leftDrawer_(nullptr)
    , rightDrawer_(nullptr)
    , welcomeScreen_(nullptr)
    , gameState_(GameState::WELCOME)
    , score_(0)
    , gameOver_(false)
    , gameTimer_(0.0f)
    , levelTimer_(0.0f)
    , hostilesPenetrated_(0)
    , hostilesDestroyed_(0)
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
    initDrawers();
    initInputHandlers();

    // Show welcome screen on top
    initWelcomeScreen();

    // Start game loop (paused during welcome)
    scheduleUpdate();
    return true;
}

// ============================================================================
// Level definitions — duration and hostile target count
// ============================================================================

float IntegratedConsoleScene::getLevelDuration(int level) const
{
    // Seconds per level
    switch (level) {
        case 1: return 120.0f;   // 2 minutes — learn the ropes
        case 2: return 150.0f;   // 2.5 minutes
        case 3: return 180.0f;   // 3 minutes
        case 4: return 210.0f;   // 3.5 minutes
        case 5: return 300.0f;   // 5 minutes — final stand
        default: return 180.0f;
    }
}

int IntegratedConsoleScene::getLevelHostileTarget(int level) const
{
    // Hostiles to destroy to complete the level
    switch (level) {
        case 1: return 5;
        case 2: return 10;
        case 3: return 18;
        case 4: return 25;
        case 5: return 40;
        default: return 15;
    }
}

// ============================================================================
// Welcome screen
// ============================================================================

void IntegratedConsoleScene::initWelcomeScreen()
{
    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
    welcomeScreen_ = WelcomeScreen::create(visibleSize.width, visibleSize.height);
    welcomeScreen_->setOnDismiss([this]() {
        welcomeScreen_ = nullptr;
        startGame();
    });
    addChild(welcomeScreen_, 100);
}

// ============================================================================
// Game flow
// ============================================================================

void IntegratedConsoleScene::startGame()
{
    gameState_ = GameState::RUNNING;
    score_ = 0;
    gameOver_ = false;
    gameTimer_ = 0.0f;
    levelTimer_ = getLevelDuration(gameConfig_.getLevel());
    hostilesPenetrated_ = 0;
    hostilesDestroyed_ = 0;

    if (consoleFrame_) {
        consoleFrame_->setScore(score_);
        consoleFrame_->setLevel(gameConfig_.getLevel());
        consoleFrame_->addMessage("=== DEFENSE OPERATION COMMENCED ===");
        consoleFrame_->addMessage("LEVEL 1 - TRAINING SCENARIO");
        std::string targetMsg = "OBJECTIVE: DESTROY " +
            std::to_string(getLevelHostileTarget(gameConfig_.getLevel())) + " HOSTILES";
        consoleFrame_->addMessage(targetMsg);
    }
}

void IntegratedConsoleScene::restartGame()
{
    // Clear all aircraft and tracks
    aircraft_.clear();
    trackManager_ = TrackManager();
    fireControlSystem_ = FireControlSystem();
    fireControlSystem_.init();

    // Reset to level 1
    gameConfig_.setLevel(1);
    aircraftGenerator_ = AircraftGenerator();
    aircraftGenerator_.init(1);
    trackManager_.getIFFSystem().setErrorRate(gameConfig_.getIFFErrorRate());

    battalionHQ_ = BattalionHQ();
    battalionHQ_.init(0.5f, 0.0f);

    // Reset display
    if (radarDisplay_) {
        radarDisplay_->setSelectedTrack(-1);
        radarDisplay_->setTrackManager(&trackManager_);
        radarDisplay_->setFireControlSystem(&fireControlSystem_);
    }
    if (consoleFrame_) {
        consoleFrame_->setTrackManager(&trackManager_);
        consoleFrame_->setFireControlSystem(&fireControlSystem_);
        consoleFrame_->setThreatBoard(&threatBoard_);
        consoleFrame_->setBattalionHQ(&battalionHQ_);
        consoleFrame_->setSelectedTrack(-1);
    }

    startGame();
}

void IntegratedConsoleScene::advanceLevel()
{
    int nextLevel = gameConfig_.getLevel() + 1;
    if (nextLevel > 5) {
        // Victory!
        gameState_ = GameState::GAME_OVER;
        if (consoleFrame_) {
            consoleFrame_->addMessage("========================================");
            consoleFrame_->addMessage("=== MISSION COMPLETE - VICTORY! ===");
            consoleFrame_->addMessage("FINAL SCORE: " + std::to_string(score_));
            consoleFrame_->addMessage("PRESS R TO RESTART");
        }
        return;
    }

    gameConfig_.setLevel(nextLevel);
    aircraftGenerator_ = AircraftGenerator();
    aircraftGenerator_.init(nextLevel);
    trackManager_.getIFFSystem().setErrorRate(gameConfig_.getIFFErrorRate());

    levelTimer_ = getLevelDuration(nextLevel);
    hostilesDestroyed_ = 0;
    hostilesPenetrated_ = 0;
    gameState_ = GameState::RUNNING;

    if (consoleFrame_) {
        consoleFrame_->setLevel(nextLevel);
        std::string lvlMsg = "=== LEVEL " + std::to_string(nextLevel) + " ===";
        consoleFrame_->addMessage(lvlMsg);
        std::string targetMsg = "OBJECTIVE: DESTROY " +
            std::to_string(getLevelHostileTarget(nextLevel)) + " HOSTILES";
        consoleFrame_->addMessage(targetMsg);
    }
}

void IntegratedConsoleScene::initConsole()
{
    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
    auto center = cocos2d::Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.5f);

    // AN/TSQ-73 console: landscape (wider than tall).
    float maxBezelW = visibleSize.width * 0.92f;
    float maxBezelH = visibleSize.height * 0.82f;
    float rFromW = (maxBezelW - 458.0f) * 0.5f;
    float rFromH = ((maxBezelH - 120.0f) / 1.25f - 50.0f) * 0.5f;
    float radarRadius = std::max(80.0f, std::min(rFromH, rFromW));

    // Console frame — AN/TSQ-73 analog console housing
    consoleFrame_ = ConsoleFrame::create(radarRadius);
    consoleFrame_->setPosition(center);
    consoleFrame_->setTrackManager(&trackManager_);
    consoleFrame_->setFireControlSystem(&fireControlSystem_);
    consoleFrame_->setThreatBoard(&threatBoard_);
    consoleFrame_->setBattalionHQ(&battalionHQ_);
    consoleFrame_->setLevel(gameConfig_.getLevel());
    addChild(consoleFrame_, 0);

    // Radar display — PPI scope, clipped to the portrait CRT glass opening
    float scopeCenterY = consoleFrame_->getScopeCenterY();
    radarDisplay_ = RadarDisplay::create(radarRadius);
    radarDisplay_->setTrackManager(&trackManager_);
    radarDisplay_->setFireControlSystem(&fireControlSystem_);

    // Build a stencil matching the CRT glass rounded-rectangle viewport
    float clipW = consoleFrame_->getDisplayWidth();
    float clipH = consoleFrame_->getDisplayHeight();
    float clipR = consoleFrame_->getDisplayCornerRadius();

    auto* stencil = cocos2d::DrawNode::create();
    {
        float hw = clipW * 0.5f;
        float hh = clipH * 0.5f;
        float r = std::min(clipR, std::min(hw, hh));
        const int arcSteps = 8;
        std::vector<cocos2d::Vec2> verts;

        for (int i = 0; i <= arcSteps; i++) {
            float angle = M_PI + (M_PI * 0.5f) * ((float)i / arcSteps);
            verts.push_back(cocos2d::Vec2(-hw + r + r * cosf(angle),
                                           -hh + r + r * sinf(angle)));
        }
        for (int i = 0; i <= arcSteps; i++) {
            float angle = M_PI * 1.5f + (M_PI * 0.5f) * ((float)i / arcSteps);
            verts.push_back(cocos2d::Vec2(hw - r + r * cosf(angle),
                                           -hh + r + r * sinf(angle)));
        }
        for (int i = 0; i <= arcSteps; i++) {
            float angle = 0.0f + (M_PI * 0.5f) * ((float)i / arcSteps);
            verts.push_back(cocos2d::Vec2(hw - r + r * cosf(angle),
                                           hh - r + r * sinf(angle)));
        }
        for (int i = 0; i <= arcSteps; i++) {
            float angle = M_PI * 0.5f + (M_PI * 0.5f) * ((float)i / arcSteps);
            verts.push_back(cocos2d::Vec2(-hw + r + r * cosf(angle),
                                           hh - r + r * sinf(angle)));
        }

        stencil->drawSolidPoly(verts.data(), (int)verts.size(),
                                cocos2d::Color4F::WHITE);
    }

    auto* clipNode = cocos2d::ClippingNode::create(stencil);
    clipNode->setAlphaThreshold(0.5f);
    clipNode->setPosition(center + cocos2d::Vec2(0, scopeCenterY));
    clipNode->addChild(radarDisplay_);
    addChild(clipNode, 1);

    // CRT glass reflection
    auto* glassReflection = cocos2d::DrawNode::create();
    {
        float hw = clipW * 0.5f;
        float hh = clipH * 0.5f;

        glassReflection->drawSolidRect(
            cocos2d::Vec2(-hw, hh * 0.15f),
            cocos2d::Vec2(hw, hh),
            cocos2d::Color4F(0.7f, 0.75f, 0.65f, 0.03f));

        glassReflection->drawSolidRect(
            cocos2d::Vec2(-hw * 0.7f, hh * 0.70f),
            cocos2d::Vec2(hw * 0.7f, hh * 0.85f),
            cocos2d::Color4F(0.8f, 0.85f, 0.75f, 0.04f));

        std::vector<cocos2d::Vec2> glare = {
            { -hw * 0.6f, hh * 0.85f },
            { -hw * 0.3f, hh * 0.90f },
            {  hw * 0.4f, hh * 0.30f },
            {  hw * 0.1f, hh * 0.25f }
        };
        glassReflection->drawSolidPoly(glare.data(), (int)glare.size(),
            cocos2d::Color4F(0.9f, 0.95f, 0.85f, 0.025f));
    }
    glassReflection->setPosition(center + cocos2d::Vec2(0, scopeCenterY));
    addChild(glassReflection, 2);
}

// ============================================================================
// Drawer reference books
// ============================================================================

void IntegratedConsoleScene::initDrawers()
{
    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
    float drawerW = 300.0f;
    float drawerH = visibleSize.height * 0.85f;

    // Left drawer — Fire Control reference
    leftDrawer_ = DrawerBook::create(DrawerSide::LEFT, drawerW, drawerH);
    leftDrawer_->setPositionY(visibleSize.height * 0.5f - drawerH * 0.5f);
    leftDrawer_->setOnOpen([this]() {
        if (rightDrawer_ && rightDrawer_->isOpen()) rightDrawer_->close();
    });
    populateFireControlBook();
    addChild(leftDrawer_, 50);

    // Right drawer — Settings/Instructions reference
    rightDrawer_ = DrawerBook::create(DrawerSide::RIGHT, drawerW, drawerH);
    rightDrawer_->setPositionY(visibleSize.height * 0.5f - drawerH * 0.5f);
    rightDrawer_->setOnOpen([this]() {
        if (leftDrawer_ && leftDrawer_->isOpen()) leftDrawer_->close();
    });
    populateSettingsBook();
    addChild(rightDrawer_, 50);
}

void IntegratedConsoleScene::populateFireControlBook()
{
    // Page 1: Fire Control Overview
    {
        BookPage page;
        page.title = "FIRE CONTROL COMMANDS";
        page.lines = {
            {"BATTERY ASSIGNMENT", TextColor::YELLOW, true},
            {""},
            {"Select a track on the radar scope by"},
            {"clicking on the blip. Then press a"},
            {"number key to assign a battery:"},
            {""},
            {"  1-3  PATRIOT batteries", TextColor::GREEN},
            {"  4-6  HAWK batteries", TextColor::GREEN},
            {"  7-9  JAVELIN platoons", TextColor::GREEN},
            {""},
            {"ENGAGEMENT CONTROLS", TextColor::YELLOW, true},
            {""},
            {"  F    Authorize fire", TextColor::GREEN},
            {"  A    Abort engagement", TextColor::GREEN},
            {"  P    Toggle phosphor color", TextColor::GREEN},
            {""},
            {"IFF PROCEDURE", TextColor::YELLOW, true},
            {""},
            {"All contacts start as PENDING."},
            {"Mode 4 interrogation takes ~2 sec."},
            {"FRIENDLY = valid squawk received.", TextColor::CYAN},
            {"HOSTILE  = no squawk response.", TextColor::RED},
            {"UNKNOWN  = inconclusive/jammed.", TextColor::YELLOW},
        };
        leftDrawer_->addPage(page);
    }

    // Page 2: Patriot Battery
    {
        BookPage page;
        page.title = "MIM-104 PATRIOT";
        page.lines = {
            {"PATRIOT-1, PATRIOT-2, PATRIOT-3", TextColor::GREEN, true},
            {""},
            {"Long-range, high-altitude backbone", TextColor::CYAN},
            {"of the defense. AN/MPQ-53 phased"},
            {"array tracking radar."},
            {""},
            {"SPECIFICATIONS", TextColor::YELLOW, true},
            {"  Max Range:     160 km"},
            {"  Min Range:       3 km"},
            {"  Altitude:    1,000 - 80,000 ft"},
            {"  Ready Msls:  4 per launcher"},
            {"  Missile Spd: 1,700 m/s (Mach 5)"},
            {"  Kill Prob:   ~85%"},
            {"  Reload Time: ~15 sec"},
            {""},
            {"BEST USED AGAINST:", TextColor::YELLOW, true},
            {"  Strategic bombers", TextColor::GREEN},
            {"  High-altitude fighters", TextColor::GREEN},
            {"  Stealth aircraft (long range)", TextColor::GREEN},
            {""},
            {"CAUTION: Do not waste Patriots on", TextColor::RED},
            {"low-altitude drones or targets", TextColor::RED},
            {"within Hawk envelope.", TextColor::RED},
        };
        leftDrawer_->addPage(page);
    }

    // Page 3: Hawk Battery
    {
        BookPage page;
        page.title = "MIM-23 HAWK";
        page.lines = {
            {"HAWK-1, HAWK-2, HAWK-3", TextColor::GREEN, true},
            {""},
            {"Medium-range, low-to-medium altitude", TextColor::CYAN},
            {"gap filler. AN/MPQ-46 High-Power"},
            {"Illuminator tracking radar."},
            {""},
            {"SPECIFICATIONS", TextColor::YELLOW, true},
            {"  Max Range:     45 km"},
            {"  Min Range:      1 km"},
            {"  Altitude:     100 - 45,000 ft"},
            {"  Ready Msls:   3 per launcher"},
            {"  Total Stock:  33 per battery"},
            {"  Missile Spd:  900 m/s (Mach 2.7)"},
            {"  Kill Prob:    ~75%"},
            {"  Reload Time:  ~10 sec"},
            {""},
            {"BEST USED AGAINST:", TextColor::YELLOW, true},
            {"  Tactical bombers", TextColor::GREEN},
            {"  Fighter/attack aircraft", TextColor::GREEN},
            {"  Medium-altitude threats", TextColor::GREEN},
        };
        leftDrawer_->addPage(page);
    }

    // Page 4: Javelin Platoon
    {
        BookPage page;
        page.title = "FGM-148 JAVELIN MANPADS";
        page.lines = {
            {"JAVELIN-1, JAVELIN-2, JAVELIN-3", TextColor::GREEN, true},
            {""},
            {"Last line of defense. IR-guided,", TextColor::CYAN},
            {"fire-and-forget. No radar signature."},
            {"CLU IR/FLIR seeker."},
            {""},
            {"SPECIFICATIONS", TextColor::YELLOW, true},
            {"  Max Range:     55 km (game)"},
            {"  Min Range:      0.5 km"},
            {"  Altitude:     GND - 15,000 ft"},
            {"  Ready Msls:   2"},
            {"  Missile Spd:  300 m/s"},
            {"  Kill Prob:    ~65%"},
            {"  Reload Time:  ~20 sec"},
            {""},
            {"KEY ADVANTAGE:", TextColor::YELLOW, true},
            {"IR guidance means stealth aircraft", TextColor::GREEN},
            {"gain NO benefit against Javelin.", TextColor::GREEN},
            {"The seeker tracks heat, not radar.", TextColor::GREEN},
            {""},
            {"Use Javelin for leakers that", TextColor::RED},
            {"penetrate the outer ring.", TextColor::RED},
        };
        leftDrawer_->addPage(page);
    }

    // Page 5: Scoring
    {
        BookPage page;
        page.title = "SCORING & GAME END";
        page.lines = {
            {"SCORING", TextColor::YELLOW, true},
            {""},
            {"  Hostile destroyed:  +100 base", TextColor::GREEN},
            {"    x type multiplier"},
            {"  First-shot kill:    2x bonus", TextColor::GREEN},
            {"  Hostile penetrates: -200", TextColor::RED},
            {"  Missile wasted:     -25", TextColor::YELLOW},
            {"  Friendly destroyed: GAME OVER", TextColor::RED, true},
            {""},
            {"GAME OVER CONDITIONS", TextColor::YELLOW, true},
            {""},
            {"  * Friendly aircraft destroyed", TextColor::RED},
            {"  * Score drops below -2000", TextColor::RED},
            {""},
            {"LEVEL COMPLETE", TextColor::YELLOW, true},
            {""},
            {"Destroy the target number of"},
            {"hostiles to advance to the next"},
            {"difficulty level."},
            {""},
            {"Press R to restart at any time.", TextColor::CYAN},
        };
        leftDrawer_->addPage(page);
    }
}

void IntegratedConsoleScene::populateSettingsBook()
{
    // Page 1: Controls overview
    {
        BookPage page;
        page.title = "OPERATOR CONTROLS";
        page.lines = {
            {"RADAR SCOPE", TextColor::YELLOW, true},
            {""},
            {"Click on radar blip to select track."},
            {"Selected track highlighted with"},
            {"bracket indicator."},
            {""},
            {"KEYBOARD COMMANDS", TextColor::YELLOW, true},
            {""},
            {"  1-9  Assign battery to track", TextColor::GREEN},
            {"  F    Authorize fire", TextColor::GREEN},
            {"  A    Abort engagement", TextColor::GREEN},
            {"  P    Toggle CRT phosphor color", TextColor::GREEN},
            {"  R    Restart game", TextColor::GREEN},
            {""},
            {"DISPLAY OPTIONS", TextColor::YELLOW, true},
            {""},
            {"  P1 GREEN  Standard radar CRT", TextColor::GREEN},
            {"  P39 AMBER Warm amber phosphor", TextColor::YELLOW},
            {""},
            {"Press P to cycle between phosphor"},
            {"display modes."},
        };
        rightDrawer_->addPage(page);
    }

    // Page 2: Level descriptions
    {
        BookPage page;
        page.title = "SCENARIO LEVELS";
        page.lines = {
            {"LEVEL 1 - TRAINING", TextColor::GREEN, true},
            {"  3 max contacts, slow spawn rate"},
            {"  No stealth, no IFF errors"},
            {"  Objective: Destroy 5 hostiles"},
            {""},
            {"LEVEL 2 - ALERT", TextColor::GREEN, true},
            {"  5 max contacts, faster spawns"},
            {"  No stealth, no IFF errors"},
            {"  Objective: Destroy 10 hostiles"},
            {""},
            {"LEVEL 3 - ENGAGEMENT", TextColor::YELLOW, true},
            {"  8 max contacts, rapid spawns"},
            {"  5% IFF error rate"},
            {"  Objective: Destroy 18 hostiles"},
            {""},
            {"LEVEL 4 - ESCALATION", TextColor::YELLOW, true},
            {"  10 max contacts, heavy pressure"},
            {"  Stealth enabled, 10% IFF errors"},
            {"  Objective: Destroy 25 hostiles"},
            {""},
            {"LEVEL 5 - FINAL STAND", TextColor::RED, true},
            {"  15 max contacts, overwhelming"},
            {"  Stealth enabled, 20% IFF errors"},
            {"  Objective: Destroy 40 hostiles"},
        };
        rightDrawer_->addPage(page);
    }

    // Page 3: Defense layout
    {
        BookPage page;
        page.title = "DEFENSE ZONE LAYOUT";
        page.lines = {
            {"BATTALION DEPLOYMENT", TextColor::YELLOW, true},
            {""},
            {"Territory radius: 25 km", TextColor::CYAN},
            {"Total footprint: ~75 km across"},
            {""},
            {"OUTER RING (35 km)", TextColor::GREEN, true},
            {"  PATRIOT-1  North"},
            {"  PATRIOT-2  Southeast"},
            {"  PATRIOT-3  Southwest"},
            {""},
            {"MIDDLE RING (20 km)", TextColor::GREEN, true},
            {"  HAWK-1  Northeast"},
            {"  HAWK-2  East"},
            {"  HAWK-3  South"},
            {""},
            {"INNER RING (8 km)", TextColor::GREEN, true},
            {"  JAVELIN-1  Northwest"},
            {"  JAVELIN-2  South"},
            {"  JAVELIN-3  Southwest"},
            {""},
            {"HQ: AN/TSQ-73 at center", TextColor::CYAN},
            {"Radar: AN/TPS-43E (250 NM range)"},
        };
        rightDrawer_->addPage(page);
    }

    // Page 4: Threat types
    {
        BookPage page;
        page.title = "THREAT IDENTIFICATION";
        page.lines = {
            {"HOSTILE AIRCRAFT", TextColor::RED, true},
            {""},
            {"  Strategic Bomber", TextColor::WHITE},
            {"    High alt, fast, large RCS"},
            {"  Fighter/Attack"},
            {"    Fast, agile, medium alt"},
            {"  Tactical Bomber"},
            {"    Low-med alt, moderate speed"},
            {"  Recon Drone"},
            {"    Small, slow, hard to classify"},
            {"  Attack Drone"},
            {"    Small, may carry ordnance"},
            {"  Stealth Fighter"},
            {"    Reduced RCS, harder to detect"},
            {""},
            {"DO NOT ENGAGE", TextColor::RED, true},
            {""},
            {"  Civilian Airliner", TextColor::RED},
            {"    -1000 pts / GAME OVER"},
            {"  Friendly Military", TextColor::RED},
            {"    GAME OVER on engagement"},
            {""},
            {"ALWAYS verify IFF before firing!", TextColor::YELLOW, true},
        };
        rightDrawer_->addPage(page);
    }
}

void IntegratedConsoleScene::initInputHandlers()
{
    // Touch/click to select tracks on radar AND interact with drawers
    auto listener = cocos2d::EventListenerTouchOneByOne::create();
    listener->onTouchBegan = [this](cocos2d::Touch* touch, cocos2d::Event* event) {
        auto touchPos = touch->getLocation();

        // Welcome screen — dismiss on click
        if (gameState_ == GameState::WELCOME && welcomeScreen_) {
            welcomeScreen_->dismiss();
            return true;
        }

        // Check drawer handles first
        if (leftDrawer_ && leftDrawer_->handleContainsPoint(touchPos)) {
            leftDrawer_->toggle();
            return true;
        }
        if (rightDrawer_ && rightDrawer_->handleContainsPoint(touchPos)) {
            rightDrawer_->toggle();
            return true;
        }

        // Check page navigation on open drawers
        if (leftDrawer_ && leftDrawer_->isOpen()) {
            auto zone = leftDrawer_->hitTest(touchPos);
            if (zone == DrawerBook::HitZone::PREV_PAGE) { leftDrawer_->prevPage(); return true; }
            if (zone == DrawerBook::HitZone::NEXT_PAGE) { leftDrawer_->nextPage(); return true; }
        }
        if (rightDrawer_ && rightDrawer_->isOpen()) {
            auto zone = rightDrawer_->hitTest(touchPos);
            if (zone == DrawerBook::HitZone::PREV_PAGE) { rightDrawer_->prevPage(); return true; }
            if (zone == DrawerBook::HitZone::NEXT_PAGE) { rightDrawer_->nextPage(); return true; }
        }

        // Radar track selection
        if (gameState_ == GameState::RUNNING && radarDisplay_) {
            auto localPos = radarDisplay_->convertToNodeSpace(touchPos);
            int nearestTrackId = radarDisplay_->findNearestTrack(localPos, 20.0f);

            if (nearestTrackId >= 0) {
                onTrackSelected(nearestTrackId);
            } else {
                onTrackSelected(-1);
            }
        }
        return true;
    };

    getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);

    // Keyboard for battery assignment and fire control
    auto keyListener = cocos2d::EventListenerKeyboard::create();
    keyListener->onKeyPressed = [this](cocos2d::EventKeyboard::KeyCode keyCode,
                                        cocos2d::Event* event) {
        // Welcome screen — any key dismisses
        if (gameState_ == GameState::WELCOME && welcomeScreen_) {
            welcomeScreen_->dismiss();
            return;
        }

        // Restart key works in any state
        if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_R) {
            restartGame();
            return;
        }

        // Game must be running for other commands
        if (gameState_ != GameState::RUNNING) return;

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
            case cocos2d::EventKeyboard::KeyCode::KEY_P:
                // Toggle phosphor color (P1 green <-> P39 amber)
                if (radarDisplay_) {
                    auto newColor = (radarDisplay_->getPhosphorColor() == PhosphorColor::GREEN)
                                     ? PhosphorColor::AMBER : PhosphorColor::GREEN;
                    radarDisplay_->setPhosphorColor(newColor);
                    if (consoleFrame_) {
                        consoleFrame_->addMessage(
                            newColor == PhosphorColor::AMBER
                            ? "PHOSPHOR: P39 AMBER" : "PHOSPHOR: P1 GREEN");
                    }
                }
                break;
            default:
                break;
        }
    };

    getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyListener, this);
}

void IntegratedConsoleScene::update(float dt)
{
    // Don't update game systems during welcome screen
    if (gameState_ == GameState::WELCOME) return;
    if (gameState_ == GameState::GAME_OVER) return;

    updateGameTimer(dt);
    spawnAircraft(dt);
    updateAircraft(dt);
    trackManager_.update(dt);
    fireControlSystem_.update(dt);
    threatBoard_.update(trackManager_);
    battalionHQ_.update(dt);
    checkEngagements(dt);
    cleanupDestroyedAircraft();
    checkLevelComplete();
    checkGameOver();
}

void IntegratedConsoleScene::updateGameTimer(float dt)
{
    gameTimer_ += dt;
    levelTimer_ -= dt;

    // Update timer display on console frame
    if (consoleFrame_) {
        int mins = (int)gameTimer_ / 60;
        int secs = (int)gameTimer_ % 60;
        // Timer is displayed via the score/level system
    }
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

void IntegratedConsoleScene::checkLevelComplete()
{
    int target = getLevelHostileTarget(gameConfig_.getLevel());
    if (hostilesDestroyed_ >= target) {
        if (consoleFrame_) {
            consoleFrame_->addMessage("=== LEVEL " +
                std::to_string(gameConfig_.getLevel()) + " COMPLETE ===");
            consoleFrame_->addMessage("HOSTILES DESTROYED: " +
                std::to_string(hostilesDestroyed_));
        }
        advanceLevel();
    }
}

void IntegratedConsoleScene::checkGameOver()
{
    if (score_ < -2000) {
        gameState_ = GameState::GAME_OVER;
        gameOver_ = true;
        if (consoleFrame_) {
            consoleFrame_->addMessage("=== GAME OVER - DEFENSE FAILED ===");
            consoleFrame_->addMessage("SCORE: " + std::to_string(score_));
            consoleFrame_->addMessage("PRESS R TO RESTART");
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
    hostilesDestroyed_++;
    aircraft->destroy();
}

void IntegratedConsoleScene::onFriendlyDestroyed(Aircraft* aircraft)
{
    addScore(GameConstants::SCORE_FRIENDLY_DESTROYED);
    aircraft->destroy();

    // Game over on friendly kill
    gameState_ = GameState::GAME_OVER;
    gameOver_ = true;
    if (consoleFrame_) {
        consoleFrame_->addMessage("*** FRIENDLY AIRCRAFT DESTROYED ***");
        consoleFrame_->addMessage("=== GAME OVER - FRIENDLY FIRE ===");
        consoleFrame_->addMessage("PRESS R TO RESTART");
    }
}

void IntegratedConsoleScene::onHostilePenetrated(Aircraft* aircraft)
{
    addScore(GameConstants::SCORE_HOSTILE_PENETRATED);
    hostilesPenetrated_++;
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
