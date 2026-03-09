// ============================================================================
// Missile Battery Unit Tests
// ============================================================================

#include "TestFramework.h"
#include "MissileBattery.h"

// ---------------------------------------------------------------------------
// Patriot construction
// ---------------------------------------------------------------------------
void test_patriot_construction()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    ASSERT_STR_EQ("PATRIOT-1", bat.getDesignation());
}

void test_patriot_type()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    ASSERT_TRUE(bat.getType() == BatteryType::PATRIOT);
}

void test_patriot_initial_ready()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    ASSERT_TRUE(bat.isReady());
}

void test_patriot_initial_missiles()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    ASSERT_EQ(GameConstants::PATRIOT_MAX_MISSILES, bat.getMissilesRemaining());
}

void test_patriot_no_track_assigned()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    ASSERT_EQ(-1, bat.getAssignedTrackId());
}

// ---------------------------------------------------------------------------
// Hawk construction
// ---------------------------------------------------------------------------
void test_hawk_construction()
{
    MissileBattery bat("HAWK-1", BatteryType::HAWK, 3.0f, 60.0f);
    ASSERT_STR_EQ("HAWK-1", bat.getDesignation());
}

void test_hawk_type()
{
    MissileBattery bat("HAWK-1", BatteryType::HAWK, 3.0f, 60.0f);
    ASSERT_TRUE(bat.getType() == BatteryType::HAWK);
}

void test_hawk_initial_missiles()
{
    MissileBattery bat("HAWK-1", BatteryType::HAWK, 3.0f, 60.0f);
    ASSERT_EQ(GameConstants::HAWK_MAX_MISSILES, bat.getMissilesRemaining());
}

// ---------------------------------------------------------------------------
// canEngage validation
// ---------------------------------------------------------------------------
void test_can_engage_in_range()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 50.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);
    ac.setTrackId(1);

    ASSERT_TRUE(bat.canEngage(&ac));
}

void test_cannot_engage_out_of_range()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 170.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);
    ac.setTrackId(1);

    ASSERT_FALSE(bat.canEngage(&ac));
}

void test_cannot_engage_too_close()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    Aircraft ac(AircraftType::ATTACK_DRONE, 2.0f, 0.0f,
                5000.0f, 200.0f, 180.0f, false);
    ac.setTrackId(1);

    ASSERT_FALSE(bat.canEngage(&ac));
}

void test_cannot_engage_too_high_for_hawk()
{
    MissileBattery bat("HAWK-1", BatteryType::HAWK, 3.0f, 60.0f);
    Aircraft ac(AircraftType::RECON_DRONE, 30.0f, 0.0f,
                50000.0f, 150.0f, 180.0f, false);  // Above Hawk ceiling
    ac.setTrackId(1);

    ASSERT_FALSE(bat.canEngage(&ac));
}

void test_cannot_engage_dead_target()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 50.0f, 0.0f,
                25000.0f, 600.0f, 180.0f, false);
    ac.setTrackId(1);
    ac.destroy();

    ASSERT_FALSE(bat.canEngage(&ac));
}

void test_cannot_engage_null()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    ASSERT_FALSE(bat.canEngage(nullptr));
}

void test_hawk_can_engage_low_alt()
{
    MissileBattery bat("HAWK-1", BatteryType::HAWK, 3.0f, 60.0f);
    Aircraft ac(AircraftType::ATTACK_DRONE, 20.0f, 0.0f,
                500.0f, 200.0f, 180.0f, false);
    ac.setTrackId(1);

    ASSERT_TRUE(bat.canEngage(&ac));
}

void test_patriot_cannot_engage_very_low()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    Aircraft ac(AircraftType::ATTACK_DRONE, 20.0f, 0.0f,
                500.0f, 200.0f, 180.0f, false);  // Below Patriot floor
    ac.setTrackId(1);

    ASSERT_FALSE(bat.canEngage(&ac));
}

// ---------------------------------------------------------------------------
// Engagement
// ---------------------------------------------------------------------------
void test_engage_success()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 50.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);
    ac.setTrackId(1);

    ASSERT_TRUE(bat.engage(&ac));
}

void test_engage_consumes_missile()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 50.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);
    ac.setTrackId(1);

    int before = bat.getMissilesRemaining();
    bat.engage(&ac);
    ASSERT_EQ(before - 1, bat.getMissilesRemaining());
}

void test_engage_changes_status()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 50.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);
    ac.setTrackId(1);

    bat.engage(&ac);
    ASSERT_TRUE(bat.getStatus() == BatteryStatus::ENGAGED);
}

void test_engage_sets_track_id()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 50.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);
    ac.setTrackId(7);

    bat.engage(&ac);
    ASSERT_EQ(7, bat.getAssignedTrackId());
}

