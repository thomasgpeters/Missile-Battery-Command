#include "RadarDisplay.h"
#include <cmath>

#if USE_COCOS2DX
// ============================================================================
// Cocos2d-x Radar PPI Display
// ============================================================================

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
    trackManager_ = nullptr;
    fireControl_ = nullptr;

    // Create draw nodes for layered rendering
    backgroundNode_ = cocos2d::DrawNode::create();
    sweepNode_ = cocos2d::DrawNode::create();
    blipNode_ = cocos2d::DrawNode::create();
    overlayNode_ = cocos2d::DrawNode::create();

    addChild(backgroundNode_, 0);
    addChild(sweepNode_, 1);
    addChild(blipNode_, 2);
    addChild(overlayNode_, 3);

    drawBackground();

    scheduleUpdate();
    return true;
}

void RadarDisplay::update(float dt)
{
    // Rotate sweep beam
    sweepAngle_ += GameConstants::RADAR_SWEEP_RATE_DPS * dt;
    if (sweepAngle_ >= 360.0f) sweepAngle_ -= 360.0f;

    // Redraw dynamic elements
    sweepNode_->clear();
    blipNode_->clear();
    overlayNode_->clear();

    drawSweepBeam();
    drawBlips();
    drawBatteryPositions();
    drawTerritoryZone();

    if (selectedTrackId_ >= 0) {
        drawSelectedTrackInfo();
    }
}

void RadarDisplay::drawBackground()
{
    backgroundNode_->clear();

    // Dark green/black radar background circle
    backgroundNode_->drawSolidCircle(
        cocos2d::Vec2::ZERO, radius_, 0, 64,
        cocos2d::Color4F(0.0f, 0.05f, 0.0f, 1.0f));

    // Border
    backgroundNode_->drawCircle(
        cocos2d::Vec2::ZERO, radius_, 0, 64, false,
        cocos2d::Color4F(0.0f, 0.6f, 0.0f, 1.0f));

    drawRangeRings();
    drawAzimuthLines();
}

void RadarDisplay::drawRangeRings()
{
    float ringSpacing = radius_ / GameConstants::RADAR_RANGE_RINGS;

    for (int i = 1; i <= GameConstants::RADAR_RANGE_RINGS; i++) {
        float ringRadius = ringSpacing * i;
        backgroundNode_->drawCircle(
            cocos2d::Vec2::ZERO, ringRadius, 0, 64, false,
            cocos2d::Color4F(0.0f, 0.3f, 0.0f, 0.5f));

        // Range label
        float rangeKm = (GameConstants::RADAR_MAX_RANGE_KM / GameConstants::RADAR_RANGE_RINGS) * i;
        char label[16];
        snprintf(label, sizeof(label), "%.0f", rangeKm);
        auto* rangeLabel = cocos2d::Label::createWithSystemFont(
            label, "Courier", 10);
        rangeLabel->setPosition(cocos2d::Vec2(5, ringRadius + 2));
        rangeLabel->setTextColor(cocos2d::Color4B(0, 180, 0, 150));
        rangeLabel->setAnchorPoint(cocos2d::Vec2(0, 0));
        backgroundNode_->addChild(rangeLabel);
    }
}

void RadarDisplay::drawAzimuthLines()
{
    // Draw lines every 30 degrees with labels at cardinal points
    const char* cardinals[] = {
        "N", "030", "060", "E", "120", "150",
        "S", "210", "240", "W", "300", "330"
    };

    for (int i = 0; i < 12; i++) {
        float angle = i * 30.0f;
        float rad = angle * M_PI / 180.0f;

        cocos2d::Vec2 end(radius_ * std::sin(rad), radius_ * std::cos(rad));

        backgroundNode_->drawLine(
            cocos2d::Vec2::ZERO, end,
            cocos2d::Color4F(0.0f, 0.2f, 0.0f, 0.3f));

        // Azimuth labels
        cocos2d::Vec2 labelPos = end * 1.05f;
        auto* azLabel = cocos2d::Label::createWithSystemFont(
            cardinals[i], "Courier", 11);
        azLabel->setPosition(labelPos);
        azLabel->setTextColor(cocos2d::Color4B(0, 200, 0, 200));
        backgroundNode_->addChild(azLabel);
    }
}

void RadarDisplay::drawSweepBeam()
{
    float rad = sweepAngle_ * M_PI / 180.0f;
    cocos2d::Vec2 beamEnd(radius_ * std::sin(rad), radius_ * std::cos(rad));

    // Sweep beam - bright green line
    sweepNode_->drawLine(
        cocos2d::Vec2::ZERO, beamEnd,
        cocos2d::Color4F(0.0f, 1.0f, 0.0f, 0.9f));

    // Trailing glow (fading wedge behind the beam)
    for (int i = 1; i <= 15; i++) {
        float trailAngle = sweepAngle_ - i * 2.0f;
        if (trailAngle < 0) trailAngle += 360.0f;
        float trailRad = trailAngle * M_PI / 180.0f;
        float alpha = 0.4f * (1.0f - (float)i / 15.0f);

        cocos2d::Vec2 trailEnd(radius_ * std::sin(trailRad),
                                radius_ * std::cos(trailRad));

        sweepNode_->drawLine(
            cocos2d::Vec2::ZERO, trailEnd,
            cocos2d::Color4F(0.0f, 0.8f, 0.0f, alpha));
    }
}

