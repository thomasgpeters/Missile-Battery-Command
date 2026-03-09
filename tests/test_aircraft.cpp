// ============================================================================
// Aircraft Unit Tests
// ============================================================================

#include "TestFramework.h"
#include "Aircraft.h"

// ---------------------------------------------------------------------------
// Construction and initial state
// ---------------------------------------------------------------------------
void test_aircraft_construction()
{
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                25000.0f, 600.0f, 225.0f, false);

    ASSERT_TRUE(ac.getType() == AircraftType::FIGHTER_ATTACK);
}

void test_aircraft_initial_position()
{
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                25000.0f, 600.0f, 225.0f, false);

    ASSERT_NEAR(80.0f, ac.getRange(), 0.01f);
}

void test_aircraft_initial_azimuth()
{
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                25000.0f, 600.0f, 225.0f, false);

    ASSERT_NEAR(45.0f, ac.getAzimuth(), 0.01f);
}

void test_aircraft_initial_altitude()
{
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                25000.0f, 600.0f, 225.0f, false);

    ASSERT_NEAR(25000.0f, ac.getAltitude(), 0.01f);
}

void test_aircraft_initial_alive()
{
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                25000.0f, 600.0f, 225.0f, false);

    ASSERT_TRUE(ac.isAlive());
}

void test_aircraft_initial_iff_pending()
{
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                25000.0f, 600.0f, 225.0f, false);

    ASSERT_TRUE(ac.getIFFStatus() == IFFStatus::PENDING);
}

void test_aircraft_initial_track_id()
{
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                25000.0f, 600.0f, 225.0f, false);

    ASSERT_EQ(-1, ac.getTrackId());
}

void test_aircraft_friendly_flag()
{
    Aircraft friendly(AircraftType::CIVILIAN_AIRLINER, 80.0f, 0.0f,
                      35000.0f, 450.0f, 180.0f, true);
    Aircraft hostile(AircraftType::FIGHTER_ATTACK, 80.0f, 0.0f,
                     25000.0f, 600.0f, 180.0f, false);

    ASSERT_TRUE(friendly.isFriendly());
}

void test_aircraft_hostile_flag()
{
    Aircraft hostile(AircraftType::FIGHTER_ATTACK, 80.0f, 0.0f,
                     25000.0f, 600.0f, 180.0f, false);

    ASSERT_FALSE(hostile.isFriendly());
}

// ---------------------------------------------------------------------------
// Movement
// ---------------------------------------------------------------------------
void test_aircraft_moves_on_update()
{
    // Aircraft at 90km heading south (180 degrees), should get closer to center
    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 90.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);

    float initialRange = ac.getRange();
    ac.update(10.0f);  // 10 seconds

    ASSERT_LT(ac.getRange(), initialRange);
}

void test_aircraft_range_decreases_inbound()
{
    // Aircraft at 50km due east, heading west (270 degrees toward center)
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 50.0f, 90.0f,
                25000.0f, 600.0f, 270.0f, false);

    float initialRange = ac.getRange();
    ac.update(5.0f);

    ASSERT_LT(ac.getRange(), initialRange);
}

void test_aircraft_no_update_when_dead()
{
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 50.0f, 90.0f,
                25000.0f, 600.0f, 270.0f, false);

    ac.destroy();
    float range = ac.getRange();
    ac.update(10.0f);

    ASSERT_NEAR(range, ac.getRange(), 0.001f);
}

// ---------------------------------------------------------------------------
// Destruction
// ---------------------------------------------------------------------------
void test_aircraft_destroy()
{
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                25000.0f, 600.0f, 225.0f, false);

    ac.destroy();
    ASSERT_FALSE(ac.isAlive());
}

// ---------------------------------------------------------------------------
// Territory penetration
// ---------------------------------------------------------------------------
void test_aircraft_territory_penetration()
{
    // Place hostile right at territory boundary
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 11.0f, 0.0f,
                25000.0f, 600.0f, 180.0f, false);

    ASSERT_FALSE(ac.hasReachedTerritory());
}

void test_aircraft_territory_penetration_inside()
{
    // Place hostile inside territory
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 9.0f, 0.0f,
                25000.0f, 600.0f, 180.0f, false);

    ac.update(0.0f);  // Trigger territory check
    ASSERT_TRUE(ac.hasReachedTerritory());
}

void test_friendly_no_territory_penetration()
{
    // Friendly inside territory should NOT trigger penetration
    Aircraft ac(AircraftType::CIVILIAN_AIRLINER, 5.0f, 0.0f,
                35000.0f, 450.0f, 180.0f, true);

    ac.update(0.0f);
    ASSERT_FALSE(ac.hasReachedTerritory());
}

// ---------------------------------------------------------------------------
// Radar Cross Section
// ---------------------------------------------------------------------------
void test_rcs_strategic_bomber()
{
    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 80.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);

    ASSERT_NEAR(100.0f, ac.getRadarCrossSection(), 0.01f);
}

void test_rcs_stealth_fighter()
{
    Aircraft ac(AircraftType::STEALTH_FIGHTER, 80.0f, 0.0f,
                30000.0f, 700.0f, 180.0f, false);

    ASSERT_NEAR(0.1f, ac.getRadarCrossSection(), 0.01f);
}

void test_rcs_stealth_smaller_than_bomber()
{
    Aircraft stealth(AircraftType::STEALTH_FIGHTER, 80.0f, 0.0f,
                     30000.0f, 700.0f, 180.0f, false);
    Aircraft bomber(AircraftType::STRATEGIC_BOMBER, 80.0f, 0.0f,
                    35000.0f, 500.0f, 180.0f, false);

    ASSERT_LT(stealth.getRadarCrossSection(), bomber.getRadarCrossSection());
}

