#ifndef __CONSOLE_FRAME_H__
#define __CONSOLE_FRAME_H__

#include "GameTypes.h"
#include "TrackManager.h"
#include "FireControlSystem.h"
#include "ThreatBoard.h"
#include "BattalionHQ.h"
#include <vector>
#include <string>

#if USE_COCOS2DX
#include "cocos2d.h"

// ============================================================================
// ConsoleFrame — AN/TSQ-73 Display Console housing with live data panels
//
// Draws the physical console structure around the PPI scope:
//   - Metal housing with rubber scope bezel
//   - Left panel:  Live track table (LED-style readout)
//   - Right panel: Battery status, HQ status, threat count
//   - Bottom strip: Rotary knobs, toggle switches
//   - Top strip: Indicator LEDs (PWR, RDR, IFF, WPN, COM)
//   - Score/level/message bar below the console
//
// This is both decorative (the console housing) and functional (live data
// panels replace the static button arrays from the reference photo).
// ============================================================================

class ConsoleFrame : public cocos2d::Node {
public:
    static ConsoleFrame* create(float scopeRadius, float panelWidth = 160.0f);
    virtual bool init(float scopeRadius, float panelWidth);

    void update(float dt) override;

    // Data sources
    void setTrackManager(TrackManager* mgr) { trackManager_ = mgr; }
    void setFireControlSystem(FireControlSystem* fcs) { fireControl_ = fcs; }
    void setThreatBoard(ThreatBoard* tb) { threatBoard_ = tb; }
    void setBattalionHQ(BattalionHQ* hq) { battalionHQ_ = hq; }

    // Game state
    void setScore(int score) { score_ = score; }
    void setLevel(int level) { level_ = level; }
    void setSelectedTrack(int trackId) { selectedTrackId_ = trackId; }
    void addMessage(const std::string& msg);

    float getScopeRadius() const { return scopeRadius_; }
    float getPanelWidth() const { return panelWidth_; }

private:
    float scopeRadius_;
    float panelWidth_;

    // Data sources
    TrackManager* trackManager_;
    FireControlSystem* fireControl_;
    ThreatBoard* threatBoard_;
    BattalionHQ* battalionHQ_;

    int score_;
    int level_;
    int selectedTrackId_;
    std::vector<std::string> messages_;
    static const int MAX_MESSAGES = 6;

    // Draw nodes (layered)
    cocos2d::DrawNode* housingNode_;     // Static: console housing, bezel
    cocos2d::DrawNode* dataPanelNode_;   // Dynamic: live data text in panels
    cocos2d::Node*     dataLabelNode_;   // Dynamic: text labels for data

    // Static drawing (called once)
    void drawConsoleHousing();
    void drawScopeBezel();
    void drawBottomControlStrip();
    void drawTopIndicators();
    void drawShelterBackground();
    void drawCornerWedges(float r);

    // Dynamic drawing (called every frame)
    void drawLeftPanel();    // Track table
    void drawRightPanel();   // Battery + HQ + threat
    void drawBottomBar();    // Score, level, messages
};

#else
// ============================================================================
// Stub ConsoleFrame (no cocos2d-x)
// ============================================================================
class ConsoleFrame {
public:
    static ConsoleFrame* create(float scopeRadius, float panelWidth = 160.0f);
    bool init(float scopeRadius, float panelWidth);
    void update(float dt);

    void setTrackManager(TrackManager* mgr) { trackManager_ = mgr; }
    void setFireControlSystem(FireControlSystem* fcs) { fireControl_ = fcs; }
    void setThreatBoard(ThreatBoard* tb) { threatBoard_ = tb; }
    void setBattalionHQ(BattalionHQ* hq) { battalionHQ_ = hq; }

    void setScore(int score) { score_ = score; }
    void setLevel(int level) { level_ = level; }
    void setSelectedTrack(int trackId) { selectedTrackId_ = trackId; }
    void addMessage(const std::string& msg);

    float getScopeRadius() const { return scopeRadius_; }
    float getPanelWidth() const { return panelWidth_; }

private:
    float scopeRadius_ = 280.0f;
    float panelWidth_ = 160.0f;
    TrackManager* trackManager_ = nullptr;
    FireControlSystem* fireControl_ = nullptr;
    ThreatBoard* threatBoard_ = nullptr;
    BattalionHQ* battalionHQ_ = nullptr;
    int score_ = 0;
    int level_ = 1;
    int selectedTrackId_ = -1;
    std::vector<std::string> messages_;
};
#endif

#endif // __CONSOLE_FRAME_H__
