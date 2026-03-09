// ============================================================================
// Threat Board Unit Tests — AN/TSQ-73 Scope 2
// ============================================================================

#include "TestFramework.h"
#include "ThreatBoard.h"

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
void test_threat_board_empty()
{
    ThreatBoard board;
    ASSERT_EQ(0, board.getThreatCount());
}

void test_threat_board_max_threats()
{
    ASSERT_EQ(5, ThreatBoard::MAX_DISPLAYED_THREATS);
}

// ---------------------------------------------------------------------------
// Threat evaluation
// ---------------------------------------------------------------------------
void test_threat_board_detects_hostile()
{
    ThreatBoard board;
    TrackManager tm;

    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 100.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);
    tm.addTrack(&ac);
    ac.setIFFStatus(IFFStatus::HOSTILE);

    board.update(tm);
    ASSERT_GT(board.getThreatCount(), 0);
}

void test_threat_board_ignores_friendly()
{
    ThreatBoard board;
    TrackManager tm;

    Aircraft ac(AircraftType::FRIENDLY_MILITARY, 100.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, true);
    tm.addTrack(&ac);
    ac.setIFFStatus(IFFStatus::FRIENDLY);
    tm.update(0.0f);  // Refresh track data with IFF status

    board.update(tm);
    ASSERT_EQ(0, board.getThreatCount());
}

void test_threat_board_limits_to_5()
{
    ThreatBoard board;
    TrackManager tm;

    // Create 8 hostile aircraft
    Aircraft ac1(AircraftType::FIGHTER_ATTACK, 100.0f, 0.0f, 30000.0f, 500.0f, 180.0f, false);
    Aircraft ac2(AircraftType::FIGHTER_ATTACK, 90.0f, 45.0f, 25000.0f, 600.0f, 225.0f, false);
    Aircraft ac3(AircraftType::STRATEGIC_BOMBER, 80.0f, 90.0f, 35000.0f, 450.0f, 270.0f, false);
    Aircraft ac4(AircraftType::ATTACK_DRONE, 70.0f, 135.0f, 10000.0f, 200.0f, 315.0f, false);
    Aircraft ac5(AircraftType::TACTICAL_BOMBER, 60.0f, 180.0f, 20000.0f, 550.0f, 0.0f, false);
    Aircraft ac6(AircraftType::RECON_DRONE, 50.0f, 225.0f, 15000.0f, 150.0f, 45.0f, false);
    Aircraft ac7(AircraftType::STEALTH_FIGHTER, 40.0f, 270.0f, 40000.0f, 700.0f, 90.0f, false);
    Aircraft ac8(AircraftType::FIGHTER_ATTACK, 30.0f, 315.0f, 28000.0f, 500.0f, 135.0f, false);

    tm.addTrack(&ac1); tm.addTrack(&ac2); tm.addTrack(&ac3); tm.addTrack(&ac4);
    tm.addTrack(&ac5); tm.addTrack(&ac6); tm.addTrack(&ac7); tm.addTrack(&ac8);

    board.update(tm);
    ASSERT_EQ(5, board.getThreatCount());
}

void test_threat_board_closer_ranks_higher()
{
    ThreatBoard board;
    TrackManager tm;

    // Both inbound, but one is much closer
    Aircraft far(AircraftType::STRATEGIC_BOMBER, 200.0f, 0.0f,
                 35000.0f, 500.0f, 180.0f, false);
    Aircraft close(AircraftType::STRATEGIC_BOMBER, 30.0f, 0.0f,
                   35000.0f, 500.0f, 180.0f, false);
    tm.addTrack(&far);
    tm.addTrack(&close);

    board.update(tm);
    ASSERT_EQ(2, board.getThreatCount());

    // Close target should be ranked first (higher threat score)
    auto* top = board.getThreat(0);
    ASSERT_NOT_NULL(top);
    ASSERT_NEAR(30.0f, top->range, 1.0f);
}

void test_threat_board_inbound_ranks_higher()
{
    ThreatBoard board;
    TrackManager tm;

    // Same range, but one is inbound and one is outbound
    Aircraft inbound(AircraftType::FIGHTER_ATTACK, 100.0f, 0.0f,
                     30000.0f, 500.0f, 180.0f, false);  // Heading south (toward center from north)
    Aircraft outbound(AircraftType::FIGHTER_ATTACK, 100.0f, 180.0f,
                      30000.0f, 500.0f, 180.0f, false);  // Heading south (away from center from south)
    tm.addTrack(&inbound);
    tm.addTrack(&outbound);

    board.update(tm);

    auto* top = board.getThreat(0);
    ASSERT_NOT_NULL(top);
    ASSERT_TRUE(top->inbound);
}

// ---------------------------------------------------------------------------
// Inbound detection
// ---------------------------------------------------------------------------
void test_threat_inbound_north_heading_south()
{
    ThreatBoard board;
    TrackManager tm;

    // Aircraft at azimuth 0 (north), heading 180 (south = toward center)
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 100.0f, 0.0f,
                30000.0f, 500.0f, 180.0f, false);
    tm.addTrack(&ac);

    board.update(tm);
    auto* threat = board.getThreat(0);
    ASSERT_NOT_NULL(threat);
    ASSERT_TRUE(threat->inbound);
}