void RadarDisplay::drawBlips()
{
    if (!trackManager_) return;

    auto tracks = trackManager_->getAllTracks();
    for (const auto& track : tracks) {
        if (!track.isAlive) continue;

        cocos2d::Vec2 pos = polarToScreen(track.range, track.azimuth);

        // Calculate blip brightness based on time since sweep passed
        float angleDiff = sweepAngle_ - track.azimuth;
        if (angleDiff < 0) angleDiff += 360.0f;
        float fadeRatio = angleDiff / 360.0f;
        float brightness = std::max(0.1f, 1.0f - fadeRatio);

        // Blip size based on radar cross section
        float blipSize = 3.0f;
        if (track.aircraftType == AircraftType::STRATEGIC_BOMBER ||
            track.aircraftType == AircraftType::CIVILIAN_AIRLINER) {
            blipSize = 5.0f;
        } else if (track.aircraftType == AircraftType::STEALTH_FIGHTER) {
            blipSize = 2.0f;
        }

        // Color based on classification
        cocos2d::Color4F blipColor;
        switch (track.classification) {
            case TrackClassification::HOSTILE:
                blipColor = cocos2d::Color4F(1.0f, 0.0f, 0.0f, brightness);
                break;
            case TrackClassification::FRIENDLY:
                blipColor = cocos2d::Color4F(0.0f, 0.5f, 1.0f, brightness);
                break;
            case TrackClassification::UNKNOWN:
                blipColor = cocos2d::Color4F(1.0f, 1.0f, 0.0f, brightness);
                break;
            case TrackClassification::PENDING:
                blipColor = cocos2d::Color4F(0.5f, 0.5f, 0.5f, brightness);
                break;
        }

        // Draw blip
        blipNode_->drawSolidCircle(pos, blipSize, 0, 8, blipColor);

        // Selected track highlight
        if (track.trackId == selectedTrackId_) {
            blipNode_->drawCircle(pos, blipSize + 4, 0, 16, false,
                cocos2d::Color4F(1.0f, 1.0f, 1.0f, 0.8f));
        }

        // Track ID label
        auto* idLabel = cocos2d::Label::createWithSystemFont(
            track.getTrackIdString(), "Courier", 8);
        idLabel->setPosition(pos + cocos2d::Vec2(8, 8));
        idLabel->setTextColor(cocos2d::Color4B(
            blipColor.r * 255, blipColor.g * 255, blipColor.b * 255, 200));
        blipNode_->addChild(idLabel);
    }
}

void RadarDisplay::drawBatteryPositions()
{
    if (!fireControl_) return;

    auto batteries = fireControl_->getAllBatteryData();
    for (const auto& battery : batteries) {
        cocos2d::Vec2 pos = polarToScreen(battery.position.range,
                                           battery.position.azimuth);

        // Battery symbol - small square for Patriot, triangle for Hawk
        cocos2d::Color4F color;
        if (battery.status == BatteryStatus::READY) {
            color = cocos2d::Color4F(0.0f, 1.0f, 0.0f, 0.8f);
        } else if (battery.status == BatteryStatus::ENGAGED) {
            color = cocos2d::Color4F(1.0f, 0.5f, 0.0f, 0.9f);
        } else if (battery.status == BatteryStatus::RELOADING) {
            color = cocos2d::Color4F(1.0f, 1.0f, 0.0f, 0.6f);
        } else {
            color = cocos2d::Color4F(0.5f, 0.5f, 0.5f, 0.5f);
        }

        if (battery.type == BatteryType::PATRIOT) {
            // Square symbol
            float s = 4.0f;
            cocos2d::Vec2 verts[] = {
                pos + cocos2d::Vec2(-s, -s), pos + cocos2d::Vec2(s, -s),
                pos + cocos2d::Vec2(s, s), pos + cocos2d::Vec2(-s, s)
            };
            blipNode_->drawPolygon(verts, 4, color, 1.0f, color);
        } else {
            // Triangle symbol
            float s = 5.0f;
            cocos2d::Vec2 verts[] = {
                pos + cocos2d::Vec2(0, s),
                pos + cocos2d::Vec2(-s, -s),
                pos + cocos2d::Vec2(s, -s)
            };
            blipNode_->drawPolygon(verts, 3, color, 1.0f, color);
        }
    }
}

void RadarDisplay::drawTerritoryZone()
{
    float territoryPixels = kmToPixels(GameConstants::TERRITORY_RADIUS_KM);
    overlayNode_->drawCircle(
        cocos2d::Vec2::ZERO, territoryPixels, 0, 32, false,
        cocos2d::Color4F(0.0f, 0.8f, 0.0f, 0.4f));
}

void RadarDisplay::drawSelectedTrackInfo()
{
    // Visual indicator drawn on the radar itself
    // Detailed info is shown in the HUD
}

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
#endif
