#include "FireControlSystem.h"

FireControlSystem::FireControlSystem() {}
FireControlSystem::~FireControlSystem() {}

void FireControlSystem::init()
{
    batteries_.clear();

    // Create 3 Patriot batteries in a triangle formation around the territory
    // Positioned at ~15 km from center for optimal coverage of the defense zone
    batteries_.push_back(std::make_unique<MissileBattery>(
        "PATRIOT-1", BatteryType::PATRIOT, 15.0f, 0.0f));     // North
    batteries_.push_back(std::make_unique<MissileBattery>(
        "PATRIOT-2", BatteryType::PATRIOT, 15.0f, 120.0f));   // SE
    batteries_.push_back(std::make_unique<MissileBattery>(
        "PATRIOT-3", BatteryType::PATRIOT, 15.0f, 240.0f));   // SW

    // Create 3 Hawk batteries between the Patriots (closer to center)
    // Positioned at ~8 km for short-range low-altitude defense
    batteries_.push_back(std::make_unique<MissileBattery>(
        "HAWK-1", BatteryType::HAWK, 8.0f, 60.0f));          // NE
    batteries_.push_back(std::make_unique<MissileBattery>(
        "HAWK-2", BatteryType::HAWK, 8.0f, 180.0f));         // South
    batteries_.push_back(std::make_unique<MissileBattery>(
        "HAWK-3", BatteryType::HAWK, 8.0f, 300.0f));         // NW
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