void test_cannot_engage_while_engaged()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    Aircraft ac1(AircraftType::STRATEGIC_BOMBER, 50.0f, 0.0f,
                 35000.0f, 500.0f, 180.0f, false);
    Aircraft ac2(AircraftType::FIGHTER_ATTACK, 40.0f, 90.0f,
                 25000.0f, 600.0f, 270.0f, false);
    ac1.setTrackId(1);
    ac2.setTrackId(2);

    bat.engage(&ac1);
    ASSERT_FALSE(bat.engage(&ac2));
}

// ---------------------------------------------------------------------------
// Missile flight and result
// ---------------------------------------------------------------------------
void test_engagement_produces_result()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 50.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);
    ac.setTrackId(1);

    bat.engage(&ac);

    // Simulate enough time for missile to arrive
    // Patriot at 1700 m/s, 50km = ~29.4 seconds
    for (int i = 0; i < 40; i++) {
        bat.update(1.0f);
    }

    ASSERT_TRUE(bat.hasEngagementResult());
}

void test_engagement_result_is_hit_or_miss()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 50.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);
    ac.setTrackId(1);

    bat.engage(&ac);
    for (int i = 0; i < 40; i++) bat.update(1.0f);

    auto result = bat.getLastResult();
    ASSERT_TRUE(result == EngagementResult::HIT || result == EngagementResult::MISS);
}

void test_battery_returns_to_ready_after_engagement()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 50.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);
    ac.setTrackId(1);

    bat.engage(&ac);
    for (int i = 0; i < 40; i++) bat.update(1.0f);

    // Should be READY or RELOADING (if out of missiles)
    auto status = bat.getStatus();
    ASSERT_TRUE(status == BatteryStatus::READY || status == BatteryStatus::RELOADING);
}

void test_clear_engagement_result()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 50.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);
    ac.setTrackId(1);

    bat.engage(&ac);
    for (int i = 0; i < 40; i++) bat.update(1.0f);

    bat.clearEngagementResult();
    ASSERT_FALSE(bat.hasEngagementResult());
}

// ---------------------------------------------------------------------------
// Reload cycle
// ---------------------------------------------------------------------------
void test_reload_after_all_missiles_spent()
{
    MissileBattery bat("HAWK-1", BatteryType::HAWK, 3.0f, 60.0f);

    // Fire all 3 Hawk missiles
    for (int m = 0; m < GameConstants::HAWK_MAX_MISSILES; m++) {
        Aircraft* ac = new Aircraft(AircraftType::ATTACK_DRONE, 20.0f,
                                     (float)(m * 30), 5000.0f, 200.0f, 180.0f, false);
        ac->setTrackId(m + 1);
        bat.engage(ac);

        // Wait for missile to arrive
        for (int t = 0; t < 30; t++) bat.update(1.0f);

        bat.clearEngagementResult();
        delete ac;
    }

    // After all missiles spent, should be reloading
    ASSERT_TRUE(bat.getStatus() == BatteryStatus::RELOADING);
}

void test_reload_completes()
{
    MissileBattery bat("HAWK-1", BatteryType::HAWK, 3.0f, 60.0f);

    // Fire all missiles
    for (int m = 0; m < GameConstants::HAWK_MAX_MISSILES; m++) {
        Aircraft* ac = new Aircraft(AircraftType::ATTACK_DRONE, 20.0f,
                                     (float)(m * 30), 5000.0f, 200.0f, 180.0f, false);
        ac->setTrackId(m + 1);
        bat.engage(ac);
        for (int t = 0; t < 30; t++) bat.update(1.0f);
        bat.clearEngagementResult();
        delete ac;
    }

    // Wait for reload to complete
    for (int t = 0; t < 15; t++) bat.update(1.0f);

    ASSERT_TRUE(bat.isReady());
}

// ---------------------------------------------------------------------------
// Battery data
// ---------------------------------------------------------------------------
void test_battery_data_populated()
{
    MissileBattery bat("PATRIOT-2", BatteryType::PATRIOT, 5.0f, 120.0f);
    BatteryData data = bat.getData();

    ASSERT_STR_EQ("PATRIOT-2", data.designation);
}

void test_battery_data_range()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    BatteryData data = bat.getData();

    ASSERT_NEAR(GameConstants::PATRIOT_MAX_RANGE, data.maxRange, 0.01f);
}

void test_battery_data_missiles()
{
    MissileBattery bat("HAWK-2", BatteryType::HAWK, 3.0f, 180.0f);
    BatteryData data = bat.getData();

    ASSERT_EQ(GameConstants::HAWK_MAX_MISSILES, data.missilesRemaining);
}

// ---------------------------------------------------------------------------
// Abort engagement
// ---------------------------------------------------------------------------
void test_abort_tracking()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 5.0f, 0.0f);
    // Can't easily test abort since engage goes straight to ENGAGED,
    // but verify abortEngagement doesn't crash
    bat.abortEngagement();
    ASSERT_TRUE(bat.isReady());
}

