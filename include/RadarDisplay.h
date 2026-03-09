#ifndef __RADAR_DISPLAY_H__
#define __RADAR_DISPLAY_H__

#include "GameTypes.h"
#include "TrackManager.h"
#include "FireControlSystem.h"
#include <vector>

#if USE_COCOS2DX
#include "cocos2d.h"

// ============================================================================
// Radar PPI Display - renders the AN/TSQ-73 radar console view
//
// Phase 2 Implementation:
//   - Full PPI scope with dark background, range rings, azimuth lines
//   - Rotating sweep beam with phosphor glow trail (~30-degree fade)
//   - Radar blips: color-coded by IFF, sized by RCS, phosphor fade decay
//   - Track ID labels adjacent to blips
//   - Track history trails (last N positions as fading dots)
//   - Click-to-select with animated pulsing bracket
//   - Battery position indicators (square=Patriot, triangle=Hawk)
//   - Territory defense zone ring
//   - Radar noise/clutter dots
//   - Missile flight trails for active engagements
// ============================================================================

struct BlipTrailPoint {
    float azimuth;
    float range;
    float age;     // seconds since recorded
};

struct BlipState {
    int trackId;
    float brightness;          // 0.0 - 1.0 based on sweep timing
    std::vector<BlipTrailPoint> trail;  // History trail points
};

class RadarDisplay : public cocos2d::Node {
public:
    static RadarDisplay* create(float radius);
    virtual bool init(float radius);

    void update(float dt) override;

    // Data sources
    void setTrackManager(TrackManager* mgr) { trackManager_ = mgr; }
    void setFireControlSystem(FireControlSystem* fcs) { fireControl_ = fcs; }

    // Interaction
    void setSelectedTrack(int trackId) { selectedTrackId_ = trackId; }
    int getSelectedTrack() const { return selectedTrackId_; }

    // Get display properties
    float getSweepAngle() const { return sweepAngle_; }
    float getRadius() const { return radius_; }

    // Find nearest track to a screen-space point (relative to this node)
    // Returns trackId or -1 if none within threshold
    int findNearestTrack(const cocos2d::Vec2& localPos, float maxPixelDist = 20.0f) const;

    // Convert polar coordinates to screen-space position relative to this node
    cocos2d::Vec2 polarToScreen(float range, float azimuth) const;
    float kmToPixels(float km) const;

    // Toggle options
    void setShowTrackLabels(bool show) { showTrackLabels_ = show; }
    void setShowTrackTrails(bool show) { showTrackTrails_ = show; }
    void setShowNoise(bool show) { showNoise_ = show; }

private:
    float radius_;
    float sweepAngle_;
    int selectedTrackId_;
    float selectionPulseTimer_;

    TrackManager* trackManager_;
    FireControlSystem* fireControl_;

    // Display options
    bool showTrackLabels_;
    bool showTrackTrails_;
    bool showNoise_;

    // Per-blip state (brightness, trail history)
    std::vector<BlipState> blipStates_;

    // Drawing layers (ordered by z-depth)
    cocos2d::DrawNode* backgroundNode_;    // Static: scope bg, range rings, azimuth lines
    cocos2d::DrawNode* noiseNode_;         // Semi-static: radar noise refreshed each sweep
    cocos2d::DrawNode* sweepNode_;         // Dynamic: rotating sweep beam
    cocos2d::DrawNode* trailNode_;         // Dynamic: track history trails
    cocos2d::DrawNode* blipNode_;          // Dynamic: radar blips
    cocos2d::DrawNode* overlayNode_;       // Dynamic: selection, territory, batteries
    cocos2d::Node*     labelNode_;         // Dynamic: track ID text labels

    // Static background drawing (called once at init)
    void drawBackground();
    void drawRangeRings();
    void drawAzimuthLines();
    void drawCenterCrosshair();

    // Dynamic drawing (called every frame)
    void drawSweepBeam();
    void drawRadarNoise();
    void drawBlips();
    void drawTrackTrails();
    void drawTrackLabels();
    void drawSelectedTrackHighlight();
    void drawBatteryPositions();
    void drawTerritoryZone();
    void drawMissileTrails();

    // Blip state management
    void updateBlipStates(float dt);
    BlipState* getBlipState(int trackId);
    void recordTrailPoint(BlipState& state, float azimuth, float range);

    // Phosphor brightness: 1.0 when sweep just passed, decaying toward 0
    float calculatePhosphorBrightness(float blipAzimuth) const;

    // Blip size from radar cross section
    float getBlipSize(AircraftType type) const;

    // IFF classification -> blip color
    cocos2d::Color4F getBlipColor(TrackClassification cls, float brightness) const;

    // Noise generation
    float noiseRefreshTimer_;
    float lastNoiseSweepAngle_;
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

    void setShowTrackLabels(bool show) { showTrackLabels_ = show; }
    void setShowTrackTrails(bool show) { showTrackTrails_ = show; }
    void setShowNoise(bool show) { showNoise_ = show; }

    float kmToPixels(float km) const;

private:
    float radius_ = 0;
    float sweepAngle_ = 0;
    int selectedTrackId_ = -1;
    TrackManager* trackManager_ = nullptr;
    FireControlSystem* fireControl_ = nullptr;
    bool showTrackLabels_ = true;
    bool showTrackTrails_ = true;
    bool showNoise_ = true;
};
#endif

#endif // __RADAR_DISPLAY_H__
