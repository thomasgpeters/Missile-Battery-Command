#ifndef __FIRE_CONTROL_SYSTEM_H__
#define __FIRE_CONTROL_SYSTEM_H__

#include "MissileBattery.h"
#include "TrackManager.h"
#include <vector>
#include <memory>

// ============================================================================
// Fire Control System - manages all batteries and engagement authorization
// ============================================================================

class FireControlSystem {
public:
    FireControlSystem();
    ~FireControlSystem();

    // Non-copyable (contains unique_ptr), but movable
    FireControlSystem(const FireControlSystem&) = delete;
    FireControlSystem& operator=(const FireControlSystem&) = delete;
    FireControlSystem(FireControlSystem&&) = default;
    FireControlSystem& operator=(FireControlSystem&&) = default;

    // Initialize batteries at their positions
    void init();

    // Update all battery states
    void update(float dt);

    // Engagement commands
    bool assignTarget(const std::string& batteryDesignation, int trackId,
                      TrackManager& trackMgr);
    bool authorizeEngagement(const std::string& batteryDesignation);
    void abortEngagement(const std::string& batteryDesignation);

    // Get battery info
    MissileBattery* getBattery(const std::string& designation);
    std::vector<BatteryData> getAllBatteryData() const;
    std::vector<std::string> getAvailableBatteries(int trackId,
                                                    TrackManager& trackMgr) const;

    // Engagement results
    std::vector<EngagementRecord> getRecentResults();

    // Get all batteries
    const std::vector<std::unique_ptr<MissileBattery>>& getBatteries() const {
        return batteries_;
    }

    // Reset all batteries
    void reset();

private:
    std::vector<std::unique_ptr<MissileBattery>> batteries_;
    std::vector<EngagementRecord> recentResults_;

    void checkEngagementResults();
};

#endif // __FIRE_CONTROL_SYSTEM_H__
