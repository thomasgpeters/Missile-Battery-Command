#include "RadarDisplay.h"
#include <cmath>
#include <random>
#include <algorithm>

#if USE_COCOS2DX
// ============================================================================
// Cocos2d-x Radar PPI Display — Phase 2 Full Implementation
// ============================================================================

static constexpr int   TRAIL_MAX_POINTS = 8;
static constexpr float TRAIL_RECORD_INTERVAL = 1.5f;  // seconds between trail dots
static constexpr float TRAIL_MAX_AGE = 15.0f;         // seconds before trail fades out
static constexpr float NOISE_REFRESH_INTERVAL = 10.0f; // refresh noise once per sweep
static constexpr float SELECTION_PULSE_SPEED = 3.0f;   // Hz for pulsing bracket
static constexpr float SWEEP_TRAIL_DEGREES = 30.0f;    // trailing glow arc
static constexpr int   SWEEP_TRAIL_SEGMENTS = 20;      // segments in the trail arc

RadarDisplay* RadarDisplay::create(float radius)
{
    auto* ret = new (std::nothrow) RadarDisplay();
    if (ret && ret->init(radius)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool RadarDisplay::init(float radius)
{
    if (!Node::init()) return false;

    radius_ = radius;
    sweepAngle_ = 0.0f;
    selectedTrackId_ = -1;
    selectionPulseTimer_ = 0.0f;
    trackManager_ = nullptr;
    fireControl_ = nullptr;
    showTrackLabels_ = true;
    showTrackTrails_ = true;
    showNoise_ = true;
    noiseRefreshTimer_ = 0.0f;
    lastNoiseSweepAngle_ = 0.0f;

    // Create draw nodes for layered rendering (order matters)
    backgroundNode_ = cocos2d::DrawNode::create();
    noiseNode_      = cocos2d::DrawNode::create();
    sweepNode_      = cocos2d::DrawNode::create();
    trailNode_      = cocos2d::DrawNode::create();
    blipNode_       = cocos2d::DrawNode::create();
    overlayNode_    = cocos2d::DrawNode::create();
    labelNode_      = cocos2d::Node::create();

    addChild(backgroundNode_, 0);
    addChild(noiseNode_,      1);
    addChild(sweepNode_,      2);
    addChild(trailNode_,      3);
    addChild(blipNode_,       4);
    addChild(overlayNode_,    5);
    addChild(labelNode_,      6);

    drawBackground();

    scheduleUpdate();
    return true;
}

// ============================================================================
// Frame update
// ============================================================================

void RadarDisplay::update(float dt)
{
    // Advance sweep beam
    float prevAngle = sweepAngle_;
    sweepAngle_ += GameConstants::RADAR_SWEEP_RATE_DPS * dt;
    if (sweepAngle_ >= 360.0f) sweepAngle_ -= 360.0f;

    // Pulse timer for selection highlight
    selectionPulseTimer_ += dt;

    // Update per-blip state (brightness, trail recording)
    updateBlipStates(dt);

    // Refresh noise once per full sweep
    noiseRefreshTimer_ += dt;
    float sweepPeriod = 60.0f / GameConstants::RADAR_SWEEP_RATE_RPM;
    if (noiseRefreshTimer_ >= sweepPeriod) {
        noiseRefreshTimer_ = 0.0f;
        if (showNoise_) {
            noiseNode_->clear();
            drawRadarNoise();
        }
    }

    // Clear dynamic layers
    sweepNode_->clear();
    trailNode_->clear();
    blipNode_->clear();
    overlayNode_->clear();
    labelNode_->removeAllChildren();

    // Draw dynamic elements
    drawSweepBeam();
    if (showTrackTrails_) drawTrackTrails();
    drawBlips();
    if (showTrackLabels_) drawTrackLabels();
    drawBatteryPositions();
    drawTerritoryZone();
    drawMissileTrails();

    if (selectedTrackId_ >= 0) {
        drawSelectedTrackHighlight();
    }
}

// ============================================================================
// Static background (drawn once)
// ============================================================================

void RadarDisplay::drawBackground()
{
    backgroundNode_->clear();

    // Dark scope background
    backgroundNode_->drawSolidCircle(
        cocos2d::Vec2::ZERO, radius_, 0, 72,
        cocos2d::Color4F(0.0f, 0.03f, 0.0f, 1.0f));

    // Outer rim (bright green border)
    backgroundNode_->drawCircle(
        cocos2d::Vec2::ZERO, radius_, 0, 72, false, 1.5f,
        cocos2d::Color4F(0.0f, 0.7f, 0.0f, 0.9f));

    drawRangeRings();
    drawAzimuthLines();
    drawCenterCrosshair();

    // Initial noise
    if (showNoise_) drawRadarNoise();
}

void RadarDisplay::drawRangeRings()
{
    float ringSpacing = radius_ / GameConstants::RADAR_RANGE_RINGS;

    for (int i = 1; i <= GameConstants::RADAR_RANGE_RINGS; i++) {
        float ringRadius = ringSpacing * i;
        float alpha = (i == GameConstants::RADAR_RANGE_RINGS) ? 0.5f : 0.3f;

        backgroundNode_->drawCircle(
            cocos2d::Vec2::ZERO, ringRadius, 0, 72, false,
            cocos2d::Color4F(0.0f, 0.35f, 0.0f, alpha));

        // Range label in nautical miles (positioned just right of north axis)
        float rangeKm = (GameConstants::RADAR_MAX_RANGE_KM /
                         GameConstants::RADAR_RANGE_RINGS) * i;
        float rangeNm = rangeKm * GameConstants::KM_TO_NM;
        char label[16];
        snprintf(label, sizeof(label), "%.0fNM", rangeNm);
        auto* rangeLabel = cocos2d::Label::createWithSystemFont(
            label, "Courier", 9);
        rangeLabel->setPosition(cocos2d::Vec2(4, ringRadius + 3));
        rangeLabel->setTextColor(cocos2d::Color4B(0, 160, 0, 140));
        rangeLabel->setAnchorPoint(cocos2d::Vec2(0, 0));
        backgroundNode_->addChild(rangeLabel);
    }
}

void RadarDisplay::drawAzimuthLines()
{
    const char* labels[] = {
        "N", "030", "060", "E", "120", "150",
        "S", "210", "240", "W", "300", "330"
    };

    for (int i = 0; i < 12; i++) {
        float angle = i * 30.0f;
        float rad = angle * M_PI / 180.0f;

        cocos2d::Vec2 end(radius_ * std::sin(rad), radius_ * std::cos(rad));

        // Dimmer lines for non-cardinal, brighter for N/E/S/W
        float alpha = (i % 3 == 0) ? 0.35f : 0.18f;
        backgroundNode_->drawLine(
            cocos2d::Vec2::ZERO, end,
            cocos2d::Color4F(0.0f, 0.25f, 0.0f, alpha));

        // Azimuth label outside the scope edge
        cocos2d::Vec2 labelPos = end * 1.06f;
        auto* azLabel = cocos2d::Label::createWithSystemFont(
            labels[i], "Courier", (i % 3 == 0) ? 12 : 9);
        azLabel->setPosition(labelPos);
        azLabel->setTextColor(cocos2d::Color4B(
            0, (i % 3 == 0) ? 220 : 160, 0, (i % 3 == 0) ? 220 : 160));
        backgroundNode_->addChild(azLabel);
    }

    // Fine tick marks every 10 degrees on the outer rim
    for (int i = 0; i < 36; i++) {
        if (i % 3 == 0) continue;  // Skip the 30-degree lines already drawn
        float angle = i * 10.0f;
        float rad = angle * M_PI / 180.0f;

        float innerR = radius_ * 0.96f;
        cocos2d::Vec2 inner(innerR * std::sin(rad), innerR * std::cos(rad));
        cocos2d::Vec2 outer(radius_ * std::sin(rad), radius_ * std::cos(rad));

        backgroundNode_->drawLine(inner, outer,
            cocos2d::Color4F(0.0f, 0.3f, 0.0f, 0.25f));
    }
}

void RadarDisplay::drawCenterCrosshair()
{
    float len = 6.0f;
    cocos2d::Color4F color(0.0f, 0.6f, 0.0f, 0.6f);

    backgroundNode_->drawLine(
        cocos2d::Vec2(-len, 0), cocos2d::Vec2(len, 0), color);
    backgroundNode_->drawLine(
        cocos2d::Vec2(0, -len), cocos2d::Vec2(0, len), color);
}

// ============================================================================
// Sweep beam
// ============================================================================

void RadarDisplay::drawSweepBeam()
{
    float rad = sweepAngle_ * M_PI / 180.0f;
    cocos2d::Vec2 beamEnd(radius_ * std::sin(rad), radius_ * std::cos(rad));

    // Leading edge — bright green line
    sweepNode_->drawLine(
        cocos2d::Vec2::ZERO, beamEnd,
        cocos2d::Color4F(0.0f, 1.0f, 0.0f, 0.85f));

    // Slightly thicker glow line along the beam
    float perpX = -std::cos(rad);
    float perpY = std::sin(rad);
    cocos2d::Vec2 offset(perpX * 0.5f, perpY * 0.5f);
    sweepNode_->drawLine(offset, beamEnd + offset,
        cocos2d::Color4F(0.0f, 0.8f, 0.0f, 0.3f));
    sweepNode_->drawLine(-offset, beamEnd - offset,
        cocos2d::Color4F(0.0f, 0.8f, 0.0f, 0.3f));

    // Trailing phosphor glow — filled triangle wedge segments
    for (int i = 1; i <= SWEEP_TRAIL_SEGMENTS; i++) {
        float trailAngle1 = sweepAngle_ - (i - 1) * (SWEEP_TRAIL_DEGREES / SWEEP_TRAIL_SEGMENTS);
        float trailAngle2 = sweepAngle_ - i * (SWEEP_TRAIL_DEGREES / SWEEP_TRAIL_SEGMENTS);
        if (trailAngle1 < 0) trailAngle1 += 360.0f;
        if (trailAngle2 < 0) trailAngle2 += 360.0f;

        float rad1 = trailAngle1 * M_PI / 180.0f;
        float rad2 = trailAngle2 * M_PI / 180.0f;

        float alpha1 = 0.25f * (1.0f - (float)(i - 1) / SWEEP_TRAIL_SEGMENTS);
        float alpha2 = 0.25f * (1.0f - (float)i / SWEEP_TRAIL_SEGMENTS);
        float avgAlpha = (alpha1 + alpha2) * 0.5f;

        cocos2d::Vec2 p1(radius_ * std::sin(rad1), radius_ * std::cos(rad1));
        cocos2d::Vec2 p2(radius_ * std::sin(rad2), radius_ * std::cos(rad2));

        cocos2d::Vec2 verts[] = {cocos2d::Vec2::ZERO, p1, p2};
        sweepNode_->drawPolygon(verts, 3,
            cocos2d::Color4F(0.0f, 0.6f, 0.0f, avgAlpha),
            0, cocos2d::Color4F::BLACK);
    }
}

// ============================================================================
// Radar noise / clutter
// ============================================================================

void RadarDisplay::drawRadarNoise()
{
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
    std::uniform_real_distribution<float> rangeDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> alphaDist(0.03f, 0.12f);
    std::uniform_real_distribution<float> sizeDist(0.5f, 1.5f);

    // Scatter noise dots across the scope
    int noiseCount = 120;

    for (int i = 0; i < noiseCount; i++) {
        float angle = angleDist(rng);
        float rangeFrac = rangeDist(rng);

        // More noise near center (ground clutter)
        rangeFrac = rangeFrac * rangeFrac;

        float r = rangeFrac * radius_;
        float rad = angle * M_PI / 180.0f;
        cocos2d::Vec2 pos(r * std::sin(rad), r * std::cos(rad));

        float alpha = alphaDist(rng);
        // Ground clutter is denser/brighter near center
        if (rangeFrac < 0.15f) alpha *= 2.0f;

        float size = sizeDist(rng);

        noiseNode_->drawSolidCircle(pos, size, 0, 4,
            cocos2d::Color4F(0.0f, 0.5f, 0.0f, alpha));
    }
}

// ============================================================================
// Blip state management
// ============================================================================

void RadarDisplay::updateBlipStates(float dt)
{
    if (!trackManager_) return;

    auto tracks = trackManager_->getAllTracks();

    // Remove stale blip states for tracks that no longer exist
    blipStates_.erase(
        std::remove_if(blipStates_.begin(), blipStates_.end(),
            [&tracks](const BlipState& bs) {
                return std::none_of(tracks.begin(), tracks.end(),
                    [&bs](const TrackData& td) { return td.trackId == bs.trackId; });
            }),
        blipStates_.end());

    // Update or create blip states for each track
    for (const auto& track : tracks) {
        if (!track.isAlive) continue;

        BlipState* state = getBlipState(track.trackId);
        if (!state) {
            blipStates_.push_back({track.trackId, 0.0f, {}});
            state = &blipStates_.back();
        }

        // Update brightness based on sweep position
        state->brightness = calculatePhosphorBrightness(track.azimuth);

        // Record trail point periodically
        if (state->trail.empty() ||
            state->trail.back().age >= TRAIL_RECORD_INTERVAL) {
            recordTrailPoint(*state, track.azimuth, track.range);
        }

        // Age trail points and remove old ones
        for (auto& pt : state->trail) {
            pt.age += dt;
        }
        state->trail.erase(
            std::remove_if(state->trail.begin(), state->trail.end(),
                [](const BlipTrailPoint& p) { return p.age > TRAIL_MAX_AGE; }),
            state->trail.end());
    }
}

BlipState* RadarDisplay::getBlipState(int trackId)
{
    for (auto& bs : blipStates_) {
        if (bs.trackId == trackId) return &bs;
    }
    return nullptr;
}

void RadarDisplay::recordTrailPoint(BlipState& state, float azimuth, float range)
{
    state.trail.push_back({azimuth, range, 0.0f});
    if ((int)state.trail.size() > TRAIL_MAX_POINTS) {
        state.trail.erase(state.trail.begin());
    }
}

float RadarDisplay::calculatePhosphorBrightness(float blipAzimuth) const
{
    // How many degrees ago did the sweep pass over this azimuth?
    float angleDiff = sweepAngle_ - blipAzimuth;
    if (angleDiff < 0) angleDiff += 360.0f;

    // 0 = sweep just passed (brightest), 360 = full revolution ago (dimmest)
    float fadeRatio = angleDiff / 360.0f;

    // Non-linear fade: bright for a while, then drops off
    // Simulates phosphor persistence
    float brightness = 1.0f - std::pow(fadeRatio, 0.6f);
    return std::max(0.08f, brightness);
}

float RadarDisplay::getBlipSize(AircraftType type) const
{
    switch (type) {
        case AircraftType::STRATEGIC_BOMBER:  return 5.0f;
        case AircraftType::CIVILIAN_AIRLINER: return 5.0f;
        case AircraftType::TACTICAL_BOMBER:   return 4.0f;
        case AircraftType::FRIENDLY_MILITARY: return 3.5f;
        case AircraftType::FIGHTER_ATTACK:    return 3.0f;
        case AircraftType::ATTACK_DRONE:      return 2.5f;
        case AircraftType::RECON_DRONE:       return 2.0f;
        case AircraftType::STEALTH_FIGHTER:   return 2.0f;
    }
    return 3.0f;
}

cocos2d::Color4F RadarDisplay::getBlipColor(TrackClassification cls,
                                             float brightness) const
{
    switch (cls) {
        case TrackClassification::HOSTILE:
            return cocos2d::Color4F(1.0f, 0.15f, 0.1f, brightness);
        case TrackClassification::FRIENDLY:
            return cocos2d::Color4F(0.2f, 0.5f, 1.0f, brightness);
        case TrackClassification::UNKNOWN:
            return cocos2d::Color4F(1.0f, 0.9f, 0.1f, brightness);
        case TrackClassification::PENDING:
            return cocos2d::Color4F(0.5f, 0.5f, 0.5f, brightness * 0.7f);
    }
    return cocos2d::Color4F(0.0f, 1.0f, 0.0f, brightness);
}

// ============================================================================
// Draw blips
// ============================================================================

void RadarDisplay::drawBlips()
{
    if (!trackManager_) return;

    auto tracks = trackManager_->getAllTracks();
    for (const auto& track : tracks) {
        if (!track.isAlive) continue;

        cocos2d::Vec2 pos = polarToScreen(track.range, track.azimuth);

        // Get brightness from blip state
        BlipState* state = getBlipState(track.trackId);
        float brightness = state ? state->brightness : 0.5f;

        float blipSize = getBlipSize(track.aircraftType);
        cocos2d::Color4F color = getBlipColor(track.classification, brightness);

        // Main blip dot
        blipNode_->drawSolidCircle(pos, blipSize, 0, 8, color);

        // Slight glow around bright blips
        if (brightness > 0.6f) {
            float glowAlpha = (brightness - 0.6f) * 0.3f;
            blipNode_->drawSolidCircle(pos, blipSize + 2.0f, 0, 8,
                cocos2d::Color4F(color.r, color.g, color.b, glowAlpha));
        }
    }
}

// ============================================================================
// Track history trails
// ============================================================================

void RadarDisplay::drawTrackTrails()
{
    for (const auto& state : blipStates_) {
        if (state.trail.size() < 2) continue;

        for (size_t i = 0; i < state.trail.size(); i++) {
            const auto& pt = state.trail[i];
            cocos2d::Vec2 pos = polarToScreen(pt.range, pt.azimuth);

            float ageFade = 1.0f - (pt.age / TRAIL_MAX_AGE);
            float dotAlpha = ageFade * 0.4f;
            float dotSize = 1.5f * ageFade;

            if (dotAlpha > 0.02f) {
                trailNode_->drawSolidCircle(pos, dotSize, 0, 4,
                    cocos2d::Color4F(0.0f, 0.6f, 0.0f, dotAlpha));
            }
        }
    }
}

// ============================================================================
// Track ID labels
// ============================================================================

void RadarDisplay::drawTrackLabels()
{
    if (!trackManager_) return;

    auto tracks = trackManager_->getAllTracks();
    for (const auto& track : tracks) {
        if (!track.isAlive) continue;

        BlipState* state = getBlipState(track.trackId);
        float brightness = state ? state->brightness : 0.5f;

        // Only show label when blip is reasonably visible
        if (brightness < 0.15f) continue;

        cocos2d::Vec2 pos = polarToScreen(track.range, track.azimuth);
        cocos2d::Color4F color = getBlipColor(track.classification, brightness);

        auto* label = cocos2d::Label::createWithSystemFont(
            track.getTrackIdString(), "Courier", 8);
        label->setPosition(pos + cocos2d::Vec2(8, 6));
        label->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
        label->setTextColor(cocos2d::Color4B(
            (uint8_t)(color.r * 255),
            (uint8_t)(color.g * 255),
            (uint8_t)(color.b * 255),
            (uint8_t)(std::min(brightness + 0.2f, 1.0f) * 200)));
        labelNode_->addChild(label);
    }
}

// ============================================================================
// Selected track highlight (animated pulsing bracket)
// ============================================================================

void RadarDisplay::drawSelectedTrackHighlight()
{
    if (!trackManager_ || selectedTrackId_ < 0) return;

    TrackData* track = trackManager_->getTrack(selectedTrackId_);
    if (!track || !track->isAlive) {
        selectedTrackId_ = -1;
        return;
    }

    cocos2d::Vec2 pos = polarToScreen(track->range, track->azimuth);

    // Pulsing ring
    float pulse = 0.6f + 0.4f * std::sin(selectionPulseTimer_ * SELECTION_PULSE_SPEED * 2.0f * M_PI);
    float ringRadius = getBlipSize(track->aircraftType) + 5.0f + pulse * 2.0f;

    // Selection ring
    overlayNode_->drawCircle(pos, ringRadius, 0, 16, false,
        cocos2d::Color4F(1.0f, 1.0f, 1.0f, 0.6f * pulse));

    // Corner brackets (tactical display style)
    float bLen = ringRadius * 0.6f;
    float bOff = ringRadius + 2.0f;
    cocos2d::Color4F bracketColor(1.0f, 1.0f, 1.0f, 0.7f * pulse);

    // Top-left bracket
    overlayNode_->drawLine(pos + cocos2d::Vec2(-bOff, bOff),
                            pos + cocos2d::Vec2(-bOff, bOff - bLen), bracketColor);
    overlayNode_->drawLine(pos + cocos2d::Vec2(-bOff, bOff),
                            pos + cocos2d::Vec2(-bOff + bLen, bOff), bracketColor);

    // Top-right bracket
    overlayNode_->drawLine(pos + cocos2d::Vec2(bOff, bOff),
                            pos + cocos2d::Vec2(bOff, bOff - bLen), bracketColor);
    overlayNode_->drawLine(pos + cocos2d::Vec2(bOff, bOff),
                            pos + cocos2d::Vec2(bOff - bLen, bOff), bracketColor);

    // Bottom-left bracket
    overlayNode_->drawLine(pos + cocos2d::Vec2(-bOff, -bOff),
                            pos + cocos2d::Vec2(-bOff, -bOff + bLen), bracketColor);
    overlayNode_->drawLine(pos + cocos2d::Vec2(-bOff, -bOff),
                            pos + cocos2d::Vec2(-bOff + bLen, -bOff), bracketColor);

    // Bottom-right bracket
    overlayNode_->drawLine(pos + cocos2d::Vec2(bOff, -bOff),
                            pos + cocos2d::Vec2(bOff, -bOff + bLen), bracketColor);
    overlayNode_->drawLine(pos + cocos2d::Vec2(bOff, -bOff),
                            pos + cocos2d::Vec2(bOff - bLen, -bOff), bracketColor);
}

// ============================================================================
// Battery position indicators
// ============================================================================

void RadarDisplay::drawBatteryPositions()
{
    if (!fireControl_) return;

    auto batteries = fireControl_->getAllBatteryData();
    for (const auto& bat : batteries) {
        cocos2d::Vec2 pos = polarToScreen(bat.position.range, bat.position.azimuth);

        // Color by status
        cocos2d::Color4F color;
        switch (bat.status) {
            case BatteryStatus::READY:
                color = cocos2d::Color4F(0.0f, 1.0f, 0.0f, 0.8f); break;
            case BatteryStatus::TRACKING:
                color = cocos2d::Color4F(0.0f, 1.0f, 0.5f, 0.8f); break;
            case BatteryStatus::ENGAGED:
                color = cocos2d::Color4F(1.0f, 0.4f, 0.0f, 0.9f); break;
            case BatteryStatus::RELOADING:
                color = cocos2d::Color4F(1.0f, 1.0f, 0.0f, 0.6f); break;
            case BatteryStatus::DESTROYED:
                color = cocos2d::Color4F(0.5f, 0.0f, 0.0f, 0.5f); break;
            default:
                color = cocos2d::Color4F(0.4f, 0.4f, 0.4f, 0.4f); break;
        }

        if (bat.type == BatteryType::PATRIOT) {
            // Square with border
            float s = 4.0f;
            cocos2d::Vec2 verts[] = {
                pos + cocos2d::Vec2(-s, -s), pos + cocos2d::Vec2(s, -s),
                pos + cocos2d::Vec2(s, s), pos + cocos2d::Vec2(-s, s)
            };
            overlayNode_->drawPolygon(verts, 4, color, 0.5f, color);
        } else {
            // Triangle (pointing up)
            float s = 5.0f;
            cocos2d::Vec2 verts[] = {
                pos + cocos2d::Vec2(0, s),
                pos + cocos2d::Vec2(-s, -s * 0.7f),
                pos + cocos2d::Vec2(s, -s * 0.7f)
            };
            overlayNode_->drawPolygon(verts, 3, color, 0.5f, color);
        }

        // Short designation label below the icon
        const char* shortName = (bat.type == BatteryType::PATRIOT) ? "P" : "H";
        char label[4];
        snprintf(label, sizeof(label), "%s%c",
                 shortName, bat.designation.back());

        auto* batLabel = cocos2d::Label::createWithSystemFont(label, "Courier", 7);
        batLabel->setPosition(pos + cocos2d::Vec2(0, -10));
        batLabel->setTextColor(cocos2d::Color4B(
            (uint8_t)(color.r * 200),
            (uint8_t)(color.g * 200),
            (uint8_t)(color.b * 200), 180));
        labelNode_->addChild(batLabel);
    }
}

// ============================================================================
// Territory defense zone
// ============================================================================

void RadarDisplay::drawTerritoryZone()
{
    float territoryPixels = kmToPixels(GameConstants::TERRITORY_RADIUS_KM);

    // Dashed circle effect (drawn as short arc segments)
    int segments = 24;
    for (int i = 0; i < segments; i += 2) {
        float a1 = i * (360.0f / segments) * M_PI / 180.0f;
        float a2 = (i + 1) * (360.0f / segments) * M_PI / 180.0f;

        cocos2d::Vec2 p1(territoryPixels * std::sin(a1),
                          territoryPixels * std::cos(a1));
        cocos2d::Vec2 p2(territoryPixels * std::sin(a2),
                          territoryPixels * std::cos(a2));

        overlayNode_->drawLine(p1, p2,
            cocos2d::Color4F(0.0f, 0.8f, 0.0f, 0.35f));
    }
}

// ============================================================================
// Missile flight trails
// ============================================================================

void RadarDisplay::drawMissileTrails()
{
    if (!fireControl_ || !trackManager_) return;

    auto batteries = fireControl_->getAllBatteryData();
    for (const auto& bat : batteries) {
        if (bat.status != BatteryStatus::ENGAGED || bat.assignedTrackId < 0) continue;

        // Draw a line from battery to target's current position
        cocos2d::Vec2 batPos = polarToScreen(bat.position.range, bat.position.azimuth);

        TrackData* target = trackManager_->getTrack(bat.assignedTrackId);
        if (!target) continue;

        cocos2d::Vec2 targetPos = polarToScreen(target->range, target->azimuth);

        // Dashed cyan line for missile trail
        cocos2d::Vec2 dir = targetPos - batPos;
        float length = dir.length();
        if (length < 1.0f) continue;
        dir.normalize();

        float dashLen = 4.0f;
        float gapLen = 3.0f;
        float drawn = 0.0f;
        bool drawing = true;

        while (drawn < length) {
            float segLen = drawing ? dashLen : gapLen;
            float segEnd = std::min(drawn + segLen, length);

            if (drawing) {
                cocos2d::Vec2 p1 = batPos + dir * drawn;
                cocos2d::Vec2 p2 = batPos + dir * segEnd;
                overlayNode_->drawLine(p1, p2,
                    cocos2d::Color4F(0.0f, 0.9f, 1.0f, 0.6f));
            }

            drawn = segEnd;
            drawing = !drawing;
        }

        // Missile head dot (moving along the trail)
        // Approximate progress — not exact without flight time data in BatteryData
        cocos2d::Vec2 midpoint = (batPos + targetPos) * 0.5f;
        overlayNode_->drawSolidCircle(midpoint, 2.0f, 0, 6,
            cocos2d::Color4F(0.0f, 1.0f, 1.0f, 0.9f));
    }
}

// ============================================================================
// Track selection helper
// ============================================================================

int RadarDisplay::findNearestTrack(const cocos2d::Vec2& localPos,
                                    float maxPixelDist) const
{
    if (!trackManager_) return -1;

    auto tracks = trackManager_->getAllTracks();
    int nearestId = -1;
    float nearestDist = maxPixelDist;

    for (const auto& track : tracks) {
        if (!track.isAlive) continue;

        cocos2d::Vec2 blipPos = polarToScreen(track.range, track.azimuth);
        float dist = localPos.distance(blipPos);

        if (dist < nearestDist) {
            nearestDist = dist;
            nearestId = track.trackId;
        }
    }

    return nearestId;
}

// ============================================================================
// Coordinate conversion
// ============================================================================

cocos2d::Vec2 RadarDisplay::polarToScreen(float range, float azimuth) const
{
    float pixels = kmToPixels(range);
    float rad = azimuth * M_PI / 180.0f;
    return cocos2d::Vec2(pixels * std::sin(rad), pixels * std::cos(rad));
}

float RadarDisplay::kmToPixels(float km) const
{
    return (km / GameConstants::RADAR_MAX_RANGE_KM) * radius_;
}

#else
// ============================================================================
// Stub Radar Display (no cocos2d-x)
// ============================================================================

RadarDisplay* RadarDisplay::create(float radius)
{
    auto* display = new RadarDisplay();
    if (display->init(radius)) return display;
    delete display;
    return nullptr;
}

bool RadarDisplay::init(float radius)
{
    radius_ = radius;
    sweepAngle_ = 0.0f;
    selectedTrackId_ = -1;
    return true;
}

void RadarDisplay::update(float dt)
{
    sweepAngle_ += GameConstants::RADAR_SWEEP_RATE_DPS * dt;
    if (sweepAngle_ >= 360.0f) sweepAngle_ -= 360.0f;
}

int RadarDisplay::findNearestTrack(float localX, float localY, float maxDist) const
{
    if (!trackManager_) return -1;

    auto tracks = trackManager_->getAllTracks();
    int nearestId = -1;
    float nearestDist = maxDist;

    for (const auto& track : tracks) {
        if (!track.isAlive) continue;

        float rad = track.azimuth * M_PI / 180.0f;
        float pixels = kmToPixels(track.range);
        float blipX = pixels * std::sin(rad);
        float blipY = pixels * std::cos(rad);

        float dx = localX - blipX;
        float dy = localY - blipY;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < nearestDist) {
            nearestDist = dist;
            nearestId = track.trackId;
        }
    }

    return nearestId;
}

float RadarDisplay::kmToPixels(float km) const
{
    return (km / GameConstants::RADAR_MAX_RANGE_KM) * radius_;
}
#endif
