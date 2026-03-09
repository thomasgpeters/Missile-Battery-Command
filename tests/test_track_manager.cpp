// ============================================================================
// Track Manager Unit Tests
// ============================================================================

#include "TestFramework.h"
#include "TrackManager.h"

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
void test_track_manager_empty()
{
    TrackManager tm;
    ASSERT_EQ(0, tm.getActiveTrackCount());
}

// ---------------------------------------------------------------------------
// Adding tracks
// ---------------------------------------------------------------------------
void test_track_add()
{
    TrackManager tm;
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                25000.0f, 600.0f, 225.0f, false);

    int id = tm.addTrack(&ac);
    ASSERT_EQ(1, id);
}

void test_track_add_increments_id()
{
    TrackManager tm;
    Aircraft ac1(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                 25000.0f, 600.0f, 225.0f, false);
    Aircraft ac2(AircraftType::RECON_DRONE, 90.0f, 120.0f,
                 40000.0f, 150.0f, 300.0f, false);

    int id1 = tm.addTrack(&ac1);
    int id2 = tm.addTrack(&ac2);
    ASSERT_EQ(2, id2);
}

void test_track_add_sets_aircraft_id()
{
    TrackManager tm;
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                25000.0f, 600.0f, 225.0f, false);

    tm.addTrack(&ac);
    ASSERT_EQ(1, ac.getTrackId());
}

void test_track_add_null_rejected()
{
    TrackManager tm;
    int id = tm.addTrack(nullptr);
    ASSERT_EQ(-1, id);
}

void test_track_count_after_add()
{
    TrackManager tm;
    Aircraft ac1(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                 25000.0f, 600.0f, 225.0f, false);
    Aircraft ac2(AircraftType::RECON_DRONE, 90.0f, 120.0f,
                 40000.0f, 150.0f, 300.0f, false);

    tm.addTrack(&ac1);
    tm.addTrack(&ac2);
    ASSERT_EQ(2, tm.getActiveTrackCount());
}

// ---------------------------------------------------------------------------
// Retrieving tracks
// ---------------------------------------------------------------------------
void test_track_get_by_id()
{
    TrackManager tm;
    Aircraft ac(AircraftType::TACTICAL_BOMBER, 60.0f, 200.0f,
                20000.0f, 400.0f, 20.0f, false);

    int id = tm.addTrack(&ac);
    TrackData* td = tm.getTrack(id);
    ASSERT_NOT_NULL(td);
}

void test_track_get_nonexistent()
{
    TrackManager tm;
    TrackData* td = tm.getTrack(999);
    ASSERT_NULL(td);
}

void test_track_get_aircraft_by_id()
{
    TrackManager tm;
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                25000.0f, 600.0f, 225.0f, false);

    int id = tm.addTrack(&ac);
    Aircraft* result = tm.getAircraftByTrackId(id);
    ASSERT_NOT_NULL(result);
}

void test_track_get_aircraft_is_same()
{
    TrackManager tm;
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                25000.0f, 600.0f, 225.0f, false);

    int id = tm.addTrack(&ac);
    Aircraft* result = tm.getAircraftByTrackId(id);
    ASSERT_TRUE(result == &ac);
}

void test_track_get_all()
{
    TrackManager tm;
    Aircraft ac1(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                 25000.0f, 600.0f, 225.0f, false);
    Aircraft ac2(AircraftType::RECON_DRONE, 90.0f, 120.0f,
                 40000.0f, 150.0f, 300.0f, false);

    tm.addTrack(&ac1);
    tm.addTrack(&ac2);
    auto tracks = tm.getAllTracks();
    ASSERT_EQ(2, (int)tracks.size());
}

// ---------------------------------------------------------------------------
// Removing tracks
// ---------------------------------------------------------------------------
void test_track_remove()
{
    TrackManager tm;
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                25000.0f, 600.0f, 225.0f, false);

    int id = tm.addTrack(&ac);
    tm.removeTrack(id);
    ASSERT_EQ(0, tm.getActiveTrackCount());
}

