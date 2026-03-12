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
    // Page 1: TM cover page
    {
        BookPage page;
        page.title = "TM 9-1430-600-10-1";
        page.lines = {
            {"TECHNICAL MANUAL", TextColor::YELLOW, true},
            {""},
            {"OPERATOR'S MANUAL", TextColor::YELLOW, true},
            {""},
            {"FIRE DISTRIBUTION CENTER", TextColor::GREEN, true},
            {"AN/TSQ-73", TextColor::GREEN, true},
            {"(MISSILE MINDER)", TextColor::GREEN, true},
            {""},
            {"HEADQUARTERS,", TextColor::WHITE},
            {"DEPARTMENT OF THE ARMY", TextColor::WHITE},
            {""},
            {"THIS PUBLICATION IS REQUIRED FOR"},
            {"OFFICIAL USE ONLY. DISTRIBUTION"},
            {"IS LIMITED TO U.S. GOVERNMENT"},
            {"AGENCIES AND THEIR CONTRACTORS."},
            {""},
            {"WARNING", TextColor::RED, true},
            {"Improper engagement sequencing can"},
            {"result in fratricide. Operators must"},
            {"verify IFF classification before"},
            {"authorizing fire on any contact.", TextColor::RED},
            {""},
            {"DESTRUCTION NOTICE: Destroy by any"},
            {"method that will prevent disclosure"},
            {"of contents or reconstruction."},
        };
        leftDrawer_->addPage(page);
    }

    // Page 2: AN/TSQ-73 System Description
    {
        BookPage page;
        page.title = "AN/TSQ-73 MISSILE MINDER";
        page.lines = {
            {"SECTION I. SYSTEM DESCRIPTION", TextColor::YELLOW, true},
            {""},
            {"1-1. PURPOSE", TextColor::GREEN, true},
            {"The AN/TSQ-73 is a mobile, tactical"},
            {"fire distribution center providing"},
            {"centralized air defense coordination"},
            {"for composite ADA battalions."},
            {""},
            {"1-2. NOMENCLATURE", TextColor::GREEN, true},
            {"  System:   AN/TSQ-73", TextColor::CYAN},
            {"  Name:     Missile Minder", TextColor::CYAN},
            {"  NSN:      5895-01-043-7212", TextColor::CYAN},
            {"  Mfr:      Litton Industries", TextColor::CYAN},
            {"            Data Systems Division"},
            {"  Shelter:  S-280 (on M927 5-ton)"},
            {"  Power:    MEP-006A (60kW, 440V)"},
            {""},
            {"1-3. CAPABILITIES", TextColor::GREEN, true},
            {"  * Simultaneous track management"},
            {"    of up to 99 air contacts"},
            {"  * Automatic IFF Mode 4 interrogation"},
            {"  * Centralized fire distribution to"},
            {"    9 organic fire units"},
            {"  * Dual L-112 processor fire control"},
        };
        leftDrawer_->addPage(page);
    }

    // Page 3: Console description
    {
        BookPage page;
        page.title = "OPERATOR CONSOLE";
        page.lines = {
            {"1-4. CONSOLE LAYOUT", TextColor::GREEN, true},
            {""},
            {"The operator station consists of a"},
            {"portrait-oriented CRT display with"},
            {"Plan Position Indicator (PPI) radar"},
            {"presentation, flanked by two control"},
            {"panels."},
            {""},
            {"a. CRT DISPLAY (CENTER)", TextColor::YELLOW, true},
            {"  P1 green or P39 amber phosphor"},
            {"  PPI presentation with 360-degree"},
            {"  rotating sweep, range rings, and"},
            {"  IFF contact symbology."},
            {""},
            {"b. LEFT CONTROL PANEL", TextColor::YELLOW, true},
            {"  Track management switches"},
            {"  Numeric keypad (data entry)"},
            {"  Fire control authorization buttons"},
            {""},
            {"c. RIGHT CONTROL PANEL", TextColor::YELLOW, true},
            {"  Display settings (gain/video)"},
            {"  Range/MTI configuration"},
            {"  Grid coordinate thumbwheels (8-digit)"},
            {"  Cursor control joystick"},
        };
        leftDrawer_->addPage(page);
    }

    // Page 4: Radar system
    {
        BookPage page;
        page.title = "AN/TPS-43E RADAR";
        page.lines = {
            {"1-5. SURVEILLANCE RADAR", TextColor::GREEN, true},
            {""},
            {"NOMENCLATURE", TextColor::YELLOW, true},
            {"  Designation: AN/TPS-43E", TextColor::CYAN},
            {"  Type:        3D Air Surveillance", TextColor::CYAN},
            {"  Mfr:         Raytheon Company", TextColor::CYAN},
            {"  Antenna:     AN/GSS-1 assembly", TextColor::CYAN},
            {""},
            {"SPECIFICATIONS", TextColor::YELLOW, true},
            {"  Max Range:      250 NM (463 km)"},
            {"  Sweep Rate:     6 RPM"},
            {"  Altitude:       100,000 ft max"},
            {"  Range Rings:    5 (equal interval)"},
            {"  Freq Band:      S-band"},
            {""},
            {"OPERATION", TextColor::YELLOW, true},
            {"The AN/TPS-43E provides long-range"},
            {"air surveillance and 3-dimensional"},
            {"track data (range, azimuth, altitude)"},
            {"to the AN/TSQ-73 via the AN/GSS-1"},
            {"antenna assembly. Track data is"},
            {"processed by dual L-112 computers"},
            {"and presented on the PPI scope."},
        };
        leftDrawer_->addPage(page);
    }

    // Page 5: PPI Scope / HUD description
    {
        BookPage page;
        page.title = "PPI SCOPE PRESENTATION";
        page.lines = {
            {"1-6. DISPLAY SYMBOLOGY", TextColor::GREEN, true},
            {""},
            {"CONTACT SYMBOLS", TextColor::YELLOW, true},
            {"  PENDING  Gray square", TextColor::WHITE},
            {"           IFF interrogation in progress"},
            {"  FRIENDLY Blue circle", TextColor::CYAN},
            {"           Valid Mode 4 squawk received"},
            {"  HOSTILE  Red diamond", TextColor::RED},
            {"           No squawk response"},
            {"  UNKNOWN  Yellow square", TextColor::YELLOW},
            {"           Jammed / inconclusive"},
            {""},
            {"TRACK DATA BLOCK", TextColor::YELLOW, true},
            {"Each classified track displays:"},
            {"  * Track ID (TK-001 thru TK-999)"},
            {"  * Velocity vector (speed + heading)"},
            {"  * Altitude readout"},
            {""},
            {"SCOPE FEATURES", TextColor::YELLOW, true},
            {"  * Rotating sweep beam (6 RPM)"},
            {"  * 5 range rings with distance labels"},
            {"  * Azimuth reference lines"},
            {"  * Territory defense zone ring"},
            {"  * Battery position markers"},
        };
        leftDrawer_->addPage(page);
    }

    // Page 6: MIM-104 PATRIOT
    {
        BookPage page;
        page.title = "MIM-104 PATRIOT SYSTEM";
        page.lines = {
            {"SECTION II. FIRE UNITS", TextColor::YELLOW, true},
            {""},
            {"2-1. PATRIOT AIR DEFENSE SYSTEM", TextColor::GREEN, true},
            {""},
            {"NOMENCLATURE", TextColor::YELLOW, true},
            {"  Missile:    MIM-104", TextColor::CYAN},
            {"  Radar:      AN/MPQ-53 Phased Array", TextColor::CYAN},
            {"  ECS:        AN/MSQ-104", TextColor::CYAN},
            {"  Launcher:   M901 (4 canisters)", TextColor::CYAN},
            {"  Mfr:        Raytheon Company", TextColor::CYAN},
            {""},
            {"SPECIFICATIONS", TextColor::YELLOW, true},
            {"  Max Range:     160 km"},
            {"  Min Range:       3 km"},
            {"  Altitude:    1,000 - 80,000 ft"},
            {"  Ready Msls:  4 per launcher"},
            {"  Missile Spd: 1,700 m/s (Mach 5)"},
            {"  Kill Prob:   ~85% (single shot)"},
            {"  Reload Time: ~15 sec"},
            {"  Guidance:    TVM (Track Via Missile)"},
            {"  Warhead:     Blast fragmentation"},
        };
        leftDrawer_->addPage(page);
    }

    // Page 7: Patriot tactical employment
    {
        BookPage page;
        page.title = "PATRIOT EMPLOYMENT";
        page.lines = {
            {"2-2. PATRIOT TACTICAL USE", TextColor::GREEN, true},
            {""},
            {"DEPLOYMENT", TextColor::YELLOW, true},
            {"  PATRIOT-1  North    (35 km)"},
            {"  PATRIOT-2  SE       (35 km)"},
            {"  PATRIOT-3  SW       (35 km)"},
            {"  Triangle formation covering all"},
            {"  high-altitude approach corridors."},
            {""},
            {"ENGAGEMENT PROCEDURE", TextColor::YELLOW, true},
            {"  1. Verify track IFF: HOSTILE", TextColor::GREEN},
            {"  2. Confirm altitude > 1,000 ft", TextColor::GREEN},
            {"  3. Select track on PPI scope", TextColor::GREEN},
            {"  4. Press 1, 2, or 3 to assign", TextColor::GREEN},
            {"  5. Battery engages autonomously", TextColor::GREEN},
            {""},
            {"RESTRICTIONS", TextColor::RED, true},
            {"  * DO NOT engage below 1,000 ft"},
            {"  * DO NOT engage within 3 km"},
            {"  * DO NOT waste on drone-class"},
            {"    targets within Hawk envelope", TextColor::RED},
            {"  * Conserve missiles for high-value"},
            {"    threats (bombers, stealth)", TextColor::RED},
        };
        leftDrawer_->addPage(page);
    }

    // Page 8: MIM-23 HAWK
    {
        BookPage page;
        page.title = "MIM-23 HAWK SYSTEM";
        page.lines = {
            {"2-3. IMPROVED HAWK AIR DEFENSE", TextColor::GREEN, true},
            {""},
            {"NOMENCLATURE", TextColor::YELLOW, true},
            {"  Missile:    MIM-23B I-HAWK", TextColor::CYAN},
            {"  Acq Radar:  AN/MPQ-50 CWAR", TextColor::CYAN},
            {"  Trk Radar:  AN/MPQ-46 HPI", TextColor::CYAN},
            {"  Illuminator:AN/MPQ-57 ROR", TextColor::CYAN},
            {"  Launcher:   M192 (3 rails)", TextColor::CYAN},
            {"  Mfr:        Raytheon Company", TextColor::CYAN},
            {""},
            {"SPECIFICATIONS", TextColor::YELLOW, true},
            {"  Max Range:     45 km"},
            {"  Min Range:      1 km"},
            {"  Altitude:     100 - 45,000 ft"},
            {"  Ready Msls:   3 per launcher"},
            {"  Total Stock:  33 per battery"},
            {"  Missile Spd:  900 m/s (Mach 2.7)"},
            {"  Kill Prob:    ~75% (single shot)"},
            {"  Reload Time:  ~10 sec"},
            {"  Guidance:     Semi-active radar homing"},
            {"  Warhead:      HE blast fragmentation"},
        };
        leftDrawer_->addPage(page);
    }

    // Page 9: Hawk tactical employment
    {
        BookPage page;
        page.title = "HAWK EMPLOYMENT";
        page.lines = {
            {"2-4. HAWK TACTICAL USE", TextColor::GREEN, true},
            {""},
            {"DEPLOYMENT", TextColor::YELLOW, true},
            {"  HAWK-1  NE    (20 km)"},
            {"  HAWK-2  East  (20 km)"},
            {"  HAWK-3  South (20 km)"},
            {"  Gap-filler ring between Patriot"},
            {"  and inner defense perimeter."},
            {""},
            {"ENGAGEMENT PROCEDURE", TextColor::YELLOW, true},
            {"  1. Verify track IFF: HOSTILE", TextColor::GREEN},
            {"  2. Confirm within Hawk envelope", TextColor::GREEN},
            {"  3. Select track on PPI scope", TextColor::GREEN},
            {"  4. Press 4, 5, or 6 to assign", TextColor::GREEN},
            {"  5. HPI illuminates, missile fires", TextColor::GREEN},
            {""},
            {"BEST EMPLOYED AGAINST", TextColor::YELLOW, true},
            {"  * Tactical bombers (low-med alt)", TextColor::GREEN},
            {"  * Fighter/attack aircraft", TextColor::GREEN},
            {"  * Targets too low for Patriot", TextColor::GREEN},
            {""},
            {"NOTE: Hawk battery has deepest ammo", TextColor::CYAN},
            {"stock (33 msls). Preferred for", TextColor::CYAN},
            {"sustained engagement.", TextColor::CYAN},
        };
        leftDrawer_->addPage(page);
    }

    // Page 10: FGM-148 JAVELIN
    {
        BookPage page;
        page.title = "FGM-148 JAVELIN MANPADS";
        page.lines = {
            {"2-5. JAVELIN MAN-PORTABLE SYSTEM", TextColor::GREEN, true},
            {""},
            {"NOMENCLATURE", TextColor::YELLOW, true},
            {"  Missile:    FGM-148 Javelin", TextColor::CYAN},
            {"  CLU:        Command Launch Unit", TextColor::CYAN},
            {"  Seeker:     IR/FLIR (dual-band)", TextColor::CYAN},
            {"  Mfr:        Javelin Joint Venture", TextColor::CYAN},
            {"              Raytheon / Lockheed Martin", TextColor::CYAN},
            {""},
            {"SPECIFICATIONS", TextColor::YELLOW, true},
            {"  Max Range:     55 km (sim value)"},
            {"  Min Range:      0.5 km"},
            {"  Altitude:     GND - 15,000 ft"},
            {"  Ready Msls:   2 per team"},
            {"  Missile Spd:  300 m/s"},
            {"  Kill Prob:    ~65% (single shot)"},
            {"  Reload Time:  ~20 sec"},
            {"  Guidance:     IR fire-and-forget"},
            {"  Warhead:      Tandem shaped charge"},
            {""},
            {"KEY: IR guidance defeats stealth.", TextColor::GREEN, true},
            {"Stealth RCS reduction has ZERO", TextColor::GREEN},
            {"effect against Javelin CLU seeker.", TextColor::GREEN},
        };
        leftDrawer_->addPage(page);
    }

    // Page 11: Javelin tactical employment
    {
        BookPage page;
        page.title = "JAVELIN EMPLOYMENT";
        page.lines = {
            {"2-6. JAVELIN TACTICAL USE", TextColor::GREEN, true},
            {""},
            {"DEPLOYMENT", TextColor::YELLOW, true},
            {"  JAVELIN-1  NW    (8 km)"},
            {"  JAVELIN-2  South (8 km)"},
            {"  JAVELIN-3  SW    (8 km)"},
            {"  Inner ring, last line of defense"},
            {"  before territory penetration."},
            {""},
            {"ENGAGEMENT PROCEDURE", TextColor::YELLOW, true},
            {"  1. Verify track IFF: HOSTILE", TextColor::GREEN},
            {"  2. Confirm low-altitude leaker", TextColor::GREEN},
            {"  3. Select track on PPI scope", TextColor::GREEN},
            {"  4. Press 7, 8, or 9 to assign", TextColor::GREEN},
            {"  5. CLU acquires IR lock, fires", TextColor::GREEN},
            {""},
            {"WARNING", TextColor::RED, true},
            {"Javelin has limited missile stock", TextColor::RED},
            {"(2 ready rounds). Reserve for", TextColor::RED},
            {"confirmed hostile leakers only.", TextColor::RED},
            {"Do NOT expend on distant targets", TextColor::RED},
            {"that Patriot or Hawk can engage.", TextColor::RED},
        };
        leftDrawer_->addPage(page);
    }

    // Page 12: Fire distribution procedure
    {
        BookPage page;
        page.title = "FIRE DISTRIBUTION";
        page.lines = {
            {"SECTION III. PROCEDURES", TextColor::YELLOW, true},
            {""},
            {"3-1. ENGAGEMENT SEQUENCE", TextColor::GREEN, true},
            {""},
            {"  CONTACT DETECTED", TextColor::CYAN, true},
            {"       |"},
            {"       v"},
            {"  IFF INTERROGATION (2 sec)", TextColor::CYAN, true},
            {"       |"},
            {"   +---+---+---+"},
            {"   |       |       |", TextColor::WHITE},
            {" FRIEND  HOST   UNKN", TextColor::WHITE},
            {" (blue)  (red)  (yel)", TextColor::WHITE},
            {"   |       |       |"},
            {"  HOLD   EVAL   MONITOR"},
            {"          |"},
            {"          v"},
            {"  SELECT BATTERY", TextColor::YELLOW, true},
            {"  (by range/alt match)"},
            {"       |"},
            {"       v"},
            {"  ASSIGN (key 1-9)", TextColor::GREEN, true},
            {"       |"},
            {"       v"},
            {"  SPLASH / MISS", TextColor::RED, true},
        };
        leftDrawer_->addPage(page);
    }

    // Page 13: Battery selection guide
    {
        BookPage page;
        page.title = "BATTERY SELECTION GUIDE";
        page.lines = {
            {"3-2. WEAPON-TARGET MATCHING", TextColor::GREEN, true},
            {""},
            {"DECISION MATRIX", TextColor::YELLOW, true},
            {""},
            {"HIGH ALT + LONG RANGE:", TextColor::WHITE, true},
            {"  -> PATRIOT (keys 1-3)", TextColor::GREEN},
            {"  Strategic bombers, high fighters"},
            {""},
            {"MED ALT + MED RANGE:", TextColor::WHITE, true},
            {"  -> HAWK (keys 4-6)", TextColor::GREEN},
            {"  Tactical bombers, attack jets"},
            {""},
            {"LOW ALT + SHORT RANGE:", TextColor::WHITE, true},
            {"  -> JAVELIN (keys 7-9)", TextColor::GREEN},
            {"  Leakers, drones, stealth"},
            {""},
            {"STEALTH TARGETS:", TextColor::WHITE, true},
            {"  -> JAVELIN preferred (IR defeats", TextColor::GREEN},
            {"     RCS reduction)", TextColor::GREEN},
            {"  -> PATRIOT at long range if high", TextColor::GREEN},
            {""},
            {"RULE: Never use Patriot when Hawk", TextColor::RED},
            {"can reach. Never use Hawk when", TextColor::RED},
            {"Javelin can reach.", TextColor::RED},
        };
        leftDrawer_->addPage(page);
    }

    // Page 14: IFF Procedures
    {
        BookPage page;
        page.title = "IFF PROCEDURES";
        page.lines = {
            {"3-3. IDENTIFICATION FRIEND OR FOE", TextColor::GREEN, true},
            {""},
            {"MODE 4 INTERROGATION", TextColor::YELLOW, true},
            {"AN/TSQ-73 automatically transmits"},
            {"Mode 4 crypto challenge to each new"},
            {"contact. Response time: ~2 seconds."},
            {""},
            {"SQUAWK PRINCIPLE", TextColor::YELLOW, true},
            {"Friendly aircraft carry IFF"},
            {"transponders that respond (squawk)"},
            {"with valid authentication codes.", TextColor::CYAN},
            {"Hostile aircraft have no valid"},
            {"transponder. Absence of squawk =", TextColor::RED},
            {"HOSTILE classification.", TextColor::RED},
            {""},
            {"IFF ERROR RATES BY LEVEL", TextColor::YELLOW, true},
            {"  Level 1:  0% error", TextColor::GREEN},
            {"  Level 2:  0% error", TextColor::GREEN},
            {"  Level 3:  5% error", TextColor::YELLOW},
            {"  Level 4: 10% error", TextColor::YELLOW},
            {"  Level 5: 20% error", TextColor::RED},
            {""},
            {"CAUTION: At high error rates, cross-", TextColor::RED},
            {"reference speed, altitude, heading", TextColor::RED},
            {"before engaging UNKNOWN contacts.", TextColor::RED},
        };
        leftDrawer_->addPage(page);
    }

    // Page 15: Scoring
    {
        BookPage page;
        page.title = "SCORING";
        page.lines = {
            {"3-4. SCORING SYSTEM", TextColor::GREEN, true},
            {""},
            {"POSITIVE ACTIONS", TextColor::YELLOW, true},
            {"  Hostile destroyed:  +100 base", TextColor::GREEN},
            {"    x aircraft type multiplier"},
            {"  First-shot kill:    2x bonus", TextColor::GREEN},
            {"  Level bonus:        varies", TextColor::GREEN},
            {""},
            {"NEGATIVE ACTIONS", TextColor::YELLOW, true},
            {"  Hostile penetrates: -200", TextColor::RED},
            {"  Missile wasted:      -25", TextColor::YELLOW},
            {"  Friendly destroyed: GAME OVER", TextColor::RED, true},
            {""},
            {"GAME END CONDITIONS", TextColor::YELLOW, true},
            {"  * Friendly aircraft destroyed", TextColor::RED},
            {"  * Score drops below -2000", TextColor::RED},
            {"  * 3+ batteries destroyed", TextColor::RED},
            {"    (degraded play -> early end)"},
            {""},
            {"LEVEL ADVANCEMENT", TextColor::YELLOW, true},
            {"Destroy the required number of"},
            {"hostiles to advance. Complete all"},
            {"5 levels for MISSION COMPLETE.", TextColor::GREEN},
        };
        leftDrawer_->addPage(page);
    }

    // Page 16: Credits
    {
        BookPage page;
        page.title = "ACKNOWLEDGMENTS";
        page.lines = {
            {"SYSTEMS & EQUIPMENT", TextColor::YELLOW, true},
            {""},
            {"AN/TSQ-73 Missile Minder", TextColor::GREEN, true},
            {"  Litton Industries"},
            {"  Data Systems Division"},
            {"  Van Nuys, California"},
            {""},
            {"AN/TPS-43E Surveillance Radar", TextColor::GREEN, true},
            {"  Raytheon Company"},
            {"  Equipment Division"},
            {"  Wayland, Massachusetts"},
            {""},
            {"MIM-104 Patriot Missile System", TextColor::GREEN, true},
            {"  Raytheon Company"},
            {"  Missile Systems Division"},
            {""},
            {"MIM-23 Improved HAWK System", TextColor::GREEN, true},
            {"  Raytheon Company"},
            {""},
            {"FGM-148 Javelin System", TextColor::GREEN, true},
            {"  Javelin Joint Venture"},
            {"  Raytheon / Lockheed Martin"},
            {""},
            {"United States Army", TextColor::YELLOW, true},
            {"Air Defense Artillery Branch"},
        };
        leftDrawer_->addPage(page);
    }
}

