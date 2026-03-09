#include "TrackManager.h"
#include <algorithm>

TrackManager::TrackManager()
    : nextTrackId_(1)
{
}

TrackManager::~TrackManager() {}

int TrackManager::addTrack(Aircraft* aircraft)
{
    if (!aircraft) return -1;

    int id = nextTrackId_++;
    aircraft->setTrackId(id);

    TrackEntry entry;
    entry.trackId = id;
    entry.aircraft = aircraft;
    entry.cachedData = aircraft->getTrackData();

    tracks_.push_back(entry);

    // Begin IFF interrogation
    iffSystem_.interrogate(aircraft);

    return id;
}

void TrackManager::removeTrack(int trackId)
{
    tracks_.erase(
        std::remove_if(tracks_.begin(), tracks_.end(),
            [trackId](const TrackEntry& e) { return e.trackId == trackId; }),
        tracks_.end());
}

void TrackManager::update(float dt)
{
    // Update IFF system
    iffSystem_.update(dt);

    // Update all track data and remove dead/out-of-range tracks
    auto it = tracks_.begin();
    while (it != tracks_.end()) {
        if (!it->aircraft || !it->aircraft->isAlive() ||
            it->aircraft->getRange() > GameConstants::RADAR_MAX_RANGE_KM) {
            it = tracks_.erase(it);
        } else {
            updateTrackData(*it);
            ++it;
        }
    }
}

void TrackManager::updateTrackData(TrackEntry& entry)
{
    if (entry.aircraft) {
        entry.cachedData = entry.aircraft->getTrackData();
    }
}

std::vector<TrackData> TrackManager::getAllTracks() const
{
    std::vector<TrackData> result;
    result.reserve(tracks_.size());
    for (const auto& entry : tracks_) {
        result.push_back(entry.cachedData);
    }
    return result;
}

TrackData* TrackManager::getTrack(int trackId)
{
    for (auto& entry : tracks_) {
        if (entry.trackId == trackId) {
            return &entry.cachedData;
        }
    }
    return nullptr;
}

Aircraft* TrackManager::getAircraftByTrackId(int trackId) const
{
    for (const auto& entry : tracks_) {
        if (entry.trackId == trackId) {
            return entry.aircraft;
        }
    }
    return nullptr;
}

std::vector<int> TrackManager::getHostileTrackIds() const
{
    std::vector<int> result;
    for (const auto& entry : tracks_) {
        if (entry.cachedData.classification == TrackClassification::HOSTILE) {
            result.push_back(entry.trackId);
        }
    }
    return result;
}

int TrackManager::getActiveTrackCount() const
{
    return static_cast<int>(tracks_.size());
}

int TrackManager::getHostileCount() const
{
    int count = 0;
    for (const auto& entry : tracks_) {
        if (entry.cachedData.classification == TrackClassification::HOSTILE) {
            ++count;
        }
    }
    return count;
}

int TrackManager::getFriendlyCount() const
{
    int count = 0;
    for (const auto& entry : tracks_) {
        if (entry.cachedData.classification == TrackClassification::FRIENDLY) {
            ++count;
        }
    }
    return count;
}

void TrackManager::reset()
{
    tracks_.clear();
    nextTrackId_ = 1;
}
