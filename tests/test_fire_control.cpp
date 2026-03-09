// ============================================================================
// Fire Control System Unit Tests
// ============================================================================

#include "TestFramework.h"
#include "FireControlSystem.h"

// ---------------------------------------------------------------------------
// Initialization
// ---------------------------------------------------------------------------
void test_fcs_construction()
{
    FireControlSystem fcs;
    fcs.init();
    ASSERT_EQ(6, (int)fcs.getBatteries().size());
}

void test_fcs_has_3_patriots()
{
    FireControlSystem fcs;
    fcs.init();

    int patriotCount = 0;
    for (const auto& bat : fcs.getBatteries()) {
        if (bat->getType() == BatteryType::PATRIOT) patriotCount++;
    }
    ASSERT_EQ(3, patriotCount);
}

void test_fcs_has_3_hawks()
{
    FireControlSystem fcs;
    fcs.init();

    int hawkCount = 0;
    for (const auto& bat : fcs.getBatteries()) {
        if (bat->getType() == BatteryType::HAWK) hawkCount++;
    }
    ASSERT_EQ(3, hawkCount);
}

// ---------------------------------------------------------------------------
// Battery lookup
// ---------------------------------------------------------------------------
void test_fcs_get_patriot1()
{
    FireControlSystem fcs;
    fcs.init();

    auto* bat = fcs.getBattery("PATRIOT-1");
    ASSERT_NOT_NULL(bat);
}

void test_fcs_get_hawk2()
{
    FireControlSystem fcs;
    fcs.init();

    auto* bat = fcs.getBattery("HAWK-2");
    ASSERT_NOT_NULL(bat);
}

void test_fcs_get_nonexistent()
{
    FireControlSystem fcs;
    fcs.init();

    auto* bat = fcs.getBattery("THAAD-1");
    ASSERT_NULL(bat);
}

// ---------------------------------------------------------------------------
// Battery designations
// ---------------------------------------------------------------------------
void test_fcs_patriot_designations()
{
    FireControlSystem fcs;
    fcs.init();

    ASSERT_NOT_NULL(fcs.getBattery("PATRIOT-1"));
}

void test_fcs_patriot2_exists()
{
    FireControlSystem fcs;
    fcs.init();

    ASSERT_NOT_NULL(fcs.getBattery("PATRIOT-2"));
}

void test_fcs_patriot3_exists()
{
    FireControlSystem fcs;
    fcs.init();

    ASSERT_NOT_NULL(fcs.getBattery("PATRIOT-3"));
}

void test_fcs_hawk1_exists()
{
    FireControlSystem fcs;
    fcs.init();

    ASSERT_NOT_NULL(fcs.getBattery("HAWK-1"));
}

void test_fcs_hawk3_exists()
{
    FireControlSystem fcs;
    fcs.init();

    ASSERT_NOT_NULL(fcs.getBattery("HAWK-3"));
}

// ---------------------------------------------------------------------------
// All battery data
// ---------------------------------------------------------------------------
void test_fcs_all_battery_data()
{
    FireControlSystem fcs;
    fcs.init();

    auto data = fcs.getAllBatteryData();
    ASSERT_EQ(6, (int)data.size());
}

void test_fcs_all_batteries_ready()
{
    FireControlSystem fcs;
    fcs.init();

    auto data = fcs.getAllBatteryData();
    bool allReady = true;
    for (const auto& d : data) {
        if (d.status != BatteryStatus::READY) {
            allReady = false;
            break;
        }
    }
    ASSERT_TRUE(allReady);
}

// ---------------------------------------------------------------------------
// Available batteries for target
// ---------------------------------------------------------------------------
void test_fcs_available_batteries_in_range()
{
    FireControlSystem fcs;
    fcs.init();
    TrackManager tm;

    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 35.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);
    tm.addTrack(&ac);

    auto available = fcs.getAvailableBatteries(1, tm);
    // At 35km, 35000ft — both Patriot and Hawk should be able to engage
    ASSERT_GT((int)available.size(), 0);
}

void test_fcs_no_batteries_for_nonexistent_track()
{
    FireControlSystem fcs;
    fcs.init();
    TrackManager tm;

    auto available = fcs.getAvailableBatteries(999, tm);
    ASSERT_EQ(0, (int)available.size());
}

// ---------------------------------------------------------------------------
// Reset
// ---------------------------------------------------------------------------
void test_fcs_reset()
{
    FireControlSystem fcs;
    fcs.init();

    // Engage a battery to change state
    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 50.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);
    ac.setTrackId(1);
    auto* bat = fcs.getBattery("PATRIOT-1");
    bat->engage(&ac);

    fcs.reset();

    // All batteries should be fresh
    auto data = fcs.getAllBatteryData();
    bool allReady = true;
    for (const auto& d : data) {
        if (d.status != BatteryStatus::READY) {
            allReady = false;
            break;
        }
    }
    ASSERT_TRUE(allReady);
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------
void test_fcs_update_no_crash()
{
    FireControlSystem fcs;
    fcs.init();
    fcs.update(1.0f);
    ASSERT_TRUE(true);
}

// ---------------------------------------------------------------------------
// Engagement results
// ---------------------------------------------------------------------------
void test_fcs_no_results_initially()
{
    FireControlSystem fcs;
    fcs.init();

    auto results = fcs.getRecentResults();
    ASSERT_EQ(0, (int)results.size());
}

// ============================================================================
void run_fire_control_tests()
{
    TEST_SUITE("Fire Control System");

    test_fcs_construction();
    test_fcs_has_3_patriots();
    test_fcs_has_3_hawks();
    test_fcs_get_patriot1();
    test_fcs_get_hawk2();
    test_fcs_get_nonexistent();
    test_fcs_patriot_designations();
    test_fcs_patriot2_exists();
    test_fcs_patriot3_exists();
    test_fcs_hawk1_exists();
    test_fcs_hawk3_exists();
    test_fcs_all_battery_data();
    test_fcs_all_batteries_ready();
    test_fcs_available_batteries_in_range();
    test_fcs_no_batteries_for_nonexistent_track();
    test_fcs_reset();
    test_fcs_update_no_crash();
    test_fcs_no_results_initially();

    TEST_SUITE_END();
}