void test_threat_outbound_north_heading_north()
{
    ThreatBoard board;
    TrackManager tm;

    // Aircraft at azimuth 0 (north), heading 0 (north = away from center)
    Aircraft ac(AircraftType::FIGHTER_ATTACK, 100.0f, 0.0f,
                30000.0f, 500.0f, 0.0f, false);
    tm.addTrack(&ac);

    board.update(tm);
    auto* threat = board.getThreat(0);
    ASSERT_NOT_NULL(threat);
    ASSERT_FALSE(threat->inbound);
}

// ---------------------------------------------------------------------------
// Lookup
// ---------------------------------------------------------------------------
void test_threat_board_is_on_board()
{
    ThreatBoard board;
    TrackManager tm;

    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 100.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);
    int id = tm.addTrack(&ac);

    board.update(tm);
    ASSERT_TRUE(board.isOnBoard(id));
}

void test_threat_board_not_on_board()
{
    ThreatBoard board;
    ASSERT_FALSE(board.isOnBoard(999));
}

void test_threat_board_get_by_track_id()
{
    ThreatBoard board;
    TrackManager tm;

    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                25000.0f, 600.0f, 225.0f, false);
    int id = tm.addTrack(&ac);

    board.update(tm);
    auto* entry = board.getThreatByTrackId(id);
    ASSERT_NOT_NULL(entry);
    ASSERT_EQ(id, entry->trackId);
}

// ---------------------------------------------------------------------------
// Formatting
// ---------------------------------------------------------------------------
void test_threat_board_format()
{
    ThreatBoard board;
    TrackManager tm;

    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 100.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);
    tm.addTrack(&ac);

    board.update(tm);
    auto text = board.formatBoard();
    ASSERT_GT((int)text.size(), 0);
}

void test_threat_entry_format_line()
{
    ThreatBoard board;
    TrackManager tm;

    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 100.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);
    tm.addTrack(&ac);

    board.update(tm);
    auto* threat = board.getThreat(0);
    ASSERT_NOT_NULL(threat);
    auto line = threat->formatLine();
    ASSERT_GT((int)line.size(), 0);
}

void test_threat_entry_format_card()
{
    ThreatBoard board;
    TrackManager tm;

    Aircraft ac(AircraftType::FIGHTER_ATTACK, 80.0f, 45.0f,
                25000.0f, 600.0f, 225.0f, false);
    tm.addTrack(&ac);

    board.update(tm);
    auto* threat = board.getThreat(0);
    ASSERT_NOT_NULL(threat);
    auto card = threat->formatCard();
    ASSERT_GT((int)card.size(), 0);
}

// ---------------------------------------------------------------------------
// Time to territory
// ---------------------------------------------------------------------------
void test_threat_time_to_territory_inbound()
{
    ThreatBoard board;
    TrackManager tm;

    // Inbound bomber: should have finite ETA
    Aircraft ac(AircraftType::STRATEGIC_BOMBER, 100.0f, 0.0f,
                35000.0f, 500.0f, 180.0f, false);
    tm.addTrack(&ac);

    board.update(tm);
    auto* threat = board.getThreat(0);
    ASSERT_NOT_NULL(threat);
    ASSERT_TRUE(threat->timeToTerritory < 9000.0f);
    ASSERT_GT(threat->timeToTerritory, 0.0f);
}

void test_threat_closing_rate_positive_inbound()
{
    ThreatBoard board;
    TrackManager tm;

    Aircraft ac(AircraftType::FIGHTER_ATTACK, 100.0f, 0.0f,
                30000.0f, 500.0f, 180.0f, false);
    tm.addTrack(&ac);

    board.update(tm);
    auto* threat = board.getThreat(0);
    ASSERT_NOT_NULL(threat);
    ASSERT_GT(threat->closingRate, 0.0f);
}

// ---------------------------------------------------------------------------
// Null/edge cases
// ---------------------------------------------------------------------------
void test_threat_get_invalid_rank()
{
    ThreatBoard board;
    ASSERT_NULL(board.getThreat(-1));
    ASSERT_NULL(board.getThreat(0));
    ASSERT_NULL(board.getThreat(10));
}

void test_threat_board_empty_format()
{
    ThreatBoard board;
    auto text = board.formatBoard();
    ASSERT_GT((int)text.size(), 0);  // Should still produce header text
}

// ============================================================================
void run_threat_board_tests()
{
    TEST_SUITE("Threat Board");

    test_threat_board_empty();
    test_threat_board_max_threats();
    test_threat_board_detects_hostile();
    test_threat_board_ignores_friendly();
    test_threat_board_limits_to_5();
    test_threat_board_closer_ranks_higher();
    test_threat_board_inbound_ranks_higher();
    test_threat_inbound_north_heading_south();
    test_threat_outbound_north_heading_north();
    test_threat_board_is_on_board();
    test_threat_board_not_on_board();
    test_threat_board_get_by_track_id();
    test_threat_board_format();
    test_threat_entry_format_line();
    test_threat_entry_format_card();
    test_threat_time_to_territory_inbound();
    test_threat_closing_rate_positive_inbound();
    test_threat_get_invalid_rank();
    test_threat_board_empty_format();

    TEST_SUITE_END();
}
