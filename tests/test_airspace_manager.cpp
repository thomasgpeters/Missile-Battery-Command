// ============================================================================
// Airspace Manager Unit Tests
// ============================================================================

#include "TestFramework.h"
#include "AirspaceManager.h"
#include "Aircraft.h"

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
void test_airspace_empty()
{
    AirspaceManager am;
    ASSERT_EQ(0, (int)am.getZones().size());
}

// ---------------------------------------------------------------------------
// Zone management
// ---------------------------------------------------------------------------
void test_airspace_add_zone()
{
    AirspaceManager am;
    AirspaceZone zone;
    zone.designation = "NFZ-TEST";
    zone.type = AirspaceZoneType::NO_FLY_ZONE;
    zone.startAzimuth = 80.0f;
    zone.endAzimuth = 100.0f;
    zone.minRange = 0.0f;
    zone.maxRange = 463.0f;
    zone.minAltitude = 0.0f;
    zone.maxAltitude = 50000.0f;
    zone.hasHeadingConstraint = false;
    zone.active = true;

    am.addZone(zone);
    ASSERT_EQ(1, (int)am.getZones().size());
}

void test_airspace_remove_zone()
{
    AirspaceManager am;
    AirspaceZone zone;
    zone.designation = "NFZ-TEST";
    zone.type = AirspaceZoneType::NO_FLY_ZONE;
    zone.active = true;
    am.addZone(zone);

    am.removeZone("NFZ-TEST");
    ASSERT_EQ(0, (int)am.getZones().size());
}

void test_airspace_get_zone()
{
    AirspaceManager am;
    AirspaceZone zone;
    zone.designation = "RTB-ALPHA";
    zone.type = AirspaceZoneType::RETURN_CORRIDOR;
    zone.active = true;
    am.addZone(zone);

    auto* z = am.getZone("RTB-ALPHA");
    ASSERT_NOT_NULL(z);
}

void test_airspace_get_nonexistent()
{
    AirspaceManager am;
    ASSERT_NULL(am.getZone("NONEXIST"));
}

void test_airspace_activate_deactivate()
{
    AirspaceManager am;
    AirspaceZone zone;
    zone.designation = "FZ-SOUTH";
    zone.type = AirspaceZoneType::FREE_ZONE;
    zone.active = true;
    am.addZone(zone);

    am.deactivateZone("FZ-SOUTH");
    auto* z = am.getZone("FZ-SOUTH");
    ASSERT_FALSE(z->active);

    am.activateZone("FZ-SOUTH");
    ASSERT_TRUE(z->active);
}

// ---------------------------------------------------------------------------
// No-fly zone detection
// ---------------------------------------------------------------------------
void test_airspace_nfz_contains()
{
    AirspaceManager am;
    AirspaceZone nfz;
    nfz.designation = "NFZ-EAST";
    nfz.type = AirspaceZoneType::NO_FLY_ZONE;
    nfz.startAzimuth = 80.0f;
    nfz.endAzimuth = 100.0f;
    nfz.minRange = 0.0f;
    nfz.maxRange = 463.0f;
    nfz.minAltitude = 0.0f;
    nfz.maxAltitude = 10000.0f;
    nfz.hasHeadingConstraint = false;
    nfz.active = true;
    am.addZone(nfz);

    // Aircraft at 90 degrees, 200km, 5000ft — should be in NFZ
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 200.0f, 90.0f,
                5000.0f, 600.0f, 270.0f, false);
    ASSERT_TRUE(am.isInNoFlyZone(&ac));
}

void test_airspace_nfz_outside()
{
    AirspaceManager am;
    AirspaceZone nfz;
    nfz.designation = "NFZ-EAST";
    nfz.type = AirspaceZoneType::NO_FLY_ZONE;
    nfz.startAzimuth = 80.0f;
    nfz.endAzimuth = 100.0f;
    nfz.minRange = 0.0f;
    nfz.maxRange = 463.0f;
    nfz.minAltitude = 0.0f;
    nfz.maxAltitude = 10000.0f;
    nfz.hasHeadingConstraint = false;
    nfz.active = true;
    am.addZone(nfz);

    // Aircraft at 45 degrees — outside NFZ sector
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 200.0f, 45.0f,
                5000.0f, 600.0f, 225.0f, false);
    ASSERT_FALSE(am.isInNoFlyZone(&ac));
}

void test_airspace_nfz_altitude_above()
{
    AirspaceManager am;
    AirspaceZone nfz;
    nfz.designation = "NFZ-EAST";
    nfz.type = AirspaceZoneType::NO_FLY_ZONE;
    nfz.startAzimuth = 80.0f;
    nfz.endAzimuth = 100.0f;
    nfz.minRange = 0.0f;
    nfz.maxRange = 463.0f;
    nfz.minAltitude = 0.0f;
    nfz.maxAltitude = 10000.0f;
    nfz.hasHeadingConstraint = false;
    nfz.active = true;
    am.addZone(nfz);

    // Aircraft at 90 degrees but above NFZ ceiling
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 200.0f, 90.0f,
                35000.0f, 600.0f, 270.0f, false);
    ASSERT_FALSE(am.isInNoFlyZone(&ac));
}

