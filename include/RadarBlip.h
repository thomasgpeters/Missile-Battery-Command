#ifndef __RADAR_BLIP_H__
#define __RADAR_BLIP_H__

#include "GameTypes.h"
#include <string>
#include <vector>

#if USE_COCOS2DX
#include "cocos2d.h"

// ============================================================================
// RadarBlip — A single radar contact on the PPI display
//
// Models the authentic CRT phosphor behavior of the AN/TSQ-73:
//   - The blip is INVISIBLE until the rotating sweep beam paints it
//   - On contact, the phosphor "tick" flares bright green then decays
//   - IFF interrogation enables a track data overlay (classification,
//     track ID, altitude, speed, heading) that follows the blip
//   - The track overlay uses classification color coding:
//       HOSTILE=red, FRIENDLY=blue, UNKNOWN=yellow, PENDING=gray
//
// Each RadarBlip is a cocos2d::Node containing:
//   - phosphorNode_:  the raw radar return (green phosphor tick)
//   - trackNode_:     IFF track overlay (symbol + data block)
// Both render relative to the blip's position in the PPI scope.
// ============================================================================

// Trail dot from previous sweep positions
struct PhosphorTrailDot {
    float azimuth;
    float range;
    float intensity;   // 0.0 (invisible) to 1.0 (peak phosphor)
};

class RadarBlip : public cocos2d::Node {
public:
    static RadarBlip* create();
    virtual bool init() override;

    // === Aircraft data feed (updated each frame by RadarDisplay) ===
    void setTrackData(const TrackData& data);
    const TrackData& getTrackData() const { return trackData_; }
    int getTrackId() const { return trackData_.trackId; }

    // === Sweep interaction ===
    // Called by RadarDisplay when the sweep beam passes over this blip's azimuth.
    // Flares the phosphor to peak brightness.
    void onSweepContact();

    // Returns true if the sweep has recently painted this blip
    // (phosphor intensity > threshold)
    bool isVisible() const { return phosphorIntensity_ > 0.02f; }

    // Current phosphor brightness (0.0 - 1.0)
    float getPhosphorIntensity() const { return phosphorIntensity_; }

    // === IFF Track overlay ===
    // Enable/disable the track data block overlay.
    // Once IFF interrogates the contact, the console paints a track.
    void setTrackOverlayEnabled(bool enabled) { trackOverlayEnabled_ = enabled; }
    bool isTrackOverlayEnabled() const { return trackOverlayEnabled_; }

    // === Trail history ===
    const std::vector<PhosphorTrailDot>& getTrail() const { return trail_; }

    // === Rendering update (called by RadarDisplay each frame) ===
    void updateBlip(float dt, float sweepAngleDeg);

    // === Screen position (set by RadarDisplay based on polar->screen conversion) ===
    void setScreenPosition(const cocos2d::Vec2& pos);

    // === Selection state ===
    void setSelected(bool sel) { selected_ = sel; }
    bool isSelected() const { return selected_; }

private:
    TrackData trackData_;

    // Phosphor state
    float phosphorIntensity_;      // 0.0 = invisible, 1.0 = just swept
    float phosphorDecayRate_;      // decay speed (per second)
    float timeSinceSweep_;         // seconds since last sweep contact

    // Trail (previous positions, each with fading intensity)
    std::vector<PhosphorTrailDot> trail_;
    float trailRecordTimer_;
    static constexpr int MAX_TRAIL_DOTS = 8;
    static constexpr float TRAIL_RECORD_INTERVAL = 1.5f;

    // IFF track overlay
    bool trackOverlayEnabled_;

    // Selection
    bool selected_;
    float selectionPulseTimer_;

    // Draw nodes
    cocos2d::DrawNode* phosphorNode_;   // Raw radar return (green tick)
    cocos2d::DrawNode* trackNode_;      // IFF track overlay

    // Internal rendering
    void drawPhosphorTick();
    void drawTrackOverlay();
    void drawSelectionBracket();

    // Phosphor tick size based on radar cross section
    float getTickSize() const;

    // Track classification -> overlay color
    cocos2d::Color4F getClassificationColor() const;
};

#else
// ============================================================================
// Stub RadarBlip (no cocos2d-x)
// ============================================================================
class RadarBlip {
public:
    static RadarBlip* create();
    bool init();

    void setTrackData(const TrackData& data) { trackData_ = data; }
    const TrackData& getTrackData() const { return trackData_; }
    int getTrackId() const { return trackData_.trackId; }

    void onSweepContact() { phosphorIntensity_ = 1.0f; timeSinceSweep_ = 0.0f; }
    bool isVisible() const { return phosphorIntensity_ > 0.02f; }
    float getPhosphorIntensity() const { return phosphorIntensity_; }

    void setTrackOverlayEnabled(bool enabled) { trackOverlayEnabled_ = enabled; }
    bool isTrackOverlayEnabled() const { return trackOverlayEnabled_; }

    void updateBlip(float dt, float sweepAngleDeg);

    void setSelected(bool sel) { selected_ = sel; }
    bool isSelected() const { return selected_; }

private:
    TrackData trackData_;
    float phosphorIntensity_ = 0.0f;
    float timeSinceSweep_ = 999.0f;
    bool trackOverlayEnabled_ = false;
    bool selected_ = false;
};
#endif

#endif // __RADAR_BLIP_H__
