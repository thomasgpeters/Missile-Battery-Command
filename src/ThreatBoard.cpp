#include "ThreatBoard.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

ThreatBoard::ThreatBoard() {}
ThreatBoard::~ThreatBoard() {}

void ThreatBoard::update(const TrackManager& trackManager)
{
    // Get all current tracks
    auto allTracks = const_cast<TrackManager&>(trackManager).getAllTracks();

    // Build threat entries for non-friendly tracks
    std::vector<ThreatEntry> candidates;

    for (const auto& track : allTracks) {
        // Skip friendly and dead tracks
        if (track.classification == TrackClassification::FRIENDLY) continue;
        if (!track.isAlive) continue;

        ThreatEntry entry;
        entry.trackId = track.trackId;
        entry.trackIdString = track.getTrackIdString();
        entry.classification = track.getClassificationString();
        entry.range = track.range;
        entry.azimuth = track.azimuth;
        entry.altitude = track.altitude;
        entry.speed = track.speed;
        entry.heading = track.heading;
        entry.inbound = isInbound(track.azimuth, track.heading);
        entry.closingRate = calculateClosingRate(track.range, track.azimuth,
                                                  track.speed, track.heading);
        entry.threatScore = calculateThreatScore(track);
        entry.timeToTerritory = estimateTimeToTerritory(track.range, entry.closingRate);

        candidates.push_back(entry);
    }

    // Sort by threat score (highest first)
    std::sort(candidates.begin(), candidates.end(),
        [](const ThreatEntry& a, const ThreatEntry& b) {
            return a.threatScore > b.threatScore;
        });

    // Keep top N
    topThreats_.clear();
    int count = std::min((int)candidates.size(), MAX_DISPLAYED_THREATS);
    for (int i = 0; i < count; i++) {
        topThreats_.push_back(candidates[i]);
    }
}

const ThreatEntry* ThreatBoard::getThreat(int rank) const
{
    if (rank < 0 || rank >= (int)topThreats_.size()) return nullptr;
    return &topThreats_[rank];
}

bool ThreatBoard::isOnBoard(int trackId) const
{
    for (const auto& t : topThreats_) {
        if (t.trackId == trackId) return true;
    }
    return false;
}

const ThreatEntry* ThreatBoard::getThreatByTrackId(int trackId) const
{
    for (const auto& t : topThreats_) {
        if (t.trackId == trackId) return &t;
    }
    return nullptr;
}

float ThreatBoard::calculateThreatScore(const TrackData& track)
{
    float score = 0.0f;

    // Proximity — closer targets are higher threat
    // Invert range: max radar range minus current range
    float proximityScore = (GameConstants::RADAR_MAX_RANGE_KM - track.range)
                           / GameConstants::RADAR_MAX_RANGE_KM * 100.0f;
    score += proximityScore * 3.0f;  // Proximity is weighted heavily

    // Speed — faster targets need faster response
    float speedScore = track.speed / 1000.0f * 50.0f;  // Normalized against ~1000 kts
    score += speedScore;

    // Inbound bonus — heading toward radar center is far more threatening
    if (isInbound(track.azimuth, track.heading)) {
        score += 150.0f;
    }

    // Classification bonus
    if (track.classification == TrackClassification::HOSTILE) {
        score += 100.0f;  // Confirmed hostile gets priority
    } else if (track.classification == TrackClassification::UNKNOWN) {
        score += 50.0f;   // Unknown is still concerning
    }

    // Altitude factor — low altitude penetrators are harder to detect and engage
    if (track.altitude < 5000.0f) {
        score += 30.0f;  // Low-altitude bonus
    }

    // Inside territory radius — critical threat
    if (track.range <= GameConstants::TERRITORY_RADIUS_KM) {
        score += 200.0f;
    }

    return score;
}

bool ThreatBoard::isInbound(float azimuth, float heading)
{
    // An aircraft is inbound if its heading is roughly opposite to its azimuth
    // from radar center. Azimuth is where the aircraft IS (from center),
    // heading is where it's GOING.
    //
    // If aircraft is at azimuth 90 (due east), an inbound heading would be
    // roughly 270 (heading west, toward center).
    float inboundHeading = std::fmod(azimuth + 180.0f, 360.0f);
    float diff = std::fmod(std::abs(heading - inboundHeading) + 360.0f, 360.0f);
    if (diff > 180.0f) diff = 360.0f - diff;

    // Within 60 degrees of direct inbound heading
    return diff <= 60.0f;
}

float ThreatBoard::calculateClosingRate(float range, float azimuth,
                                          float speed, float heading)
{
    // Convert speed from knots to km/s
    float speedKmPerSec = speed * GameConstants::NM_TO_KM / 3600.0f;

    // Calculate radial component of velocity (positive = closing)
    float inboundHeading = std::fmod(azimuth + 180.0f, 360.0f);
    float diff = (heading - inboundHeading) * M_PI / 180.0f;
    float radialComponent = speedKmPerSec * std::cos(diff);

    return radialComponent;
}

float ThreatBoard::estimateTimeToTerritory(float range, float closingRate)
{
    if (closingRate <= 0.001f) return 9999.0f;  // Not closing

    float distToTerritory = range - GameConstants::TERRITORY_RADIUS_KM;
    if (distToTerritory <= 0.0f) return 0.0f;   // Already inside

    return distToTerritory / closingRate;
}

std::string ThreatEntry::formatLine() const
{
    char buf[256];
    snprintf(buf, sizeof(buf),
        "%s %s RNG:%.0fkm AZ:%03d SPD:%.0fkts %s ETA:%s",
        trackIdString.c_str(),
        classification.c_str(),
        range, (int)azimuth, speed,
        inbound ? "INBOUND" : "OUTBND ",
        timeToTerritory < 9000.0f ?
            (std::to_string((int)timeToTerritory) + "s").c_str() : "N/A");
    return std::string(buf);
}

std::string ThreatEntry::formatCard() const
{
    char buf[512];
    snprintf(buf, sizeof(buf),
        "  TRACK: %s  [%s]\n"
        "  RNG:   %.1f km (%.1f NM)\n"
        "  AZM:   %03d deg\n"
        "  ALT:   %.0f ft\n"
        "  SPD:   %.0f kts\n"
        "  HDG:   %03d deg\n"
        "  DIR:   %s\n"
        "  CLOSE: %.2f km/s\n"
        "  ETA:   %s\n"
        "  SCORE: %.0f",
        trackIdString.c_str(), classification.c_str(),
        range, range * GameConstants::KM_TO_NM,
        (int)azimuth,
        altitude,
        speed,
        (int)heading,
        inbound ? "INBOUND" : "OUTBOUND",
        closingRate,
        timeToTerritory < 9000.0f ?
            (std::to_string((int)timeToTerritory) + "s").c_str() : "N/A",
        threatScore);
    return std::string(buf);
}

std::string ThreatBoard::formatBoard() const
{
    std::string board;
    board += "=== THREAT BOARD (SCOPE 2) ===\n";
    board += "  TOP " + std::to_string(MAX_DISPLAYED_THREATS) + " THREATS\n";
    board += "------------------------------\n";

    if (topThreats_.empty()) {
        board += "  NO THREATS DETECTED\n";
        return board;
    }

    for (int i = 0; i < (int)topThreats_.size(); i++) {
        char header[32];
        snprintf(header, sizeof(header), "#%d ", i + 1);
        board += header;
        board += topThreats_[i].formatLine() + "\n";
    }

    return board;
}
