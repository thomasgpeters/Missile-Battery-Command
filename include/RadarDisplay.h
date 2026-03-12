#ifndef __RADAR_DISPLAY_H__
#define __RADAR_DISPLAY_H__

#include "GameTypes.h"
#include "TrackManager.h"
#include "FireControlSystem.h"
#include "RadarBlip.h"
#include <vector>
#include <unordered_map>

#if USE_COCOS2DX
#include "cocos2d.h"

// ============================================================================
// Radar PPI Display — AN/TSQ-73 radar console view
//
// Renders the Plan Position Indicator with authentic CRT phosphor behavior:
//   - Dark CRT background with range rings and azimuth lines
//   - Rotating sweep beam with trailing phosphor glow
//   - RadarBlip entities: invisible until swept, phosphor tick + decay
//   - IFF track data overlay painted on blips after interrogation
//   - Battery position indicators and missile flight trails
//   - Territory defense zone ring
//   - Radar noise/ground clutter
// ============================================================================

class RadarDisplay : public cocos2d::Node {
public:
    static RadarDisplay* create(float radius);
    virtual bool init(float radius);

    void update(float dt) override;

    // Data sources
    void setTrackManager(TrackManager* mgr) { trackManager_ = mgr; }
    void setFireControlSystem(FireControlSystem* fcs) { fireControl_ = fcs; }

    // Interaction
    void setSelectedTrack(int trackId);
    int getSelectedTrack() const { return selectedTrackId_; }

    // Get display properties
    float getSweepAngle() const { return sweepAngle_; }
    float getRadius() const { return radius_; }

    // Find nearest track to a screen-space point (relative to this node)
    int findNearestTrack(const cocos2d::Vec2& localPos, float maxPixelDist = 20.0f) const;

    // Convert polar coordinates to screen-space position relative to this node
    cocos2d::Vec2 polarToScreen(float range, float azimuth) const;
    float kmToPixels(float km) const;

    // Toggle options
    void setShowNoise(bool show) { showNoise_ = show; }

private:
    float radius_;          // Coordinate-mapping radius (aircraft at max range map here)
    float sweepRadius_;     // Visual sweep radius (extends behind the bezel)
    float sweepAngle_;
    float prevSweepAngle_;
    int selectedTrackId_;

    TrackManager* trackManager_;
    FireControlSystem* fireControl_;

    // Display options
    bool showNoise_;

    // RadarBlip entities — one per active track
    std::unordered_map<int, RadarBlip*> blips_;
    cocos2d::Node* blipContainer_;   // Parent node for all RadarBlip children

    // Drawing layers (ordered by z-depth)
    cocos2d::DrawNode* backgroundNode_;    // Static: scope bg, range rings, azimuth lines
    cocos2d::DrawNode* noiseNode_;         // Semi-static: radar noise refreshed each sweep
    cocos2d::DrawNode* sweepNode_;         // Dynamic: rotating sweep beam
    cocos2d::DrawNode* trailNode_;         // Dynamic: phosphor trail dots from blips
    cocos2d::DrawNode* overlayNode_;       // Dynamic: territory, batteries, missiles
    cocos2d::Node*     labelNode_;         // Dynamic: battery labels

    // Static background drawing (called once at init)
    void drawBackground();
    void drawRangeRings();
    void drawAzimuthLines();
    void drawCenterCrosshair();

    // Dynamic drawing (called every frame)
    void drawSweepBeam();
    void drawRadarNoise();
    void drawBatteryPositions();
    void drawTerritoryZone();
    void drawMissileTrails();
    void drawTrailDots();

    // Blip entity management
    void syncBlips(float dt);
    void checkSweepContacts();

    // Returns true if the sweep passed over the given azimuth this frame
    bool sweepPassedOver(float azimuthDeg) const;

    // Noise generation
    float noiseRefreshTimer_;
};

#else
// ============================================================================
// Stub RadarDisplay (no cocos2d-x)
// ============================================================================
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
    float getRadius() const { return radius_; }

    int findNearestTrack(float localX, float localY, float maxDist = 20.0f) const;

    void setShowNoise(bool show) { showNoise_ = show; }

    float kmToPixels(float km) const;

private:
    float radius_ = 0;
    float sweepRadius_ = 0;
    float sweepAngle_ = 0;
    int selectedTrackId_ = -1;
    TrackManager* trackManager_ = nullptr;
    FireControlSystem* fireControl_ = nullptr;
    bool showNoise_ = true;
};
#endif

#endif // __RADAR_DISPLAY_H__
