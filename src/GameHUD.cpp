#include "GameHUD.h"
#include <cstdio>

#if USE_COCOS2DX
// ============================================================================
// Cocos2d-x Game HUD
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

    // Track Info Panel (right side)
    trackInfoLabel_ = cocos2d::Label::createWithSystemFont(
        "NO TRACK SELECTED", "Courier", 12);
    trackInfoLabel_->setAnchorPoint(cocos2d::Vec2(0, 1));
    trackInfoLabel_->setPosition(cocos2d::Vec2(10, -10));
    trackInfoLabel_->setTextColor(cocos2d::Color4B(0, 255, 0, 255));
    addChild(trackInfoLabel_);

    // Battery Status Panel
    batteryStatusLabel_ = cocos2d::Label::createWithSystemFont(
        "BATTERY STATUS", "Courier", 11);
    batteryStatusLabel_->setAnchorPoint(cocos2d::Vec2(0, 1));
    batteryStatusLabel_->setPosition(cocos2d::Vec2(10, -150));
    batteryStatusLabel_->setTextColor(cocos2d::Color4B(0, 200, 0, 255));
    addChild(batteryStatusLabel_);

    // Score and Level
    scoreLabel_ = cocos2d::Label::createWithSystemFont("SCORE: 0", "Courier", 14);
    scoreLabel_->setAnchorPoint(cocos2d::Vec2(0, 1));
    scoreLabel_->setPosition(cocos2d::Vec2(10, -400));
    scoreLabel_->setTextColor(cocos2d::Color4B(0, 255, 0, 255));
    addChild(scoreLabel_);

    levelLabel_ = cocos2d::Label::createWithSystemFont("LEVEL: 1", "Courier", 14);
    levelLabel_->setAnchorPoint(cocos2d::Vec2(0, 1));
    levelLabel_->setPosition(cocos2d::Vec2(10, -420));
    levelLabel_->setTextColor(cocos2d::Color4B(0, 255, 0, 255));
    addChild(levelLabel_);

    // Message Log
    messageLog_ = cocos2d::Label::createWithSystemFont("", "Courier", 10);
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
        trackInfoLabel_->setString("NO TRACK SELECTED");
        return;
    }

    auto* track = trackManager_->getTrack(selectedTrackId_);
    if (!track) {
        trackInfoLabel_->setString("TRACK LOST");
        return;
    }

    char info[512];
    snprintf(info, sizeof(info),
        "=== TRACK DATA ===\n"
        "ID:    %s\n"
        "CLASS: %s\n"
        "ALT:   %s\n"
        "AZM:   %03d deg\n"
        "RNG:   %.1f km\n"
        "SPD:   %.0f kts\n"
        "HDG:   %03d deg",
        track->getTrackIdString().c_str(),
        track->getClassificationString().c_str(),
        track->getAltitudeString().c_str(),
        (int)track->azimuth,
        track->range,
        track->speed,
        (int)track->heading);

    trackInfoLabel_->setString(info);
}

void GameHUD::updateBatteryStatusPanel()
{
    if (!fireControl_) return;

    std::string status = "=== BATTERY STATUS ===\n";
    auto batteries = fireControl_->getAllBatteryData();

    for (const auto& bat : batteries) {
        char line[128];
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

        snprintf(line, sizeof(line), "%-10s [%s] %d/%d",
            bat.designation.c_str(), statusStr,
            bat.missilesRemaining, bat.maxMissiles);
        status += line;
        status += "\n";
    }

    batteryStatusLabel_->setString(status);
}

void GameHUD::updateScoreDisplay()
{
    char buf[32];
    snprintf(buf, sizeof(buf), "SCORE: %d", score_);
    scoreLabel_->setString(buf);

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
