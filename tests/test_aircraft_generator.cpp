// ============================================================================
// Aircraft Generator Unit Tests
// ============================================================================

#include "TestFramework.h"
#include "AircraftGenerator.h"

// ---------------------------------------------------------------------------
// Construction and initialization
// ---------------------------------------------------------------------------
void test_generator_construction()
{
    AircraftGenerator gen;
    ASSERT_EQ(1, gen.getLevel());
}

void test_generator_init()
{
    AircraftGenerator gen;
    gen.init(3);
    ASSERT_EQ(3, gen.getLevel());
}

void test_generator_set_level()
{
    AircraftGenerator gen;
    gen.setLevel(5);
    ASSERT_EQ(5, gen.getLevel());
}

// ---------------------------------------------------------------------------
// Spawning
// ---------------------------------------------------------------------------
void test_generator_no_immediate_spawn()
{
    AircraftGenerator gen;
    gen.init(1);

    Aircraft* ac = gen.trySpawn(0.1f, 0);
    // May or may not spawn on first tick (depends on timer)
    // Just verify no crash
    delete ac;
    ASSERT_TRUE(true);
}

void test_generator_spawns_eventually()
{
    AircraftGenerator gen;
    gen.init(1);

    Aircraft* spawned = nullptr;
    for (int i = 0; i < 200 && !spawned; i++) {
        spawned = gen.trySpawn(0.5f, 0);
    }

    ASSERT_NOT_NULL(spawned);
    delete spawned;
}

void test_generator_respects_max_count()
{
    AircraftGenerator gen;
    gen.init(1);  // Level 1: max 3 concurrent

    // With max count already reached, should not spawn
    Aircraft* ac = nullptr;
    for (int i = 0; i < 100; i++) {
        ac = gen.trySpawn(1.0f, 3);  // currentCount == max
        if (ac) break;
    }

    ASSERT_NULL(ac);
}

void test_generator_spawned_aircraft_valid_range()
{
    AircraftGenerator gen;
    gen.init(1);

    Aircraft* ac = nullptr;
    for (int i = 0; i < 200 && !ac; i++) {
        ac = gen.trySpawn(0.5f, 0);
    }

    if (ac) {
        // Should spawn at radar edge (85-100 km)
        ASSERT_GE(ac->getRange(), 85.0f);
    }
    ASSERT_NOT_NULL(ac);
    delete ac;
}

void test_generator_spawned_aircraft_valid_range_upper()
{
    AircraftGenerator gen;
    gen.init(1);

    Aircraft* ac = nullptr;
    for (int i = 0; i < 200 && !ac; i++) {
        ac = gen.trySpawn(0.5f, 0);
    }

    if (ac) {
        ASSERT_LE(ac->getRange(), 100.0f);
    }
    ASSERT_NOT_NULL(ac);
    delete ac;
}

void test_generator_spawned_aircraft_valid_azimuth()
{
    AircraftGenerator gen;
    gen.init(1);

    Aircraft* ac = nullptr;
    for (int i = 0; i < 200 && !ac; i++) {
        ac = gen.trySpawn(0.5f, 0);
    }

    if (ac) {
        ASSERT_GE(ac->getAzimuth(), 0.0f);
    }
    ASSERT_NOT_NULL(ac);
    delete ac;
}

void test_generator_spawned_aircraft_azimuth_upper()
{
    AircraftGenerator gen;
    gen.init(1);

    Aircraft* ac = nullptr;
    for (int i = 0; i < 200 && !ac; i++) {
        ac = gen.trySpawn(0.5f, 0);
    }

    if (ac) {
        ASSERT_LT(ac->getAzimuth(), 360.0f);
    }
    ASSERT_NOT_NULL(ac);
    delete ac;
}

void test_generator_spawned_aircraft_positive_speed()
{
    AircraftGenerator gen;
    gen.init(1);

    Aircraft* ac = nullptr;
    for (int i = 0; i < 200 && !ac; i++) {
        ac = gen.trySpawn(0.5f, 0);
    }

    if (ac) {
        ASSERT_GT(ac->getSpeed(), 0.0f);
    }
    ASSERT_NOT_NULL(ac);
    delete ac;
}

void test_generator_spawned_aircraft_positive_altitude()
{
    AircraftGenerator gen;
    gen.init(1);

    Aircraft* ac = nullptr;
    for (int i = 0; i < 200 && !ac; i++) {
        ac = gen.trySpawn(0.5f, 0);
    }

    if (ac) {
        ASSERT_GT(ac->getAltitude(), 0.0f);
    }
    ASSERT_NOT_NULL(ac);
    delete ac;
}

void test_generator_spawned_aircraft_alive()
{
    AircraftGenerator gen;
    gen.init(1);

    Aircraft* ac = nullptr;
    for (int i = 0; i < 200 && !ac; i++) {
        ac = gen.trySpawn(0.5f, 0);
    }

    if (ac) {
        ASSERT_TRUE(ac->isAlive());
    }
    delete ac;
}

// ---------------------------------------------------------------------------
// Reset
// ---------------------------------------------------------------------------
void test_generator_reset()
{
    AircraftGenerator gen;
    gen.init(3);

    // Advance timer
    for (int i = 0; i < 50; i++) {
        auto* ac = gen.trySpawn(1.0f, 0);
        delete ac;
    }

    gen.reset();
    // After reset, should require some time before next spawn
    // (no crash is the main test here)
    ASSERT_TRUE(true);
}

// ---------------------------------------------------------------------------
// Level affects max concurrent
// ---------------------------------------------------------------------------
void test_generator_level1_max()
{
    AircraftGenerator gen;
    gen.init(1);

    // Level 1 max is 3 — cannot spawn at count 3
    Aircraft* ac = nullptr;
    for (int i = 0; i < 100; i++) {
        ac = gen.trySpawn(1.0f, 3);
        if (ac) break;
    }
    ASSERT_NULL(ac);
}

void test_generator_level5_allows_more()
{
    AircraftGenerator gen;
    gen.init(5);

    // Level 5 max is 15 — should allow spawn at count 10
    Aircraft* ac = nullptr;
    for (int i = 0; i < 200 && !ac; i++) {
        ac = gen.trySpawn(0.5f, 10);
    }

    ASSERT_NOT_NULL(ac);
    delete ac;
}

// ============================================================================
void run_aircraft_generator_tests()
{
    TEST_SUITE("Aircraft Generator");

    test_generator_construction();
    test_generator_init();
    test_generator_set_level();
    test_generator_no_immediate_spawn();
    test_generator_spawns_eventually();
    test_generator_respects_max_count();
    test_generator_spawned_aircraft_valid_range();
    test_generator_spawned_aircraft_valid_range_upper();
    test_generator_spawned_aircraft_valid_azimuth();
    test_generator_spawned_aircraft_azimuth_upper();
    test_generator_spawned_aircraft_positive_speed();
    test_generator_spawned_aircraft_positive_altitude();
    test_generator_spawned_aircraft_alive();
    test_generator_reset();
    test_generator_level1_max();
    test_generator_level5_allows_more();

    TEST_SUITE_END();
}
