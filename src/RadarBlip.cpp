#include "RadarBlip.h"
#include <cmath>
#include <algorithm>
#include <cstdio>

#if USE_COCOS2DX
// ============================================================================
// Cocos2d-x RadarBlip — Authentic PPI phosphor radar contact
// ============================================================================

// Phosphor decay constants (emulate P7 long-persistence CRT phosphor)
static constexpr float PHOSPHOR_PEAK_DURATION = 0.3f;  // seconds at peak after sweep
static constexpr float PHOSPHOR_HALF_LIFE = 2.5f;       // seconds to decay to 50%
static constexpr float PHOSPHOR_FLOOR = 0.0f;           // fully invisible when decayed

RadarBlip* RadarBlip::create()
{
    auto* ret = new (std::nothrow) RadarBlip();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool RadarBlip::init()
{
    if (!Node::init()) return false;

    trackData_ = {};
    phosphorIntensity_ = 0.0f;
    phosphorDecayRate_ = 0.693f / PHOSPHOR_HALF_LIFE;  // ln(2)/halflife
    timeSinceSweep_ = 999.0f;
    trailRecordTimer_ = 0.0f;
    trackOverlayEnabled_ = false;
    selected_ = false;
    selectionPulseTimer_ = 0.0f;

    // Phosphor tick layer (raw radar return)
    phosphorNode_ = cocos2d::DrawNode::create();
    addChild(phosphorNode_, 0);

    // Track data overlay layer (IFF classification + data block)
    trackNode_ = cocos2d::DrawNode::create();
    addChild(trackNode_, 1);

    return true;
}

void RadarBlip::setTrackData(const TrackData& data)
{
    trackData_ = data;
}

void RadarBlip::onSweepContact()
{
    // Flare the phosphor to peak brightness
    phosphorIntensity_ = 1.0f;
    timeSinceSweep_ = 0.0f;

    // Record trail dot at current position
    trailRecordTimer_ += TRAIL_RECORD_INTERVAL;  // force record
    if (trailRecordTimer_ >= TRAIL_RECORD_INTERVAL) {
        trailRecordTimer_ = 0.0f;
        trail_.push_back({trackData_.azimuth, trackData_.range, 1.0f});
        if ((int)trail_.size() > MAX_TRAIL_DOTS) {
            trail_.erase(trail_.begin());
        }
    }
}

void RadarBlip::setScreenPosition(const cocos2d::Vec2& pos)
{
    setPosition(pos);
}

void RadarBlip::updateBlip(float dt, float sweepAngleDeg)
{
    timeSinceSweep_ += dt;
    trailRecordTimer_ += dt;
    selectionPulseTimer_ += dt;

    // === Phosphor decay ===
    // After the initial peak hold, decay exponentially
    if (timeSinceSweep_ > PHOSPHOR_PEAK_DURATION) {
        float decayTime = timeSinceSweep_ - PHOSPHOR_PEAK_DURATION;
        phosphorIntensity_ = std::exp(-phosphorDecayRate_ * decayTime);
    }

    // Clamp to floor
    if (phosphorIntensity_ < PHOSPHOR_FLOOR) {
        phosphorIntensity_ = PHOSPHOR_FLOOR;
    }

    // === Decay trail dots ===
    for (auto& dot : trail_) {
        dot.intensity *= std::exp(-phosphorDecayRate_ * dt * 0.5f);  // slower trail decay
    }
    // Remove fully faded dots
    trail_.erase(
        std::remove_if(trail_.begin(), trail_.end(),
            [](const PhosphorTrailDot& d) { return d.intensity < 0.01f; }),
        trail_.end());

    // === Redraw ===
    phosphorNode_->clear();
    trackNode_->clear();
    // Remove old labels from trackNode_
    trackNode_->removeAllChildren();

    if (isVisible()) {
        drawPhosphorTick();

        if (trackOverlayEnabled_ &&
            trackData_.classification != TrackClassification::PENDING) {
            drawTrackOverlay();
        }

        if (selected_) {
            drawSelectionBracket();
        }
    }
}

// ============================================================================
// Phosphor tick — the raw radar return
// ============================================================================

void RadarBlip::drawPhosphorTick()
{
    float intensity = phosphorIntensity_;
    float size = getTickSize();

    // The phosphor tick: a small bright green arc/dot
    // At peak intensity it's a bright, slightly elongated mark
    // As it fades, it dims and shrinks

    // Core tick (bright green phosphor)
    float coreAlpha = intensity;
    float coreGreen = 0.4f + 0.6f * intensity;  // brighter green at peak
    cocos2d::Color4F coreColor(0.0f, coreGreen, 0.0f, coreAlpha);

    // Draw as a small filled circle (the "raised" phosphor dot)
    phosphorNode_->drawSolidCircle(
        cocos2d::Vec2::ZERO, size * intensity + 1.0f, 0, 8,
        coreColor);

    // Bright center point at peak
    if (intensity > 0.7f) {
        float peakAlpha = (intensity - 0.7f) / 0.3f;
        phosphorNode_->drawSolidCircle(
            cocos2d::Vec2::ZERO, size * 0.5f, 0, 6,
            cocos2d::Color4F(0.3f * peakAlpha, 1.0f, 0.3f * peakAlpha, peakAlpha));
    }

    // Phosphor glow halo (faint bloom around the tick)
    if (intensity > 0.15f) {
        float glowAlpha = intensity * 0.2f;
        phosphorNode_->drawSolidCircle(
            cocos2d::Vec2::ZERO, size * 2.0f + 2.0f, 0, 10,
            cocos2d::Color4F(0.0f, 0.5f, 0.0f, glowAlpha));
    }
}

// ============================================================================
// Track data overlay — IFF classification + data block
// ============================================================================

void RadarBlip::drawTrackOverlay()
{
    float intensity = phosphorIntensity_;
    cocos2d::Color4F clsColor = getClassificationColor();

    // Scale overlay alpha with phosphor intensity (track fades with blip)
    float overlayAlpha = std::min(intensity + 0.1f, 1.0f);
    clsColor.a = overlayAlpha * 0.85f;

    float size = getTickSize() + 3.0f;

    // === Classification symbol around the blip ===
    if (trackData_.classification == TrackClassification::HOSTILE) {
        // Hostile: diamond shape
        cocos2d::Vec2 verts[] = {
            {0, size}, {size, 0}, {0, -size}, {-size, 0}
        };
        trackNode_->drawPolygon(verts, 4,
            cocos2d::Color4F(0, 0, 0, 0), 1.0f, clsColor);

    } else if (trackData_.classification == TrackClassification::FRIENDLY) {
        // Friendly: circle
        trackNode_->drawCircle(
            cocos2d::Vec2::ZERO, size, 0, 12, false, clsColor);

    } else {
        // Unknown: square
        trackNode_->drawRect(
            cocos2d::Vec2(-size, -size),
            cocos2d::Vec2(size, size),
            clsColor);
    }

    // === Data block (track ID, altitude, speed/heading) ===
    // Positioned to the upper-right of the blip
    float dbX = size + 4.0f;
    float dbY = size + 2.0f;

    // Leader line from blip to data block
    trackNode_->drawLine(
        cocos2d::Vec2(size * 0.7f, size * 0.7f),
        cocos2d::Vec2(dbX, dbY),
        cocos2d::Color4F(clsColor.r, clsColor.g, clsColor.b, overlayAlpha * 0.5f));

    // Track ID
    auto* idLabel = cocos2d::Label::createWithSystemFont(
        trackData_.getTrackIdString(), "Courier", 8);
    idLabel->setPosition(cocos2d::Vec2(dbX, dbY));
    idLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
    idLabel->setTextColor(cocos2d::Color4B(
        (uint8_t)(clsColor.r * 255), (uint8_t)(clsColor.g * 255),
        (uint8_t)(clsColor.b * 255), (uint8_t)(overlayAlpha * 220)));
    trackNode_->addChild(idLabel);

    // Altitude readout
    char altBuf[16];
    if (trackData_.altitude >= 1000.0f) {
        snprintf(altBuf, sizeof(altBuf), "A%02d", (int)(trackData_.altitude / 1000.0f));
    } else {
        snprintf(altBuf, sizeof(altBuf), "A%03d", (int)(trackData_.altitude / 100.0f));
    }
    auto* altLabel = cocos2d::Label::createWithSystemFont(altBuf, "Courier", 7);
    altLabel->setPosition(cocos2d::Vec2(dbX, dbY - 9));
    altLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
    altLabel->setTextColor(cocos2d::Color4B(
        (uint8_t)(clsColor.r * 255), (uint8_t)(clsColor.g * 255),
        (uint8_t)(clsColor.b * 255), (uint8_t)(overlayAlpha * 180)));
    trackNode_->addChild(altLabel);

    // Speed/heading vector indicator (small line showing direction of travel)
    float hdgRad = trackData_.heading * M_PI / 180.0f;
    float vecLen = std::min(trackData_.speed / 100.0f, 12.0f);
    cocos2d::Vec2 vecEnd(sinf(hdgRad) * vecLen, cosf(hdgRad) * vecLen);
    trackNode_->drawLine(
        cocos2d::Vec2::ZERO, vecEnd,
        cocos2d::Color4F(clsColor.r, clsColor.g, clsColor.b, overlayAlpha * 0.6f));
}

// ============================================================================
// Selection bracket — animated pulsing tactical bracket
// ============================================================================

void RadarBlip::drawSelectionBracket()
{
    float pulse = 0.6f + 0.4f * sinf(selectionPulseTimer_ * 3.0f * 2.0f * M_PI);
    float size = getTickSize() + 8.0f + pulse * 2.0f;

    cocos2d::Color4F bracketColor(1.0f, 1.0f, 1.0f, 0.7f * pulse);
    float bLen = size * 0.5f;

    // Corner brackets
    // Top-left
    trackNode_->drawLine({-size, size}, {-size, size - bLen}, bracketColor);
    trackNode_->drawLine({-size, size}, {-size + bLen, size}, bracketColor);
    // Top-right
    trackNode_->drawLine({size, size}, {size, size - bLen}, bracketColor);
    trackNode_->drawLine({size, size}, {size - bLen, size}, bracketColor);
    // Bottom-left
    trackNode_->drawLine({-size, -size}, {-size, -size + bLen}, bracketColor);
    trackNode_->drawLine({-size, -size}, {-size + bLen, -size}, bracketColor);
    // Bottom-right
    trackNode_->drawLine({size, -size}, {size, -size + bLen}, bracketColor);
    trackNode_->drawLine({size, -size}, {size - bLen, -size}, bracketColor);
}

// ============================================================================
// Helpers
// ============================================================================

float RadarBlip::getTickSize() const
{
    // RCS-based tick size (bigger aircraft = bigger radar return)
    switch (trackData_.aircraftType) {
        case AircraftType::STRATEGIC_BOMBER:  return 5.0f;
        case AircraftType::CIVILIAN_AIRLINER: return 5.0f;
        case AircraftType::TACTICAL_BOMBER:   return 4.0f;
        case AircraftType::FRIENDLY_MILITARY: return 3.5f;
        case AircraftType::FIGHTER_ATTACK:    return 3.0f;
        case AircraftType::ATTACK_DRONE:      return 2.5f;
        case AircraftType::RECON_DRONE:       return 2.0f;
        case AircraftType::STEALTH_FIGHTER:   return 1.5f;  // small RCS
    }
    return 3.0f;
}

cocos2d::Color4F RadarBlip::getClassificationColor() const
{
    switch (trackData_.classification) {
        case TrackClassification::HOSTILE:
            return cocos2d::Color4F(1.0f, 0.15f, 0.1f, 1.0f);
        case TrackClassification::FRIENDLY:
            return cocos2d::Color4F(0.2f, 0.5f, 1.0f, 1.0f);
        case TrackClassification::UNKNOWN:
            return cocos2d::Color4F(1.0f, 0.9f, 0.1f, 1.0f);
        case TrackClassification::PENDING:
            return cocos2d::Color4F(0.5f, 0.5f, 0.5f, 1.0f);
    }
    return cocos2d::Color4F(0.0f, 1.0f, 0.0f, 1.0f);
}

#else
// ============================================================================
// Stub RadarBlip (no cocos2d-x)
// ============================================================================

static constexpr float PHOSPHOR_PEAK_DURATION = 0.3f;
static constexpr float PHOSPHOR_HALF_LIFE = 2.5f;

RadarBlip* RadarBlip::create()
{
    auto* blip = new RadarBlip();
    if (blip->init()) return blip;
    delete blip;
    return nullptr;
}

bool RadarBlip::init()
{
    trackData_ = {};
    phosphorIntensity_ = 0.0f;
    timeSinceSweep_ = 999.0f;
    trackOverlayEnabled_ = false;
    selected_ = false;
    return true;
}

void RadarBlip::updateBlip(float dt, float sweepAngleDeg)
{
    timeSinceSweep_ += dt;
    float decayRate = 0.693f / PHOSPHOR_HALF_LIFE;

    if (timeSinceSweep_ > PHOSPHOR_PEAK_DURATION) {
        float decayTime = timeSinceSweep_ - PHOSPHOR_PEAK_DURATION;
        phosphorIntensity_ = std::exp(-decayRate * decayTime);
    }
    if (phosphorIntensity_ < 0.0f) phosphorIntensity_ = 0.0f;
}
#endif