void IntegratedConsoleScene::populateSettingsBook()
{
    // Page 1: Settings TM cover page
    {
        BookPage page;
        page.title = "TM 9-1430-600-10-2";
        page.lines = {
            {"TECHNICAL MANUAL", TextColor::YELLOW, true},
            {""},
            {"OPERATOR INSTRUCTIONS", TextColor::YELLOW, true},
            {""},
            {"CONSOLE OPERATION &", TextColor::GREEN, true},
            {"TACTICAL PROCEDURES", TextColor::GREEN, true},
            {"AN/TSQ-73 MISSILE MINDER", TextColor::GREEN, true},
            {""},
            {"HEADQUARTERS,", TextColor::WHITE},
            {"DEPARTMENT OF THE ARMY", TextColor::WHITE},
            {""},
            {"This manual provides operating"},
            {"instructions, display settings,"},
            {"scenario configurations, and threat"},
            {"identification procedures for the"},
            {"AN/TSQ-73 fire distribution console."},
            {""},
            {"TABLE OF CONTENTS", TextColor::YELLOW, true},
            {"  Sec I.   Console Operation"},
            {"  Sec II.  Display Settings"},
            {"  Sec III. Scenario Levels"},
            {"  Sec IV.  Threat Identification"},
            {"  Sec V.   Defense Zone Layout"},
            {"  Sec VI.  Communications"},
        };
        rightDrawer_->addPage(page);
    }

    // Page 2: Console operation
    {
        BookPage page;
        page.title = "CONSOLE OPERATION";
        page.lines = {
            {"SECTION I. OPERATOR CONTROLS", TextColor::YELLOW, true},
            {""},
            {"1-1. TRACK SELECTION", TextColor::GREEN, true},
            {"Click on radar blip to select a"},
            {"contact. Selected track displays"},
            {"bracket indicator and data block."},
            {""},
            {"1-2. BATTERY ASSIGNMENT", TextColor::GREEN, true},
            {"  KEY  BATTERY        RANGE", TextColor::CYAN, true},
            {"  1    PATRIOT-1      160km", TextColor::GREEN},
            {"  2    PATRIOT-2      160km", TextColor::GREEN},
            {"  3    PATRIOT-3      160km", TextColor::GREEN},
            {"  4    HAWK-1          45km", TextColor::GREEN},
            {"  5    HAWK-2          45km", TextColor::GREEN},
            {"  6    HAWK-3          45km", TextColor::GREEN},
            {"  7    JAVELIN-1       55km", TextColor::GREEN},
            {"  8    JAVELIN-2       55km", TextColor::GREEN},
            {"  9    JAVELIN-3       55km", TextColor::GREEN},
            {""},
            {"1-3. ADDITIONAL COMMANDS", TextColor::GREEN, true},
            {"  F  Authorize fire", TextColor::GREEN},
            {"  A  Abort engagement", TextColor::GREEN},
            {"  P  Toggle CRT phosphor color", TextColor::GREEN},
            {"  R  Restart game", TextColor::GREEN},
        };
        rightDrawer_->addPage(page);
    }

    // Page 3: Display settings
    {
        BookPage page;
        page.title = "DISPLAY SETTINGS";
        page.lines = {
            {"SECTION II. CRT CONFIGURATION", TextColor::YELLOW, true},
            {""},
            {"2-1. PHOSPHOR SELECTION", TextColor::GREEN, true},
            {"Press P to toggle between:", TextColor::WHITE},
            {""},
            {"  P1 GREEN PHOSPHOR", TextColor::GREEN, true},
            {"  Standard radar CRT. High contrast"},
            {"  with good dark-room visibility."},
            {"  Preferred for long-duration ops."},
            {""},
            {"  P39 AMBER PHOSPHOR", TextColor::YELLOW, true},
            {"  Warm amber/orange presentation."},
            {"  Reduced eye strain under extended"},
            {"  operations. Historically used in"},
            {"  some AN/TSQ-73 variants."},
            {""},
            {"2-2. GAIN/VIDEO CONTROLS", TextColor::GREEN, true},
            {"Right panel rotary knobs adjust:"},
            {"  * GAIN - Receiver sensitivity"},
            {"  * VIDEO - Signal amplification"},
            {"  * MTI - Moving target indication"},
            {"  * RANGE - Display scale factor"},
            {""},
            {"2-3. GRID COORDINATES", TextColor::GREEN, true},
            {"8-digit thumbwheels set UTM grid."},
        };
        rightDrawer_->addPage(page);
    }

    // Page 4: PPI scope reference
    {
        BookPage page;
        page.title = "PPI SCOPE REFERENCE";
        page.lines = {
            {"2-4. SCOPE PRESENTATION", TextColor::GREEN, true},
            {""},
            {"The PPI (Plan Position Indicator)"},
            {"displays a top-down view of the"},
            {"defended airspace."},
            {""},
            {"SWEEP BEAM", TextColor::YELLOW, true},
            {"  Clockwise rotation at 6 RPM."},
            {"  Contacts illuminate as the sweep"},
            {"  passes their azimuth position,"},
            {"  then fade with phosphor decay."},
            {""},
            {"RANGE RINGS", TextColor::YELLOW, true},
            {"  5 concentric rings at equal"},
            {"  intervals. Full scale = 463 km"},
            {"  (250 NM radar max range)."},
            {""},
            {"TERRITORY ZONE", TextColor::YELLOW, true},
            {"  Dashed circle at 25 km radius.", TextColor::CYAN},
            {"  Hostile penetration of this zone"},
            {"  results in -200 point penalty."},
            {""},
            {"BATTERY MARKERS", TextColor::YELLOW, true},
            {"  Fixed position icons showing the"},
            {"  location of each fire unit."},
        };
        rightDrawer_->addPage(page);
    }

    // Page 5: Scenario levels
    {
        BookPage page;
        page.title = "SCENARIO LEVELS 1-3";
        page.lines = {
            {"SECTION III. TACTICAL SCENARIOS", TextColor::YELLOW, true},
            {""},
            {"LEVEL 1: TRAINING EXERCISE", TextColor::GREEN, true},
            {"  Max contacts:    3 simultaneous"},
            {"  Spawn rate:      8-15 sec"},
            {"  Speed:           0.7x normal"},
            {"  Friendly ratio:  30%"},
            {"  Stealth:         None"},
            {"  IFF errors:      0%"},
            {"  Duration:        2 minutes"},
            {"  Objective:       Destroy 5 hostiles"},
            {""},
            {"LEVEL 2: ELEVATED ALERT", TextColor::GREEN, true},
            {"  Max contacts:    5 simultaneous"},
            {"  Spawn rate:      6-12 sec"},
            {"  Speed:           0.85x normal"},
            {"  Friendly ratio:  25%"},
            {"  Stealth:         None"},
            {"  IFF errors:      0%"},
            {"  Duration:        2.5 minutes"},
            {"  Objective:       Destroy 10 hostiles"},
            {""},
            {"LEVEL 3: ACTIVE ENGAGEMENT", TextColor::YELLOW, true},
            {"  Max contacts:    8 simultaneous"},
        };
        rightDrawer_->addPage(page);
    }

    // Page 6: Scenario levels continued
    {
        BookPage page;
        page.title = "SCENARIO LEVELS 3-5";
        page.lines = {
            {"LEVEL 3 (continued)", TextColor::YELLOW, true},
            {"  Spawn rate:      4-9 sec"},
            {"  Speed:           1.0x normal"},
            {"  Friendly ratio:  20%"},
            {"  Stealth:         None"},
            {"  IFF errors:      5%"},
            {"  Duration:        3 minutes"},
            {"  Objective:       Destroy 18 hostiles"},
            {""},
            {"LEVEL 4: FORCE ESCALATION", TextColor::YELLOW, true},
            {"  Max contacts:    10 simultaneous"},
            {"  Spawn rate:      3-7 sec"},
            {"  Speed:           1.15x normal"},
            {"  Friendly ratio:  15%"},
            {"  Stealth:         ENABLED", TextColor::RED},
            {"  IFF errors:      10%"},
            {"  Duration:        3.5 minutes"},
            {"  Objective:       Destroy 25 hostiles"},
            {""},
            {"LEVEL 5: FINAL STAND", TextColor::RED, true},
            {"  Max contacts:    15 simultaneous"},
            {"  Spawn rate:      2-5 sec"},
            {"  Speed:           1.3x normal"},
            {"  Friendly ratio:  10%"},
            {"  Stealth:         ENABLED", TextColor::RED},
            {"  IFF errors:      20%", TextColor::RED},
            {"  Duration:        5 minutes"},
            {"  Objective:       Destroy 40 hostiles"},
        };
        rightDrawer_->addPage(page);
    }

    // Page 7: Threat identification
    {
        BookPage page;
        page.title = "THREAT IDENTIFICATION";
        page.lines = {
            {"SECTION IV. THREAT AIRCRAFT", TextColor::YELLOW, true},
            {""},
            {"STRATEGIC BOMBER", TextColor::RED, true},
            {"  Profile: High altitude, fast"},
            {"  RCS:     Large (easy to detect)"},
            {"  Danger:  HIGH - carries ordnance"},
            {"  Counter: PATRIOT at range"},
            {""},
            {"FIGHTER / ATTACK", TextColor::RED, true},
            {"  Profile: Fast, agile, med alt"},
            {"  RCS:     Medium"},
            {"  Danger:  HIGH - closes quickly"},
            {"  Counter: HAWK or PATRIOT"},
            {""},
            {"TACTICAL BOMBER", TextColor::RED, true},
            {"  Profile: Low-med alt, moderate"},
            {"  RCS:     Medium-large"},
            {"  Danger:  MEDIUM"},
            {"  Counter: HAWK preferred"},
            {""},
            {"RECON DRONE", TextColor::YELLOW, true},
            {"  Profile: Small, slow"},
            {"  RCS:     Small (hard to classify)"},
            {"  Danger:  LOW (no ordnance)"},
            {"  Counter: JAVELIN or ignore"},
        };
        rightDrawer_->addPage(page);
    }

    // Page 8: Threat identification continued
    {
        BookPage page;
        page.title = "THREATS (CONTINUED)";
        page.lines = {
            {"ATTACK DRONE", TextColor::RED, true},
            {"  Profile: Small, slow-medium"},
            {"  RCS:     Small"},
            {"  Danger:  MEDIUM (may have ordnance)"},
            {"  Counter: JAVELIN or HAWK"},
            {""},
            {"STEALTH FIGHTER", TextColor::RED, true},
            {"  Profile: Med alt, fast"},
            {"  RCS:     VERY LOW", TextColor::RED},
            {"  Danger:  EXTREME"},
            {"  IFF:     30% chance UNKNOWN result"},
            {"  Counter: JAVELIN (IR defeats RCS)", TextColor::GREEN},
            {""},
            {"DO NOT ENGAGE", TextColor::RED, true},
            {""},
            {"CIVILIAN AIRLINER", TextColor::RED, true},
            {"  IFF:     Should show FRIENDLY"},
            {"  Penalty: -1000 pts"},
            {"  Result:  GAME OVER", TextColor::RED},
            {""},
            {"FRIENDLY MILITARY", TextColor::RED, true},
            {"  IFF:     Should show FRIENDLY"},
            {"  Result:  IMMEDIATE GAME OVER", TextColor::RED},
            {""},
            {"VERIFY IFF BEFORE EVERY SHOT.", TextColor::RED, true},
        };
        rightDrawer_->addPage(page);
    }

    // Page 9: Defense zone layout
    {
        BookPage page;
        page.title = "DEFENSE ZONE LAYOUT";
        page.lines = {
            {"SECTION V. BATTALION DEPLOYMENT", TextColor::YELLOW, true},
            {""},
            {"Total footprint: ~75 km across", TextColor::CYAN},
            {"Territory radius: 25 km", TextColor::CYAN},
            {""},
            {"        N (0 deg)", TextColor::GREEN},
            {"          |"},
            {"    PAT-1 (35km)"},
            {"     /          \\"},
            {"  JAV-1(8)   HAWK-1(20)"},
            {"    |              |"},
            {"    |  [AN/TSQ-73] |"},
            {"    |   (center)   |"},
            {"    |  [==25km==]  |"},
            {"    |              |"},
            {"  JAV-3(8)   HAWK-2(20)"},
            {"     \\          /"},
            {"  PAT-3(35) HAWK-3(20)"},
            {"          |"},
            {"       JAV-2(8)"},
            {"          |"},
            {"        S (180 deg)", TextColor::GREEN},
            {""},
            {"Batteries OUTSIDE territory to"},
            {"intercept before penetration.", TextColor::YELLOW},
        };
        rightDrawer_->addPage(page);
    }

    // Page 10: Communications
    {
        BookPage page;
        page.title = "COMMUNICATIONS";
        page.lines = {
            {"SECTION VI. COMM SYSTEMS", TextColor::YELLOW, true},
            {""},
            {"6-1. DATA LINKS", TextColor::GREEN, true},
            {"AN/TSQ-73 communicates with fire"},
            {"batteries via dedicated circuits:"},
            {""},
            {"  * Point-to-point microwave", TextColor::CYAN},
            {"    Directional site-to-site links"},
            {"  * SATCOM backup", TextColor::CYAN},
            {"    Extended range / terrain blocked"},
            {"  * Voice circuits", TextColor::CYAN},
            {"    Tactical coordination"},
            {"  * Digital data circuits", TextColor::CYAN},
            {"    Track data, engagement commands,"},
            {"    and battery status reporting"},
            {"    via L-112 processors"},
            {""},
            {"6-2. AUTONOMOUS OPERATIONS", TextColor::GREEN, true},
            {"When HQ displaces, comm links go"},
            {"down. Batteries switch to autonomous"},
            {"engagement using organic radars:", TextColor::YELLOW},
            {"  Patriot: AN/MPQ-53"},
            {"  Hawk:    AN/MPQ-46 + AN/MPQ-50"},
            {"  Javelin: CLU IR/FLIR (no radar)"},
        };
        rightDrawer_->addPage(page);
    }

    // Page 11: S-280 shelter description
    {
        BookPage page;
        page.title = "S-280 SHELTER";
        page.lines = {
            {"SECTION VII. EQUIPMENT", TextColor::YELLOW, true},
            {""},
            {"7-1. S-280 COMMAND SHELTER", TextColor::GREEN, true},
            {""},
            {"The S-280 is a climate-controlled"},
            {"aluminum shelter mounted on an"},
            {"M927 5-ton cargo truck. Interior"},
            {"houses two complete operator"},
            {"consoles with PPI scopes."},
            {""},
            {"SHELTER SPECIFICATIONS", TextColor::YELLOW, true},
            {"  Length:     12 ft (3.66m)"},
            {"  Width:      8 ft (2.44m)"},
            {"  Height:     6 ft (1.83m)"},
            {"  Weight:   ~4,500 lbs (loaded)"},
            {"  Power:    440V 3-phase from"},
            {"            MEP-006A 60kW genset"},
            {"  HVAC:     Integral A/C and heat"},
            {""},
            {"INTERIOR LAYOUT", TextColor::YELLOW, true},
            {"  * Two operator stations (L & R)"},
            {"  * Equipment racks (rear wall)"},
            {"  * Writing bench w/ plexiglass"},
            {"  * Emergency exit hatch (rear)"},
            {"  * Overhead red night-ops lighting"},
        };
        rightDrawer_->addPage(page);
    }

    // Page 12: Credits
    {
        BookPage page;
        page.title = "CREDITS & ACKNOWLEDGMENTS";
        page.lines = {
            {"SIMULATION CREDITS", TextColor::YELLOW, true},
            {""},
            {"AN/TSQ-73 Missile Minder", TextColor::GREEN, true},
            {"Command Simulation"},
            {""},
            {"(c) 2026 Imagery Business Systems, LLC"},
            {"All Rights Reserved."},
            {""},
            {"SYSTEM MANUFACTURERS", TextColor::YELLOW, true},
            {""},
            {"Litton Industries", TextColor::CYAN, true},
            {"  Data Systems Division"},
            {"  AN/TSQ-73 Fire Distribution Center"},
            {""},
            {"Raytheon Company", TextColor::CYAN, true},
            {"  AN/TPS-43E Surveillance Radar"},
            {"  MIM-104 Patriot Missile System"},
            {"  MIM-23 Improved HAWK System"},
            {""},
            {"Raytheon / Lockheed Martin", TextColor::CYAN, true},
            {"  Javelin Joint Venture"},
            {"  FGM-148 Javelin Weapon System"},
            {""},
            {"United States Army", TextColor::YELLOW, true},
            {"  Air Defense Artillery Branch"},
            {"  \"First To Fire\""},
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