// ---------------------------------------------------------------------------
// Javelin battery
// ---------------------------------------------------------------------------
void test_javelin_construction()
{
    MissileBattery bat("JAVELIN-1", BatteryType::JAVELIN, 3.0f, 30.0f);
    ASSERT_STR_EQ("JAVELIN-1", bat.getDesignation());
}

void test_javelin_type()
{
    MissileBattery bat("JAVELIN-1", BatteryType::JAVELIN, 3.0f, 30.0f);
    ASSERT_TRUE(bat.getType() == BatteryType::JAVELIN);
}

void test_javelin_initial_missiles()
{
    MissileBattery bat("JAVELIN-1", BatteryType::JAVELIN, 3.0f, 30.0f);
    ASSERT_EQ(GameConstants::JAVELIN_MAX_MISSILES, bat.getMissilesRemaining());
}

void test_javelin_can_engage_close_target()
{
    MissileBattery bat("JAVELIN-1", BatteryType::JAVELIN, 3.0f, 30.0f);
    Aircraft ac(AircraftType::ATTACK_DRONE, 30.0f, 0.0f,
                5000.0f, 200.0f, 180.0f, false);
    ac.setTrackId(1);

    ASSERT_TRUE(bat.canEngage(&ac));
}

void test_javelin_cannot_engage_high_alt()
{
    MissileBattery bat("JAVELIN-1", BatteryType::JAVELIN, 3.0f, 30.0f);
    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 30.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);  // Too high for Javelin
    ac.setTrackId(1);

    ASSERT_FALSE(bat.canEngage(&ac));
}

void test_javelin_ir_seeker_no_missile_tracking()
{
    MissileBattery bat("JAVELIN-1", BatteryType::JAVELIN, 3.0f, 30.0f);
    ASSERT_FALSE(bat.hasMissileTracking());
}

// ---------------------------------------------------------------------------
// Hawk ammo stock
// ---------------------------------------------------------------------------
void test_hawk_missile_stock()
{
    MissileBattery bat("HAWK-1", BatteryType::HAWK, 8.0f, 60.0f);
    // Ready: 3, Stock: 30 remaining, Total reported: 33
    ASSERT_EQ(GameConstants::HAWK_TOTAL_STOCK, bat.getTotalMissileStock() + bat.getMissilesRemaining());
}

void test_hawk_loaders()
{
    MissileBattery bat("HAWK-1", BatteryType::HAWK, 8.0f, 60.0f);
    ASSERT_EQ(GameConstants::HAWK_LOADERS, bat.getLoaderCount());
}

// ---------------------------------------------------------------------------
// Tracking radar
// ---------------------------------------------------------------------------
void test_patriot_tracking_radar_type()
{
    MissileBattery bat("PATRIOT-1", BatteryType::PATRIOT, 15.0f, 0.0f);
    ASSERT_STR_EQ("AN/MPQ-53", bat.getTrackingRadarType());
    ASSERT_TRUE(bat.hasMissileTracking());
}

void test_hawk_tracking_radar_type()
{
    MissileBattery bat("HAWK-1", BatteryType::HAWK, 8.0f, 60.0f);
    ASSERT_STR_EQ("AN/MPQ-46 HPI", bat.getTrackingRadarType());
    ASSERT_TRUE(bat.hasMissileTracking());
}

// ============================================================================
void run_missile_battery_tests()
{
    TEST_SUITE("Missile Battery");

    test_patriot_construction();
    test_patriot_type();
    test_patriot_initial_ready();
    test_patriot_initial_missiles();
    test_patriot_no_track_assigned();
    test_hawk_construction();
    test_hawk_type();
    test_hawk_initial_missiles();
    test_can_engage_in_range();
    test_cannot_engage_out_of_range();
    test_cannot_engage_too_close();
    test_cannot_engage_too_high_for_hawk();
    test_cannot_engage_dead_target();
    test_cannot_engage_null();
    test_hawk_can_engage_low_alt();
    test_patriot_cannot_engage_very_low();
    test_engage_success();
    test_engage_consumes_missile();
    test_engage_changes_status();
    test_engage_sets_track_id();
    test_cannot_engage_while_engaged();
    test_engagement_produces_result();
    test_engagement_result_is_hit_or_miss();
    test_battery_returns_to_ready_after_engagement();
    test_clear_engagement_result();
    test_reload_after_all_missiles_spent();
    test_reload_completes();
    test_battery_data_populated();
    test_battery_data_range();
    test_battery_data_missiles();
    test_abort_tracking();
    test_javelin_construction();
    test_javelin_type();
    test_javelin_initial_missiles();
    test_javelin_can_engage_close_target();
    test_javelin_cannot_engage_high_alt();
    test_javelin_ir_seeker_no_missile_tracking();
    test_hawk_missile_stock();
    test_hawk_loaders();
    test_patriot_tracking_radar_type();
    test_hawk_tracking_radar_type();

    TEST_SUITE_END();
}
