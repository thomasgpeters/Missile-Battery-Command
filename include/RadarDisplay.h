#ifndef __RADAR_DISPLAY_H__
#define __RADAR_DISPLAY_H__

#include "GameTypes.h"
#include "TrackManager.h"
#include "FireControlSystem.h"

#if USE_COCOS2DX
#include "cocos2d.h"

// ============================================================================
// Radar PPI Display - renders the AN/TSQ-73 radar console view
// ============================================================================

class RadarDisplay : public cocos2d::Node {
public:
    static RadarDisplay* create(float radius);
    virtual bool init(float radius);

    // Update radar state
    void update(float dt) override;

    // Set data sources
    void setTrackManager(TrackManager* mgr) { trackManager_ = mgr; }
    void setFireControlSystem(FireControlSystem* fcs) { fireControl_ = fcs; }

    // Interaction
    void setSelectedTrack(int trackId) { selectedTrackId_ = trackId; }
    int getSelectedTrack() const { return selectedTrackId_; }

    // Get current sweep angle
    float getSweepAngle() const { return sweepAngle_; }

private:
    float radius_;           // Display radius in pixels
    float sweepAngle_;       // Current sweep beam angle (degrees)
    int selectedTrackId_;    // Currently selected track (-1 = none)

    TrackManager* trackManager_;
    FireControlSystem* fireControl_;

    // Drawing
    cocos2d::DrawNode* backgroundNode_;
    cocos2d::DrawNode* sweepNode_;
    cocos2d::DrawNode* blipNode_;
    cocos2d::DrawNode* overlayNode_;

    void drawBackground();
    void drawRangeRings();
    void drawAzimuthLines();
    void drawSweepBeam();
    void drawBlips();
    void drawBatteryPositions();
    void drawTerritoryZone();
    void drawSelectedTrackInfo();

    // Convert polar to screen coordinates
    cocos2d::Vec2 polarToScreen(float range, float azimuth) const;
    float kmToPixels(float km) const;
};

#else
// Stub for development without cocos2d-x
class RadarDisplay {
public:
    static RadarDisplay* create(float radius);
    bool init(float radius);
    void update(float dt);

    void setTrackManager(TrackManager* mgr) { trackManager_ = mgr; }
    void setFireControlSystem(FireControlSystem* fcs) { fireControl_ = fcs; }
    void setSelectedTrack(int trackId) { selectedTrackId_ = trackId; }
    int getSelectedTrack() const { return selectedTrackId_; }
    float getSweepAngle() const { return sweepAngle_; }

private:
    float radius_ = 0;
    float sweepAngle_ = 0;
    int selectedTrackId_ = -1;
    TrackManager* trackManager_ = nullptr;
    FireControlSystem* fireControl_ = nullptr;
};
#endif

#endif // __RADAR_DISPLAY_H__
