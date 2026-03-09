// ============================================================================
// GameTypes Unit Tests
// Tests for enums, structs, constants, and data formatting
// ============================================================================

#include "TestFramework.h"
#include "GameTypes.h"

// ---------------------------------------------------------------------------
// Constants validation
// ---------------------------------------------------------------------------
void test_radar_constants()
{
    ASSERT_NEAR(463.0f, GameConstants::RADAR_MAX_RANGE_KM, 0.01f);
}

void test_radar_range_nm()
{
    ASSERT_NEAR(250.0f, GameConstants::RADAR_MAX_RANGE_NM, 0.01f);
}

void test_nm_km_conversion()
{
    ASSERT_NEAR(1.852f, GameConstants::NM_TO_KM, 0.001f);
}

void test_radar_sweep_rate()
{
    // 6 RPM = 36 degrees/sec
    ASSERT_NEAR(6.0f, GameConstants::RADAR_SWEEP_RATE_RPM, 0.01f);
    ASSERT_NEAR(36.0f, GameConstants::RADAR_SWEEP_RATE_DPS, 0.01f);
}

void test_patriot_constants()
{
    ASSERT_NEAR(160.0f, GameConstants::PATRIOT_MAX_RANGE, 0.01f);
    ASSERT_NEAR(3.0f, GameConstants::PATRIOT_MIN_RANGE, 0.01f);
    ASSERT_NEAR(80000.0f, GameConstants::PATRIOT_MAX_ALT, 0.01f);
    ASSERT_NEAR(1000.0f, GameConstants::PATRIOT_MIN_ALT, 0.01f);
    ASSERT_EQ(4, GameConstants::PATRIOT_MAX_MISSILES);
}

void test_hawk_constants()
{
    ASSERT_NEAR(45.0f, GameConstants::HAWK_MAX_RANGE, 0.01f);
    ASSERT_NEAR(1.0f, GameConstants::HAWK_MIN_RANGE, 0.01f);
    ASSERT_NEAR(45000.0f, GameConstants::HAWK_MAX_ALT, 0.01f);
    ASSERT_NEAR(100.0f, GameConstants::HAWK_MIN_ALT, 0.01f);
    ASSERT_EQ(3, GameConstants::HAWK_MAX_MISSILES);
}

void test_scoring_constants()
{
    ASSERT_EQ(100, GameConstants::SCORE_HOSTILE_DESTROYED_BASE);
    ASSERT_EQ(-1000, GameConstants::SCORE_FRIENDLY_DESTROYED);
    ASSERT_EQ(-200, GameConstants::SCORE_HOSTILE_PENETRATED);
    ASSERT_EQ(-25, GameConstants::SCORE_MISSILE_WASTED);
    ASSERT_NEAR(2.0f, GameConstants::SCORE_FIRST_SHOT_MULTIPLIER, 0.01f);
}

void test_javelin_constants()
{
    ASSERT_NEAR(55.0f, GameConstants::JAVELIN_MAX_RANGE, 0.01f);
    ASSERT_NEAR(0.5f, GameConstants::JAVELIN_MIN_RANGE, 0.01f);
    ASSERT_NEAR(15000.0f, GameConstants::JAVELIN_MAX_ALT, 0.01f);
    ASSERT_NEAR(0.0f, GameConstants::JAVELIN_MIN_ALT, 0.01f);
    ASSERT_EQ(2, GameConstants::JAVELIN_MAX_MISSILES);
}

void test_hawk_stock_constants()
{
    ASSERT_EQ(33, GameConstants::HAWK_TOTAL_STOCK);
    ASSERT_EQ(3, GameConstants::HAWK_LOADERS);
}

void test_territory_radius()
{
    ASSERT_NEAR(25.0f, GameConstants::TERRITORY_RADIUS_KM, 0.01f);
}

// ---------------------------------------------------------------------------
// PolarCoord tests
// ---------------------------------------------------------------------------
void test_polar_to_cartesian_north()
{
    PolarCoord p{50.0f, 0.0f};  // 50km due North
    ASSERT_NEAR(0.0f, p.toScreenX(1.0f), 0.1f);
    ASSERT_NEAR(50.0f, p.toScreenY(1.0f), 0.1f);
}

void test_polar_to_cartesian_east()
{
    PolarCoord p{50.0f, 90.0f};  // 50km due East
    ASSERT_NEAR(50.0f, p.toScreenX(1.0f), 0.1f);
    ASSERT_NEAR(0.0f, p.toScreenY(1.0f), 0.1f);
}

void test_polar_to_cartesian_south()
{
    PolarCoord p{50.0f, 180.0f};  // 50km due South
    ASSERT_NEAR(0.0f, p.toScreenX(1.0f), 0.1f);
    ASSERT_NEAR(-50.0f, p.toScreenY(1.0f), 0.1f);
}

void test_polar_to_cartesian_scale()
{
    PolarCoord p{50.0f, 90.0f};
    ASSERT_NEAR(100.0f, p.toScreenX(2.0f), 0.1f);
}

// ---------------------------------------------------------------------------
// TrackData formatting
// ---------------------------------------------------------------------------
void test_track_id_formatting()
{
    TrackData td;
    td.trackId = 1;
    ASSERT_STR_EQ("TK-001", td.getTrackIdString());
}

void test_track_id_formatting_large()
{
    TrackData td;
    td.trackId = 123;
    ASSERT_STR_EQ("TK-123", td.getTrackIdString());
}

void test_altitude_string_flight_level()
{
    TrackData td;
    td.altitude = 35000.0f;
    ASSERT_STR_EQ("FL350", td.getAltitudeString());
}

void test_altitude_string_low()
{
    TrackData td;
    td.altitude = 500.0f;
    ASSERT_STR_EQ("500ft", td.getAltitudeString());
}

void test_classification_strings()
{
    TrackData td;

    td.classification = TrackClassification::HOSTILE;
    ASSERT_STR_EQ("HOSTILE", td.getClassificationString());
}

void test_classification_friendly()
{
    TrackData td;
    td.classification = TrackClassification::FRIENDLY;
    ASSERT_STR_EQ("FRIENDLY", td.getClassificationString());
}

void test_classification_pending()
{
    TrackData td;
    td.classification = TrackClassification::PENDING;
    ASSERT_STR_EQ("PENDING", td.getClassificationString());
}

void test_classification_unknown()
{
    TrackData td;
    td.classification = TrackClassification::UNKNOWN;
    ASSERT_STR_EQ("UNKNOWN", td.getClassificationString());
}

// ============================================================================
void run_game_types_tests()
{
    TEST_SUITE("GameTypes");

    test_radar_constants();
    test_radar_range_nm();
    test_nm_km_conversion();
    test_radar_sweep_rate();
    test_patriot_constants();
    test_hawk_constants();
    test_javelin_constants();
    test_hawk_stock_constants();
    test_scoring_constants();
    test_territory_radius();
    test_polar_to_cartesian_north();
    test_polar_to_cartesian_east();
    test_polar_to_cartesian_south();
    test_polar_to_cartesian_scale();
    test_track_id_formatting();
    test_track_id_formatting_large();
    test_altitude_string_flight_level();
    test_altitude_string_low();
    test_classification_strings();
    test_classification_friendly();
    test_classification_pending();
    test_classification_unknown();

    TEST_SUITE_END();
}