// ---------------------------------------------------------------------------
// Return corridor
// ---------------------------------------------------------------------------
void test_airspace_corridor_match()
{
    AirspaceManager am;
    AirspaceZone rtb;
    rtb.designation = "RTB-ALPHA";
    rtb.type = AirspaceZoneType::RETURN_CORRIDOR;
    rtb.startAzimuth = 210.0f;
    rtb.endAzimuth = 240.0f;
    rtb.minRange = 30.0f;
    rtb.maxRange = 463.0f;
    rtb.minAltitude = 20000.0f;
    rtb.maxAltitude = 40000.0f;
    rtb.allowedHeadingMin = 25.0f;
    rtb.allowedHeadingMax = 65.0f;
    rtb.hasHeadingConstraint = true;
    rtb.active = true;
    am.addZone(rtb);

    // Aircraft at 220 deg azimuth, 100km, FL300, heading 045 — matches corridor
    Aircraft ac(AircraftType::FRIENDLY_MILITARY, 100.0f, 220.0f,
                30000.0f, 450.0f, 45.0f, true);
    ASSERT_TRUE(am.matchesReturnCorridor(&ac));
}

void test_airspace_corridor_wrong_heading()
{
    AirspaceManager am;
    AirspaceZone rtb;
    rtb.designation = "RTB-ALPHA";
    rtb.type = AirspaceZoneType::RETURN_CORRIDOR;
    rtb.startAzimuth = 210.0f;
    rtb.endAzimuth = 240.0f;
    rtb.minRange = 30.0f;
    rtb.maxRange = 463.0f;
    rtb.minAltitude = 20000.0f;
    rtb.maxAltitude = 40000.0f;
    rtb.allowedHeadingMin = 25.0f;
    rtb.allowedHeadingMax = 65.0f;
    rtb.hasHeadingConstraint = true;
    rtb.active = true;
    am.addZone(rtb);

    // Aircraft at correct position but heading 180 — does NOT match corridor
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 100.0f, 220.0f,
                30000.0f, 600.0f, 180.0f, false);
    ASSERT_FALSE(am.matchesReturnCorridor(&ac));
}

// ---------------------------------------------------------------------------
// Default airspace
// ---------------------------------------------------------------------------
void test_airspace_default_init()
{
    AirspaceManager am;
    am.initDefaultAirspace();

    // Should have 3 default zones
    ASSERT_EQ(3, (int)am.getZones().size());
    ASSERT_NOT_NULL(am.getZone("RTB-ALPHA"));
    ASSERT_NOT_NULL(am.getZone("RTB-BRAVO"));
    ASSERT_NOT_NULL(am.getZone("NFZ-EAST"));
}

// ---------------------------------------------------------------------------
// Zone type strings
// ---------------------------------------------------------------------------
void test_airspace_zone_type_strings()
{
    AirspaceZone z;
    z.type = AirspaceZoneType::NO_FLY_ZONE;
    ASSERT_STR_EQ("NFZ", z.getTypeString());

    z.type = AirspaceZoneType::FREE_ZONE;
    ASSERT_STR_EQ("FREE", z.getTypeString());

    z.type = AirspaceZoneType::RETURN_CORRIDOR;
    ASSERT_STR_EQ("RTB", z.getTypeString());
}

// ---------------------------------------------------------------------------
// Azimuth wrap-around
// ---------------------------------------------------------------------------
void test_airspace_azimuth_wraparound()
{
    AirspaceManager am;
    AirspaceZone nfz;
    nfz.designation = "NFZ-NORTH";
    nfz.type = AirspaceZoneType::NO_FLY_ZONE;
    nfz.startAzimuth = 350.0f;
    nfz.endAzimuth = 10.0f;  // Wraps around north
    nfz.minRange = 0.0f;
    nfz.maxRange = 463.0f;
    nfz.minAltitude = 0.0f;
    nfz.maxAltitude = 80000.0f;
    nfz.hasHeadingConstraint = false;
    nfz.active = true;
    am.addZone(nfz);

    // Aircraft at 355 degrees — should be in the wrap-around zone
    Aircraft ac1(AircraftType::FIGHTER_ATTACK, 200.0f, 355.0f,
                 20000.0f, 600.0f, 180.0f, false);
    ASSERT_TRUE(am.isInNoFlyZone(&ac1));

    // Aircraft at 5 degrees — also in wrap-around zone
    Aircraft ac2(AircraftType::FIGHTER_ATTACK, 200.0f, 5.0f,
                 20000.0f, 600.0f, 180.0f, false);
    ASSERT_TRUE(am.isInNoFlyZone(&ac2));
}

// ============================================================================
void run_airspace_manager_tests()
{
    TEST_SUITE("Airspace Manager");

    test_airspace_empty();
    test_airspace_add_zone();
    test_airspace_remove_zone();
    test_airspace_get_zone();
    test_airspace_get_nonexistent();
    test_airspace_activate_deactivate();
    test_airspace_nfz_contains();
    test_airspace_nfz_outside();
    test_airspace_nfz_altitude_above();
    test_airspace_corridor_match();
    test_airspace_corridor_wrong_heading();
    test_airspace_default_init();
    test_airspace_zone_type_strings();
    test_airspace_azimuth_wraparound();

    TEST_SUITE_END();
}