void test_track_remove_nonexistent_no_crash()
{
    TrackManager tm;
    tm.removeTrack(999);
    ASSERT_EQ(0, tm.getActiveTrackCount());
}

// ---------------------------------------------------------------------------
// Update removes dead aircraft
// ---------------------------------------------------------------------------
void test_track_update_removes_dead()
{
    TrackManager tm;
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                25000.0f, 600.0f, 225.0f, false);

    tm.addTrack(&ac);
    ac.destroy();
    tm.update(0.1f);
    ASSERT_EQ(0, tm.getActiveTrackCount());
}

// ---------------------------------------------------------------------------
// Hostile/Friendly counts
// ---------------------------------------------------------------------------
void test_track_hostile_count()
{
    TrackManager tm;
    tm.getIFFSystem().setErrorRate(0.0f);

    Aircraft ac1(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                 25000.0f, 600.0f, 225.0f, false);
    Aircraft ac2(AircraftType::CIVILIAN_AIRLINER, 70.0f, 90.0f,
                 35000.0f, 450.0f, 270.0f, true);

    tm.addTrack(&ac1);
    tm.addTrack(&ac2);

    // Let IFF complete
    tm.update(3.0f);

    ASSERT_EQ(1, tm.getHostileCount());
}

void test_track_friendly_count()
{
    TrackManager tm;
    tm.getIFFSystem().setErrorRate(0.0f);

    Aircraft ac1(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                 25000.0f, 600.0f, 225.0f, false);
    Aircraft ac2(AircraftType::CIVILIAN_AIRLINER, 70.0f, 90.0f,
                 35000.0f, 450.0f, 270.0f, true);

    tm.addTrack(&ac1);
    tm.addTrack(&ac2);

    tm.update(3.0f);
    ASSERT_EQ(1, tm.getFriendlyCount());
}

void test_track_hostile_ids()
{
    TrackManager tm;
    tm.getIFFSystem().setErrorRate(0.0f);

    Aircraft ac1(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                 25000.0f, 600.0f, 225.0f, false);

    tm.addTrack(&ac1);
    tm.update(3.0f);

    auto hostileIds = tm.getHostileTrackIds();
    ASSERT_EQ(1, (int)hostileIds.size());
}

// ---------------------------------------------------------------------------
// Reset
// ---------------------------------------------------------------------------
void test_track_reset()
{
    TrackManager tm;
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                25000.0f, 600.0f, 225.0f, false);

    tm.addTrack(&ac);
    tm.reset();
    ASSERT_EQ(0, tm.getActiveTrackCount());
}

void test_track_reset_resets_ids()
{
    TrackManager tm;
    Aircraft ac1(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                 25000.0f, 600.0f, 225.0f, false);
    Aircraft ac2(AircraftType::RECON_DRONE, 90.0f, 120.0f,
                 40000.0f, 150.0f, 300.0f, false);

    tm.addTrack(&ac1);
    tm.reset();
    int id = tm.addTrack(&ac2);
    ASSERT_EQ(1, id);  // Should reset back to 1
}

// ============================================================================
void run_track_manager_tests()
{
    TEST_SUITE("Track Manager");

    test_track_manager_empty();
    test_track_add();
    test_track_add_increments_id();
    test_track_add_sets_aircraft_id();
    test_track_add_null_rejected();
    test_track_count_after_add();
    test_track_get_by_id();
    test_track_get_nonexistent();
    test_track_get_aircraft_by_id();
    test_track_get_aircraft_is_same();
    test_track_get_all();
    test_track_remove();
    test_track_remove_nonexistent_no_crash();
    test_track_update_removes_dead();
    test_track_hostile_count();
    test_track_friendly_count();
    test_track_hostile_ids();
    test_track_reset();
    test_track_reset_resets_ids();

    TEST_SUITE_END();
}
