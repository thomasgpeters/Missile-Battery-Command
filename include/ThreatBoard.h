#ifndef __THREAT_BOARD_H__
#define __THREAT_BOARD_H__

#include "GameTypes.h"
#include "TrackManager.h"
#include <vector>
#include <string>

// ============================================================================
// Threat Assessment Entry — one row on the threat board
//
// Represents the second missile tracking scope on the AN/TSQ-73 console.
// Displays the 5 highest-priority threats based on composite scoring:
//   proximity, direction of travel, speed, and inbound determination.
// ============================================================================

struct ThreatEntry {
    int trackId;
    std::string trackIdString;      // "TK-001"
    std::string classification;     // "HOSTILE", "UNKNOWN", etc.
    float range;                    // km from radar
    float azimuth;                  // degrees
    float altitude;                 // feet MSL
    float speed;                    // knots
    float heading;                  // degrees
    bool inbound;                   // True if heading toward radar center
    float closingRate;              // km/s — positive means closing
    float threatScore;              // Composite threat score (higher = more urgent)
    float timeToTerritory;          // Estimated seconds until territory breach

    // Format a single threat line for display
    std::string formatLine() const;

    // Format detailed threat card
    std::string formatCard() const;
};

// ============================================================================
// ThreatBoard — the second scope heads-up display
//
// The AN/TSQ-73 Missile Minder provides 2 missile tracking scopes:
//   Scope 1: Primary PPI radar display (RadarDisplay)
//   Scope 2: Threat assessment board (ThreatBoard)
//
// The ThreatBoard continuously evaluates all contacts and displays the
// top 5 highest threats, updated each radar sweep. The operator uses this
// to prioritize engagements when multiple hostiles are inbound.
// ============================================================================

class ThreatBoard {
public:
    ThreatBoard();
    ~ThreatBoard();

    static constexpr int MAX_DISPLAYED_THREATS = 5;

    // Update threat assessments from current track data
    void update(const TrackManager& trackManager);

    // Get the top threats (up to MAX_DISPLAYED_THREATS)
    const std::vector<ThreatEntry>& getTopThreats() const { return topThreats_; }

    // Get threat count
    int getThreatCount() const { return (int)topThreats_.size(); }

    // Get a specific threat entry by rank (0 = highest threat)
    const ThreatEntry* getThreat(int rank) const;

    // Format the entire threat board for display
    std::string formatBoard() const;

    // Check if a specific track is on the threat board
    bool isOnBoard(int trackId) const;

    // Get the threat entry for a specific track (nullptr if not on board)
    const ThreatEntry* getThreatByTrackId(int trackId) const;

private:
    std::vector<ThreatEntry> topThreats_;

    // Calculate composite threat score for an aircraft
    static float calculateThreatScore(const TrackData& track);

    // Determine if aircraft is heading toward radar center (inbound)
    static bool isInbound(float azimuth, float heading);

    // Calculate closing rate in km/s
    static float calculateClosingRate(float range, float azimuth,
                                       float speed, float heading);

    // Estimate time to territory breach
    static float estimateTimeToTerritory(float range, float closingRate);
};

#endif // __THREAT_BOARD_H__
