#ifndef __BATTALION_HQ_H__
#define __BATTALION_HQ_H__

#include "GameTypes.h"
#include <string>

// ============================================================================
// Battalion HQ — Mobile AN/TSQ-73 Missile Minder Battery
//
// The AN/TSQ-73 Missile Minder operates in a mobile field configuration.
// The battalion HQ coordinates all fire units from a command vehicle that
// can relocate when enemy intelligence discovers the position.
//
// The mobile HQ battery includes:
//   - AN/TSQ-73 Missile Minder console (2 missile tracking scopes)
//   - AN/TPS-43E surveillance radar
//   - Communications suite (encrypted data links to all fire units)
//   - Generator and power distribution
//   - Command vehicle (M577 or equivalent)
//
// When the HQ relocates:
//   - All batteries lose centralized fire control for the duration
//   - Batteries can still operate independently (degraded mode)
//   - Radar goes offline during teardown/setup
//   - Relocation time: ~90 seconds (game time)
// ============================================================================

enum class HQStatus {
    OPERATIONAL,        // Fully operational, all systems active
    RELOCATING,         // Tearing down and moving to new position
    SETTING_UP,         // At new position, bringing systems online
    DEGRADED            // Partial capability (e.g., radar up but comms limited)
};

struct HQData {
    std::string designation;       // "MINDER-HQ"
    HQStatus status;
    PolarCoord position;           // Current position relative to defense center
    PolarCoord relocateDestination;
    bool isRelocating;
    float relocateTimeRemaining;   // Seconds until relocation complete
    float totalRelocateTime;       // Total time for current relocation
    int personnelCount;            // Staff operating the HQ
    bool radarOnline;              // AN/TPS-43E operational
    bool commsOnline;              // Data links to fire units active
    std::string statusString;      // Human-readable status

    std::string format() const;
};

class BattalionHQ {
public:
    BattalionHQ();
    ~BattalionHQ();

    // Initialize at starting position
    void init(float posRange, float posAzimuth);

    // Update state (relocation countdown)
    void update(float dt);

    // Relocate the HQ to a new position
    // Returns false if already relocating or setting up
    bool relocate(float newRange, float newAzimuth);

    // Cancel a relocation in progress (emergency halt)
    void cancelRelocation();

    // Status queries
    HQStatus getStatus() const { return status_; }
    bool isOperational() const { return status_ == HQStatus::OPERATIONAL; }
    bool isRelocating() const { return isRelocating_; }
    bool isRadarOnline() const { return radarOnline_; }
    bool isCommsOnline() const { return commsOnline_; }

    // Position
    PolarCoord getPosition() const { return position_; }
    float getRelocateTimeRemaining() const { return relocateTimeRemaining_; }

    // Get formatted data for display
    HQData getData() const;

    // Get status string
    std::string getStatusString() const;

    // Relocation timing (game seconds representing real-world minutes)
    // Actual AN/TSQ-73 battalion displacement: ~45 min teardown (power down,
    // disconnect cables, lower AN/GSS-1 antenna, secure L-112 racks), convoy
    // travel, ~45 min setup (erect antenna, lay cables, boot processors).
    // Game uses 1 game-second = 1 real-minute time compression for relocation.
    static constexpr float TEARDOWN_TIME = 45.0f;   // 45 min — power down, cables, antenna
    static constexpr float MOVE_TIME = 30.0f;        // 30 min — convoy travel (variable)
    static constexpr float SETUP_TIME = 45.0f;       // 45 min — erect, cable, boot L-112s
    static constexpr float TOTAL_RELOCATE_TIME = TEARDOWN_TIME + MOVE_TIME + SETUP_TIME;

    // Personnel
    static constexpr int HQ_PERSONNEL = 35;          // AN/TSQ-73 operating staff

private:
    std::string designation_;
    HQStatus status_;
    PolarCoord position_;
    PolarCoord relocateDestination_;
    bool isRelocating_;
    float relocateTimeRemaining_;
    bool radarOnline_;
    bool commsOnline_;
};

#endif // __BATTALION_HQ_H__
