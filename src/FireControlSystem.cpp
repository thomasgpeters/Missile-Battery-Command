#include "FireControlSystem.h"

FireControlSystem::FireControlSystem() {}
FireControlSystem::~FireControlSystem() {}

void FireControlSystem::init()
{
    batteries_.clear();

    // Realistic battalion spread: ~75 km across (batteries ~25 km apart)
    // Each battery has its own organic radar and operates autonomously
    // when HQ is offline. Positions are relative to the defended asset.

    // 3 Patriot batteries in triangle formation (~35 km from center)
    // Long-range / high-altitude umbrella — AN/MPQ-53 phased array
    // Each Patriot covers 160 km range, so at 35 km out they overlap
    // heavily and provide layered coverage over the entire zone.
    batteries_.push_back(std::make_unique<MissileBattery>(
        "PATRIOT-1", BatteryType::PATRIOT, 35.0f, 0.0f));     // North
    batteries_.push_back(std::make_unique<MissileBattery>(
        "PATRIOT-2", BatteryType::PATRIOT, 35.0f, 120.0f));   // SE
    batteries_.push_back(std::make_unique<MissileBattery>(
        "PATRIOT-3", BatteryType::PATRIOT, 35.0f, 240.0f));   // SW

    // 3 Hawk batteries between the Patriots (~20 km from center)
    // Medium-range / low-altitude defense — AN/MPQ-46 HPI radar
    // Covers the low-altitude corridor that Patriot can't reach,
    // and fills gaps between the Patriot positions.
    batteries_.push_back(std::make_unique<MissileBattery>(
        "HAWK-1", BatteryType::HAWK, 20.0f, 60.0f));          // NE
    batteries_.push_back(std::make_unique<MissileBattery>(
        "HAWK-2", BatteryType::HAWK, 20.0f, 180.0f));         // South
    batteries_.push_back(std::make_unique<MissileBattery>(
        "HAWK-3", BatteryType::HAWK, 20.0f, 300.0f));         // NW

    // 3 Javelin MANPADS platoons (~8 km from center, inner ring)
    // Last line of defense — CLU IR/FLIR seeker, no radar signature
    // Close-in protection for the asset itself.
    batteries_.push_back(std::make_unique<MissileBattery>(
        "JAVELIN-1", BatteryType::JAVELIN, 8.0f, 30.0f));     // NNE
    batteries_.push_back(std::make_unique<MissileBattery>(
        "JAVELIN-2", BatteryType::JAVELIN, 8.0f, 150.0f));    // SSE
    batteries_.push_back(std::make_unique<MissileBattery>(
        "JAVELIN-3", BatteryType::JAVELIN, 8.0f, 270.0f));    // West
}

void FireControlSystem::update(float dt)
{
    for (auto& battery : batteries_) {
        battery->update(dt);
    }
    checkEngagementResults();
}

bool FireControlSystem::assignTarget(const std::string& batteryDesignation,
                                      int trackId, TrackManager& trackMgr)
{
    auto* battery = getBattery(batteryDesignation);
    if (!battery) return false;

    auto* aircraft = trackMgr.getAircraftByTrackId(trackId);
    if (!aircraft) return false;

    if (!battery->canEngage(aircraft)) return false;

    return true;  // Target assigned, awaiting authorization
}

bool FireControlSystem::authorizeEngagement(const std::string& batteryDesignation)
{
    // In a full implementation, this would fire the assigned target
    // For now, engagement happens through direct battery->engage() calls
    auto* battery = getBattery(batteryDesignation);
    return battery && battery->isReady();
}

void FireControlSystem::abortEngagement(const std::string& batteryDesignation)
{
    auto* battery = getBattery(batteryDesignation);
    if (battery) {
        battery->abortEngagement();
    }
}

MissileBattery* FireControlSystem::getBattery(const std::string& designation)
{
    for (auto& battery : batteries_) {
        if (battery->getDesignation() == designation) {
            return battery.get();
        }
    }
    return nullptr;
}

std::vector<BatteryData> FireControlSystem::getAllBatteryData() const
{
    std::vector<BatteryData> result;
    result.reserve(batteries_.size());
    for (const auto& battery : batteries_) {
        result.push_back(battery->getData());
    }
    return result;
}

std::vector<std::string> FireControlSystem::getAvailableBatteries(
    int trackId, TrackManager& trackMgr) const
{
    std::vector<std::string> result;
    auto* aircraft = trackMgr.getAircraftByTrackId(trackId);
    if (!aircraft) return result;

    for (const auto& battery : batteries_) {
        if (battery->canEngage(aircraft)) {
            result.push_back(battery->getDesignation());
        }
    }
    return result;
}

std::vector<EngagementRecord> FireControlSystem::getRecentResults()
{
    auto results = recentResults_;
    recentResults_.clear();
    return results;
}

void FireControlSystem::checkEngagementResults()
{
    for (auto& battery : batteries_) {
        if (battery->hasEngagementResult()) {
            EngagementRecord record;
            record.batteryDesignation = battery->getDesignation();
            record.result = battery->getLastResult();
            record.trackId = battery->getAssignedTrackId();
            recentResults_.push_back(record);
            battery->clearEngagementResult();
        }
    }
}

void FireControlSystem::reset()
{
    batteries_.clear();
    recentResults_.clear();
    init();
}
