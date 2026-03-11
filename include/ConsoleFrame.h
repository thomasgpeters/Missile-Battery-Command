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
// ConsoleFrame — AN/TSQ-73 Missile Battery Command Console
//
// Authentic portrait-format console based on the real AN/TSQ-73:
//   - Portrait bezel (~25% taller than wide) with rounded corners
//   - Round PPI radar scope centered in the bezel
//   - Left side: Analog illuminated button arrays + numeric keypad
//   - Right side: Console settings button arrays
//   - Bottom-left: Fire command action buttons
//   - Bottom-right: Small joystick
//   - Top: Status indicator row
//   - Below console: Score/level/message bar
// ============================================================================

class ConsoleFrame : public cocos2d::Node {
public:
    static ConsoleFrame* create(float scopeRadius);
    virtual bool init(float scopeRadius);

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

    // Bezel dimensions (for scene layout)
    float getBezelWidth() const { return bezelW_; }
    float getBezelHeight() const { return bezelH_; }

private:
    float scopeRadius_;
    float bezelW_;           // total bezel width
    float bezelH_;           // total bezel height (25% taller than wide)
    float cornerRadius_;     // rounded corner radius

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
    cocos2d::DrawNode* buttonNode_;      // Static: analog button arrays
    cocos2d::DrawNode* dynamicNode_;     // Dynamic: lit button states, indicators
    cocos2d::Node*     labelNode_;       // Dynamic: text labels

    // Static drawing (called once in init)
    void drawShelterBackground();
    void drawBezel();
    void drawScopeRing();
    void drawLeftButtonPanel();
    void drawRightButtonPanel();
    void drawBottomLeftFireButtons();
    void drawBottomRightJoystick();
    void drawTopIndicatorRow();
    void drawManufacturerPlate();

    // Helper: draw a rounded rectangle
    void drawRoundedRect(cocos2d::DrawNode* node,
                         const cocos2d::Vec2& origin,
                         const cocos2d::Vec2& dest,
                         float radius,
                         const cocos2d::Color4F& fill,
                         const cocos2d::Color4F& border);

    // Helper: draw a row of illuminated pushbuttons
    void drawButtonRow(cocos2d::DrawNode* node,
                       float x, float y, int count,
                       float btnW, float btnH, float gap,
                       const cocos2d::Color4F& btnColor,
                       const cocos2d::Color4F& litColor,
                       bool vertical = false);

    // Helper: draw a single button with label
    void drawLabeledButton(cocos2d::DrawNode* node,
                           float cx, float cy,
                           float w, float h,
                           const cocos2d::Color4F& color,
                           const char* label,
                           float fontSize = 6.0f);

    // Dynamic drawing (called every frame)
    void drawDynamicIndicators();
    void drawBottomBar();
};

#else
// ============================================================================
// Stub ConsoleFrame (no cocos2d-x)
// ============================================================================
class ConsoleFrame {
public:
    static ConsoleFrame* create(float scopeRadius);
    bool init(float scopeRadius);
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
    float getBezelWidth() const { return bezelW_; }
    float getBezelHeight() const { return bezelH_; }

private:
    float scopeRadius_ = 280.0f;
    float bezelW_ = 0.0f;
    float bezelH_ = 0.0f;
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
