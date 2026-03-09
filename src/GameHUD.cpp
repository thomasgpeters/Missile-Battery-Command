#include "GameHUD.h"
#include <cstdio>

#if USE_COCOS2DX
// ============================================================================
// Cocos2d-x Game HUD — Phase 2 Enhanced
// ============================================================================

GameHUD* GameHUD::create()
{
    auto* ret = new (std::nothrow) GameHUD();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool GameHUD::init()
{
    if (!Node::init()) return false;

    trackManager_ = nullptr;
    fireControl_ = nullptr;
    selectedTrackId_ = -1;
    score_ = 0;
    level_ = 1;

    // === HEADER ===
    auto* headerLabel = cocos2d::Label::createWithSystemFont(
        "AN/TSQ-73 CONSOLE", "Courier", 13);
    headerLabel->setAnchorPoint(cocos2d::Vec2(0, 1));
    headerLabel->setPosition(cocos2d::Vec2(10, -5));
    headerLabel->setTextColor(cocos2d::Color4B(0, 255, 0, 255));
    addChild(headerLabel);

    // === TRACK INFO PANEL ===
    trackInfoLabel_ = cocos2d::Label::createWithSystemFont(
        "NO TRACK SELECTED", "Courier", 11);
    trackInfoLabel_->setAnchorPoint(cocos2d::Vec2(0, 1));
    trackInfoLabel_->setPosition(cocos2d::Vec2(10, -30));
    trackInfoLabel_->setTextColor(cocos2d::Color4B(0, 255, 0, 255));
    addChild(trackInfoLabel_);

    // === BATTERY STATUS PANEL ===
    batteryStatusLabel_ = cocos2d::Label::createWithSystemFont(
        "BATTERY STATUS", "Courier", 10);
    batteryStatusLabel_->setAnchorPoint(cocos2d::Vec2(0, 1));
    batteryStatusLabel_->setPosition(cocos2d::Vec2(10, -180));
    batteryStatusLabel_->setTextColor(cocos2d::Color4B(0, 200, 0, 255));
    addChild(batteryStatusLabel_);

    // === CONTROLS HELP ===
    auto* controlsLabel = cocos2d::Label::createWithSystemFont(
        "FIRE CONTROL:\n"
        " 1-3 PATRIOT  4-6 HAWK\n"
        " 7-9 JAVELIN\n"
        " F=FIRE  A=ABORT\n"
        " CLICK=SELECT TRACK",
        "Courier", 9);
    controlsLabel->setAnchorPoint(cocos2d::Vec2(0, 1));
    controlsLabel->setPosition(cocos2d::Vec2(10, -370));
    controlsLabel->setTextColor(cocos2d::Color4B(0, 150, 0, 180));
    addChild(controlsLabel);

    // === SCORE AND LEVEL ===
    scoreLabel_ = cocos2d::Label::createWithSystemFont("SCORE: 0", "Courier", 13);
    scoreLabel_->setAnchorPoint(cocos2d::Vec2(0, 1));
    scoreLabel_->setPosition(cocos2d::Vec2(10, -440));
    scoreLabel_->setTextColor(cocos2d::Color4B(0, 255, 0, 255));
    addChild(scoreLabel_);

    levelLabel_ = cocos2d::Label::createWithSystemFont("LEVEL: 1", "Courier", 13);
    levelLabel_->setAnchorPoint(cocos2d::Vec2(0, 1));
    levelLabel_->setPosition(cocos2d::Vec2(10, -460));
    levelLabel_->setTextColor(cocos2d::Color4B(0, 255, 0, 255));
    addChild(levelLabel_);

    // === MESSAGE LOG ===
    messageLog_ = cocos2d::Label::createWithSystemFont("", "Courier", 9);
    messageLog_->setAnchorPoint(cocos2d::Vec2(0, 0));
    messageLog_->setPosition(cocos2d::Vec2(10, 10));
    messageLog_->setTextColor(cocos2d::Color4B(0, 200, 0, 200));
    addChild(messageLog_);

    scheduleUpdate();
    return true;
}

void GameHUD::update(float dt)
{
    updateTrackInfoPanel();
    updateBatteryStatusPanel();
    updateScoreDisplay();
    updateMessageLog();
}

void GameHUD::setSelectedTrack(int trackId)
{
    selectedTrackId_ = trackId;
}

void GameHUD::setScore(int score)
{
    score_ = score;
}

void GameHUD::setLevel(int level)
{
    level_ = level;
}

void GameHUD::addMessage(const std::string& message)
{
    messages_.push_back(message);
    if ((int)messages_.size() > MAX_MESSAGES) {
        messages_.erase(messages_.begin());
    }
}

void GameHUD::updateTrackInfoPanel()
{
    if (!trackManager_ || selectedTrackId_ < 0) {
        trackInfoLabel_->setString(
            "=== TRACK DATA ===\n"
            "\n"
            "  NO TRACK SELECTED\n"
            "\n"
            "  Click a blip on the\n"
            "  radar to select.\n");
        return;
    }

    auto* track = trackManager_->getTrack(selectedTrackId_);
    if (!track) {
        trackInfoLabel_->setString(
            "=== TRACK DATA ===\n"
            "\n"
            "  ** TRACK LOST **\n");
        return;
    }

    // Determine range band for engagement awareness
    const char* rangeBand;
    if (track->range <= 10.0f) rangeBand = "CRITICAL";
    else if (track->range <= 40.0f) rangeBand = "SHORT";
    else if (track->range <= 70.0f) rangeBand = "MEDIUM";
    else rangeBand = "LONG";

    char info[600];
    snprintf(info, sizeof(info),
        "=== TRACK DATA ===\n"
        "ID:    %s\n"
        "CLASS: %s\n"
        "ALT:   %s\n"
        "AZM:   %03d deg\n"
        "RNG:   %.0f NM (%.0f km) [%s]\n"
        "SPD:   %.0f kts\n"
        "HDG:   %03d deg\n"
        "\n"
        "AVAILABLE BATTERIES:",
        track->getTrackIdString().c_str(),
        track->getClassificationString().c_str(),
        track->getAltitudeString().c_str(),
        (int)track->azimuth,
        track->range * GameConstants::KM_TO_NM, track->range, rangeBand,
        track->speed,
        (int)track->heading);

    std::string infoStr(info);

    // Show which batteries can engage this track
    if (fireControl_) {
        auto available = fireControl_->getAvailableBatteries(selectedTrackId_,
                                                              *trackManager_);
        if (available.empty()) {
            infoStr += "\n  (none in range)";
        } else {
            for (const auto& batName : available) {
                infoStr += "\n  > " + batName;
            }
        }
    }

    trackInfoLabel_->setString(infoStr);
}

void GameHUD::updateBatteryStatusPanel()
{
    if (!fireControl_) return;

    std::string status = "=== BATTERY STATUS ===\n";
    auto batteries = fireControl_->getAllBatteryData();

    // Determine which batteries can engage the selected track
    std::vector<std::string> availableNames;
    if (trackManager_ && selectedTrackId_ >= 0) {
        availableNames = fireControl_->getAvailableBatteries(
            selectedTrackId_, *trackManager_);
    }

    for (const auto& bat : batteries) {
        const char* statusStr;
        switch (bat.status) {
            case BatteryStatus::READY:     statusStr = "RDY"; break;
            case BatteryStatus::TRACKING:  statusStr = "TRK"; break;
            case BatteryStatus::ENGAGED:   statusStr = "ENG"; break;
            case BatteryStatus::RELOADING: statusStr = "RLD"; break;
            case BatteryStatus::DESTROYED: statusStr = "DES"; break;
            case BatteryStatus::OFFLINE:   statusStr = "OFF"; break;
            default: statusStr = "---"; break;
        }

        // Mark available batteries with an arrow
        bool isAvailable = false;
        for (const auto& name : availableNames) {
            if (name == bat.designation) { isAvailable = true; break; }
        }

        char line[128];
        if (bat.totalMissileStock > bat.missilesRemaining) {
            // Show stock count for batteries with reserves (e.g., Hawk)
            snprintf(line, sizeof(line), "%s%-10s [%s] %d/%d stk:%d",
                isAvailable ? ">" : " ",
                bat.designation.c_str(), statusStr,
                bat.missilesRemaining, bat.maxMissiles,
                bat.totalMissileStock);
        } else {
            snprintf(line, sizeof(line), "%s%-10s [%s] %d/%d",
                isAvailable ? ">" : " ",
                bat.designation.c_str(), statusStr,
                bat.missilesRemaining, bat.maxMissiles);
        }
        status += line;

        // Show reload time if reloading
        if (bat.status == BatteryStatus::RELOADING && bat.reloadTimeRemaining > 0) {
            char reload[16];
            snprintf(reload, sizeof(reload), " (%.0fs)", bat.reloadTimeRemaining);
            status += reload;
        }

        // Show assigned track if engaged
        if (bat.status == BatteryStatus::ENGAGED && bat.assignedTrackId >= 0) {
            char assigned[16];
            snprintf(assigned, sizeof(assigned), " ->TK-%03d", bat.assignedTrackId);
            status += assigned;
        }

        status += "\n";
    }

    batteryStatusLabel_->setString(status);
}

void GameHUD::updateScoreDisplay()
{
    char buf[32];
    snprintf(buf, sizeof(buf), "SCORE: %d", score_);
    scoreLabel_->setString(buf);

    // Color score red if negative
    if (score_ < 0) {
        scoreLabel_->setTextColor(cocos2d::Color4B(255, 80, 80, 255));
    } else {
        scoreLabel_->setTextColor(cocos2d::Color4B(0, 255, 0, 255));
    }

    snprintf(buf, sizeof(buf), "LEVEL: %d", level_);
    levelLabel_->setString(buf);
}

void GameHUD::updateMessageLog()
{
    std::string log;
    for (const auto& msg : messages_) {
        log += "> " + msg + "\n";
    }
    messageLog_->setString(log);
}

#else
// ============================================================================
// Stub HUD
// ============================================================================

GameHUD* GameHUD::create()
{
    auto* hud = new GameHUD();
    if (hud->init()) return hud;
    delete hud;
    return nullptr;
}

bool GameHUD::init() { return true; }
void GameHUD::update(float dt) {}

void GameHUD::addMessage(const std::string& message)
{
    messages_.push_back(message);
    if (messages_.size() > 8) {
        messages_.erase(messages_.begin());
    }
}
#endif
