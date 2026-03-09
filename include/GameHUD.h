#ifndef __GAME_HUD_H__
#define __GAME_HUD_H__

#include "GameTypes.h"
#include "TrackManager.h"
#include "FireControlSystem.h"

#if USE_COCOS2DX
#include "cocos2d.h"

// ============================================================================
// Game HUD - displays track info panel, battery status, and score
// ============================================================================

class GameHUD : public cocos2d::Node {
public:
    static GameHUD* create();
    virtual bool init() override;

    void update(float dt) override;

    // Data sources
    void setTrackManager(TrackManager* mgr) { trackManager_ = mgr; }
    void setFireControlSystem(FireControlSystem* fcs) { fireControl_ = fcs; }

    // Update displayed information
    void setSelectedTrack(int trackId);
    void setScore(int score);
    void setLevel(int level);
    void addMessage(const std::string& message);

private:
    TrackManager* trackManager_;
    FireControlSystem* fireControl_;
    int selectedTrackId_;
    int score_;
    int level_;

    // UI Elements
    cocos2d::Label* trackInfoLabel_;
    cocos2d::Label* batteryStatusLabel_;
    cocos2d::Label* scoreLabel_;
    cocos2d::Label* levelLabel_;
    cocos2d::Label* messageLog_;

    std::vector<std::string> messages_;
    static const int MAX_MESSAGES = 8;

    void updateTrackInfoPanel();
    void updateBatteryStatusPanel();
    void updateScoreDisplay();
    void updateMessageLog();
};

#else
// Stub
class GameHUD {
public:
    static GameHUD* create();
    bool init();
    void update(float dt);

    void setTrackManager(TrackManager* mgr) { trackManager_ = mgr; }
    void setFireControlSystem(FireControlSystem* fcs) { fireControl_ = fcs; }
    void setSelectedTrack(int trackId) { selectedTrackId_ = trackId; }
    void setScore(int score) { score_ = score; }
    void setLevel(int level) { level_ = level; }
    void addMessage(const std::string& message);

private:
    TrackManager* trackManager_ = nullptr;
    FireControlSystem* fireControl_ = nullptr;
    int selectedTrackId_ = -1;
    int score_ = 0;
    int level_ = 1;
    std::vector<std::string> messages_;
};
#endif

#endif // __GAME_HUD_H__
