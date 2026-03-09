#ifndef __TRACK_MANAGER_H__
#define __TRACK_MANAGER_H__

#include "Aircraft.h"
#include "IFFSystem.h"
#include <vector>
#include <memory>

// ============================================================================
// Track Manager - assigns and manages track IDs for all radar contacts
// ============================================================================

class TrackManager {
public:
    TrackManager();
    ~TrackManager();

    // Add a new aircraft and assign a track ID
    int addTrack(Aircraft* aircraft);

    // Remove a track (aircraft destroyed or left radar range)
    void removeTrack(int trackId);

    // Update all tracks
    void update(float dt);

    // Get track data for display
    std::vector<TrackData> getAllTracks() const;
    TrackData* getTrack(int trackId);

    // Get aircraft by track ID
    Aircraft* getAircraftByTrackId(int trackId) const;

    // Get all hostile tracks for engagement
    std::vector<int> getHostileTrackIds() const;

    // Track count
    int getActiveTrackCount() const;
    int getHostileCount() const;
    int getFriendlyCount() const;

    // Clear all tracks
    void reset();

    // Get the IFF system for configuration
    IFFSystem& getIFFSystem() { return iffSystem_; }

private:
    struct TrackEntry {
        int trackId;
        Aircraft* aircraft;    // Non-owning pointer
        TrackData cachedData;
    };

    std::vector<TrackEntry> tracks_;
    int nextTrackId_;
    IFFSystem iffSystem_;

    void updateTrackData(TrackEntry& entry);
};

#endif // __TRACK_MANAGER_H__
