#include "RadarDisplay.h"
#include <cmath>
#include <random>
#include <algorithm>

#if USE_COCOS2DX
// ============================================================================
// Cocos2d-x Radar PPI Display — RadarBlip entity-based rendering
// ============================================================================

static constexpr float SWEEP_TRAIL_DEGREES = 30.0f;
static constexpr int   SWEEP_TRAIL_SEGMENTS = 20;
// Sweep beam width: contacts within this many degrees of the beam are "painted"
static constexpr float SWEEP_BEAM_WIDTH = 3.0f;

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
    prevSweepAngle_ = 0.0f;
    selectedTrackId_ = -1;
    trackManager_ = nullptr;
    fireControl_ = nullptr;
    showNoise_ = true;
    noiseRefreshTimer_ = 0.0f;

    // Drawing layers (order matters for z-depth)
    backgroundNode_ = cocos2d::DrawNode::create();
    noiseNode_      = cocos2d::DrawNode::create();
    sweepNode_      = cocos2d::DrawNode::create();
    trailNode_      = cocos2d::DrawNode::create();
    blipContainer_  = cocos2d::Node::create();
    overlayNode_    = cocos2d::DrawNode::create();
    labelNode_      = cocos2d::Node::create();

    addChild(backgroundNode_, 0);
    addChild(noiseNode_,      1);
    addChild(sweepNode_,      2);
    addChild(trailNode_,      3);
    addChild(blipContainer_,  4);   // RadarBlip entities live here
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
    prevSweepAngle_ = sweepAngle_;
    sweepAngle_ += GameConstants::RADAR_SWEEP_RATE_DPS * dt;
    if (sweepAngle_ >= 360.0f) sweepAngle_ -= 360.0f;

    // Sync blip entities with track manager (add/remove as needed)
    syncBlips(dt);

    // Check which blips the sweep just passed over — trigger phosphor flare
    checkSweepContacts();

    // Refresh noise once per full sweep revolution
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
    overlayNode_->clear();
    labelNode_->removeAllChildren();

    // Draw dynamic elements
    drawSweepBeam();
    drawTrailDots();
    drawBatteryPositions();
    drawTerritoryZone();
    drawMissileTrails();
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
        cocos2d::Vec2::ZERO, radius_, 0, 72, false,
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

    int noiseCount = 120;
    for (int i = 0; i < noiseCount; i++) {
        float angle = angleDist(rng);
        float rangeFrac = rangeDist(rng);
        rangeFrac = rangeFrac * rangeFrac;  // more noise near center

        float r = rangeFrac * radius_;
        float rad = angle * M_PI / 180.0f;
        cocos2d::Vec2 pos(r * std::sin(rad), r * std::cos(rad));

        float alpha = alphaDist(rng);
        if (rangeFrac < 0.15f) alpha *= 2.0f;
        float size = sizeDist(rng);

        noiseNode_->drawSolidCircle(pos, size, 0, 4,
            cocos2d::Color4F(0.0f, 0.5f, 0.0f, alpha));
    }
}

// ============================================================================
// RadarBlip entity management
// ============================================================================

void RadarDisplay::setSelectedTrack(int trackId)
{
    // Deselect old
    if (selectedTrackId_ >= 0) {
        auto it = blips_.find(selectedTrackId_);
        if (it != blips_.end()) it->second->setSelected(false);
    }
    selectedTrackId_ = trackId;
    // Select new
    if (trackId >= 0) {
        auto it = blips_.find(trackId);
        if (it != blips_.end()) it->second->setSelected(true);
    }
}

void RadarDisplay::syncBlips(float dt)
{
    if (!trackManager_) return;

    auto tracks = trackManager_->getAllTracks();

    // Build set of active track IDs
    std::vector<int> activeIds;
    for (const auto& track : tracks) {
        if (!track.isAlive) continue;
        activeIds.push_back(track.trackId);

        auto it = blips_.find(track.trackId);
        if (it == blips_.end()) {
            // New contact — create a RadarBlip entity
            auto* blip = RadarBlip::create();
            blip->setTrackData(track);
            blipContainer_->addChild(blip);
            blips_[track.trackId] = blip;
        } else {
            // Existing blip — update its track data
            it->second->setTrackData(track);
        }

        // Update blip position and state
        auto* blip = blips_[track.trackId];
        blip->setScreenPosition(polarToScreen(track.range, track.azimuth));
        blip->updateBlip(dt, sweepAngle_);

        // Enable track overlay once IFF has interrogated (not PENDING)
        bool iffDone = (track.classification != TrackClassification::PENDING);
        blip->setTrackOverlayEnabled(iffDone);

        // Selection state
        blip->setSelected(track.trackId == selectedTrackId_);
    }

    // Remove blips for tracks that no longer exist
    std::vector<int> toRemove;
    for (auto& pair : blips_) {
        bool found = false;
        for (int id : activeIds) {
            if (id == pair.first) { found = true; break; }
        }
        if (!found) toRemove.push_back(pair.first);
    }
    for (int id : toRemove) {
        blipContainer_->removeChild(blips_[id]);
        blips_.erase(id);
    }
}

void RadarDisplay::checkSweepContacts()
{
    for (auto& pair : blips_) {
        float blipAz = pair.second->getTrackData().azimuth;
        if (sweepPassedOver(blipAz)) {
            pair.second->onSweepContact();
        }
    }
}

bool RadarDisplay::sweepPassedOver(float azimuthDeg) const
{
    // Did the sweep beam pass over this azimuth between prevSweepAngle_ and sweepAngle_?
    float prev = prevSweepAngle_;
    float curr = sweepAngle_;

    // Normalize azimuth
    float az = azimuthDeg;
    if (az < 0) az += 360.0f;
    if (az >= 360.0f) az -= 360.0f;

    // Handle the beam width — check if azimuth is within SWEEP_BEAM_WIDTH of the sweep
    // Also handle the wrap-around at 0/360
    if (curr >= prev) {
        // Normal case: no wrap
        float lo = prev - SWEEP_BEAM_WIDTH;
        float hi = curr + SWEEP_BEAM_WIDTH;
        if (lo < 0) {
            return (az >= lo + 360.0f || az <= hi);
        }
        return (az >= lo && az <= hi);
    } else {
        // Wrapped past 360
        float lo = prev - SWEEP_BEAM_WIDTH;
        float hi = curr + SWEEP_BEAM_WIDTH;
        return (az >= lo || az <= hi);
    }
}

// ============================================================================
// Draw phosphor trail dots from blip history
// ============================================================================

void RadarDisplay::drawTrailDots()
{
    for (const auto& pair : blips_) {
        const auto& trail = pair.second->getTrail();
        for (const auto& dot : trail) {
            cocos2d::Vec2 pos = polarToScreen(dot.range, dot.azimuth);
            float alpha = dot.intensity * 0.35f;
            float size = 1.2f + dot.intensity;

            if (alpha > 0.02f) {
                trailNode_->drawSolidCircle(pos, size, 0, 4,
                    cocos2d::Color4F(0.0f, 0.5f, 0.0f, alpha));
            }
        }
    }
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