// ---------------------------------------------------------------------------
// Track data
// ---------------------------------------------------------------------------
void test_track_data_populated()
{
    Aircraft ac(AircraftType::TACTICAL_BOMBER, 60.0f, 120.0f,
                20000.0f, 400.0f, 300.0f, false);
    ac.setTrackId(5);

    TrackData td = ac.getTrackData();
    ASSERT_EQ(5, td.trackId);
}

void test_track_data_altitude()
{
    Aircraft ac(AircraftType::TACTICAL_BOMBER, 60.0f, 120.0f,
                20000.0f, 400.0f, 300.0f, false);

    TrackData td = ac.getTrackData();
    ASSERT_NEAR(20000.0f, td.altitude, 0.01f);
}

void test_track_data_iff_maps_to_classification()
{
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 60.0f, 120.0f,
                20000.0f, 500.0f, 300.0f, false);

    ac.setIFFStatus(IFFStatus::HOSTILE);
    TrackData td = ac.getTrackData();
    ASSERT_TRUE(td.classification == TrackClassification::HOSTILE);
}

// ---------------------------------------------------------------------------
// IFF status
// ---------------------------------------------------------------------------
void test_iff_set_hostile()
{
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 0.0f,
                25000.0f, 600.0f, 180.0f, false);

    ac.setIFFStatus(IFFStatus::HOSTILE);
    ASSERT_TRUE(ac.getIFFStatus() == IFFStatus::HOSTILE);
}

void test_iff_set_friendly()
{
    Aircraft ac(AircraftType::CIVILIAN_AIRLINER, 80.0f, 0.0f,
                35000.0f, 450.0f, 180.0f, true);

    ac.setIFFStatus(IFFStatus::FRIENDLY);
    ASSERT_TRUE(ac.getIFFStatus() == IFFStatus::FRIENDLY);
}

// ---------------------------------------------------------------------------
// Threat scoring
// ---------------------------------------------------------------------------
void test_threat_score_hostile()
{
    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 80.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);

    ASSERT_GT(ac.getThreatScore(), 0);
}

void test_threat_score_friendly_zero()
{
    Aircraft ac(AircraftType::CIVILIAN_AIRLINER, 80.0f, 0.0f,
                35000.0f, 450.0f, 180.0f, true);

    ASSERT_EQ(0, ac.getThreatScore());
}

void test_threat_score_closer_is_higher()
{
    Aircraft far(AircraftType::FIGHTER_ATTACK, 80.0f, 0.0f,
                 25000.0f, 600.0f, 180.0f, false);
    Aircraft close(AircraftType::FIGHTER_ATTACK, 20.0f, 0.0f,
                   25000.0f, 600.0f, 180.0f, false);

    ASSERT_GT(close.getThreatScore(), far.getThreatScore());
}

// ---------------------------------------------------------------------------
// Type names
// ---------------------------------------------------------------------------
void test_type_name_bomber()
{
    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 80.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);

    ASSERT_STR_EQ("STRAT BOMBER", ac.getTypeName());
}

void test_type_name_stealth()
{
    Aircraft ac(AircraftType::STEALTH_FIGHTER, 80.0f, 0.0f,
                30000.0f, 700.0f, 180.0f, false);

    ASSERT_STR_EQ("STEALTH FTR", ac.getTypeName());
}

// ---------------------------------------------------------------------------
// Sweep timer
// ---------------------------------------------------------------------------
void test_sweep_timer_initial()
{
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                25000.0f, 600.0f, 225.0f, false);

    ASSERT_NEAR(0.0f, ac.getTimeSinceLastSweep(), 0.001f);
}

void test_sweep_timer_increments()
{
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                25000.0f, 600.0f, 225.0f, false);

    ac.update(3.0f);
    ASSERT_NEAR(3.0f, ac.getTimeSinceLastSweep(), 0.01f);
}

void test_sweep_timer_reset()
{
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                25000.0f, 600.0f, 225.0f, false);

    ac.update(5.0f);
    ac.resetSweepTimer();
    ASSERT_NEAR(0.0f, ac.getTimeSinceLastSweep(), 0.001f);
}

// ============================================================================
void run_aircraft_tests()
{
    TEST_SUITE("Aircraft");

    test_aircraft_construction();
    test_aircraft_initial_position();
    test_aircraft_initial_azimuth();
    test_aircraft_initial_altitude();
    test_aircraft_initial_alive();
    test_aircraft_initial_iff_pending();
    test_aircraft_initial_track_id();
    test_aircraft_friendly_flag();
    test_aircraft_hostile_flag();
    test_aircraft_moves_on_update();
    test_aircraft_range_decreases_inbound();
    test_aircraft_no_update_when_dead();
    test_aircraft_destroy();
    test_aircraft_territory_penetration();
    test_aircraft_territory_penetration_inside();
    test_friendly_no_territory_penetration();
    test_rcs_strategic_bomber();
    test_rcs_stealth_fighter();
    test_rcs_stealth_smaller_than_bomber();
    test_track_data_populated();
    test_track_data_altitude();
    test_track_data_iff_maps_to_classification();
    test_iff_set_hostile();
    test_iff_set_friendly();
    test_threat_score_hostile();
    test_threat_score_friendly_zero();
    test_threat_score_closer_is_higher();
    test_type_name_bomber();
    test_type_name_stealth();
    test_sweep_timer_initial();
    test_sweep_timer_increments();
    test_sweep_timer_reset();

    TEST_SUITE_END();
}
