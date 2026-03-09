// ============================================================================
// Missile Battery Command — Interactive Test Harness (ncurses)
//
// AN/TSQ-73 Test Console: browse suites, enable/disable tests, run
// selectively, and review pass/fail results in a terminal UI.
//
// Test definitions are loaded from tests/test_manifest.json at startup.
// To add a new test:
//   1. Write the test function in a test_*.cpp file
//   2. Add a forward declaration below and a MAPTEST entry
//   3. Add the test name to test_manifest.json under the right suite
//
// Controls:
//   UP/DOWN    Navigate tests and suites
//   SPACE      Toggle test/suite enabled
//   ENTER      Expand/collapse suite
//   r          Run enabled tests
//   R          Run all tests (ignoring enable state)
//   a          Enable all tests
//   n          Disable all tests
//   f          Filter: show only failures
//   /          Search test by name
//   q          Quit
// ============================================================================

#include "TestFramework.h"
#include <ncurses.h>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <cstring>
#include <csignal>
#include <unistd.h>

// ============================================================================
// Forward declarations of ALL test functions
// ============================================================================

// --- Cocos2d-x Validation ---
void test_cocos2dx_build_flag();
void test_cocos2dx_headers_available();
void test_cocos2dx_director_handle();
void test_cocos2dx_node_creation();
void test_cocos2dx_scene_creation();
void test_cocos2dx_drawnode_creation();
void test_cocos2dx_label_creation();
void test_cocos2dx_installation_path();
void test_stub_mode_functional();

// --- GameTypes ---
void test_radar_constants();
void test_radar_range_nm();
void test_nm_km_conversion();
void test_radar_sweep_rate();
void test_patriot_constants();
void test_hawk_constants();
void test_scoring_constants();
void test_javelin_constants();
void test_hawk_stock_constants();
void test_territory_radius();
void test_polar_to_cartesian_north();
void test_polar_to_cartesian_east();
void test_polar_to_cartesian_south();
void test_polar_to_cartesian_scale();
void test_track_id_formatting();
void test_track_id_formatting_large();
void test_altitude_string_flight_level();
void test_altitude_string_low();
void test_classification_strings();
void test_classification_friendly();
void test_classification_pending();
void test_classification_unknown();

// --- Aircraft ---
void test_aircraft_construction();
void test_aircraft_initial_position();
void test_aircraft_initial_azimuth();
void test_aircraft_initial_altitude();
void test_aircraft_initial_alive();
void test_aircraft_initial_iff_pending();
void test_aircraft_initial_track_id();
void test_aircraft_friendly_flag();
void test_aircraft_hostile_flag();
void test_aircraft_moves_on_update();
void test_aircraft_range_decreases_inbound();
void test_aircraft_no_update_when_dead();
void test_aircraft_destroy();
void test_aircraft_territory_penetration();
void test_aircraft_territory_penetration_inside();
void test_friendly_no_territory_penetration();
void test_rcs_strategic_bomber();
void test_rcs_stealth_fighter();
void test_rcs_stealth_smaller_than_bomber();
void test_track_data_populated();
void test_track_data_altitude();
void test_track_data_iff_maps_to_classification();
void test_iff_set_hostile();
void test_iff_set_friendly();
void test_threat_score_hostile();
void test_threat_score_friendly_zero();
void test_threat_score_closer_is_higher();
void test_type_name_bomber();
void test_type_name_stealth();
void test_sweep_timer_initial();
void test_sweep_timer_increments();
void test_sweep_timer_reset();

// --- IFF System ---
void test_iff_construction();
void test_iff_interrogation_starts();
void test_iff_not_interrogating_unknown_track();
void test_iff_no_duplicate_interrogation();
void test_iff_requires_track_id();
void test_iff_null_aircraft_ignored();
void test_iff_pending_before_complete();
void test_iff_completes_after_interval();
void test_iff_hostile_identified();
void test_iff_friendly_identified();
void test_iff_skip_already_identified();
void test_iff_multiple_aircraft();
void test_iff_multiple_both_interrogating();
void test_iff_multiple_both_resolve();
void test_iff_multiple_friendly_resolved();
void test_iff_dead_aircraft_no_result();
void test_iff_error_rate_setter();

// --- Track Manager ---
void test_track_manager_empty();
void test_track_add();
void test_track_add_increments_id();
void test_track_add_sets_aircraft_id();
void test_track_add_null_rejected();
void test_track_count_after_add();
void test_track_get_by_id();
void test_track_get_nonexistent();
void test_track_get_aircraft_by_id();
void test_track_get_aircraft_is_same();
void test_track_get_all();
void test_track_remove();
void test_track_remove_nonexistent_no_crash();
void test_track_update_removes_dead();
void test_track_hostile_count();
void test_track_friendly_count();
void test_track_hostile_ids();
void test_track_reset();
void test_track_reset_resets_ids();

// --- Missile Battery ---
void test_patriot_construction();
void test_patriot_type();
void test_patriot_initial_ready();
void test_patriot_initial_missiles();
void test_patriot_no_track_assigned();
void test_hawk_construction();
void test_hawk_type();
void test_hawk_initial_missiles();
void test_can_engage_in_range();
void test_cannot_engage_out_of_range();
void test_cannot_engage_too_close();
void test_cannot_engage_too_high_for_hawk();
void test_cannot_engage_dead_target();
void test_cannot_engage_null();
void test_hawk_can_engage_low_alt();
void test_patriot_cannot_engage_very_low();
void test_engage_success();
void test_engage_consumes_missile();
void test_engage_changes_status();
void test_engage_sets_track_id();
void test_cannot_engage_while_engaged();
void test_engagement_produces_result();
void test_engagement_result_is_hit_or_miss();
void test_battery_returns_to_ready_after_engagement();
void test_clear_engagement_result();
void test_reload_after_all_missiles_spent();
void test_reload_completes();
void test_battery_data_populated();
void test_battery_data_range();
void test_battery_data_missiles();
void test_abort_tracking();
void test_javelin_construction();
void test_javelin_type();
void test_javelin_initial_missiles();
void test_javelin_can_engage_close_target();
void test_javelin_cannot_engage_high_alt();
void test_javelin_ir_seeker_no_missile_tracking();
void test_hawk_missile_stock();
void test_hawk_loaders();
void test_patriot_tracking_radar_type();
void test_hawk_tracking_radar_type();
void test_relocate_sets_offline();
void test_relocate_is_relocating();
void test_relocate_cannot_while_engaged();
void test_relocate_cannot_while_already_relocating();
void test_relocate_completes_patriot();
void test_relocate_completes_javelin();
void test_relocate_updates_position();
void test_relocate_clears_target();
void test_engagement_count_increments();
void test_hit_miss_count_adds_up();

// --- Aircraft Generator ---
void test_generator_construction();
void test_generator_init();
void test_generator_set_level();
void test_generator_no_immediate_spawn();
void test_generator_spawns_eventually();
void test_generator_respects_max_count();
void test_generator_spawned_aircraft_valid_range();
void test_generator_spawned_aircraft_valid_range_upper();
void test_generator_spawned_aircraft_valid_azimuth();
void test_generator_spawned_aircraft_azimuth_upper();
void test_generator_spawned_aircraft_positive_speed();
void test_generator_spawned_aircraft_positive_altitude();
void test_generator_spawned_aircraft_alive();
void test_generator_reset();
void test_generator_level1_max();
void test_generator_level5_allows_more();

// --- Fire Control System ---
void test_fcs_construction();
void test_fcs_has_3_patriots();
void test_fcs_has_3_hawks();
void test_fcs_get_patriot1();
void test_fcs_get_hawk2();
void test_fcs_get_nonexistent();
void test_fcs_patriot_designations();
void test_fcs_patriot2_exists();
void test_fcs_patriot3_exists();
void test_fcs_hawk1_exists();
void test_fcs_hawk3_exists();
void test_fcs_all_battery_data();
void test_fcs_all_batteries_ready();
void test_fcs_available_batteries_in_range();
void test_fcs_no_batteries_for_nonexistent_track();
void test_fcs_reset();
void test_fcs_update_no_crash();
void test_fcs_no_results_initially();
void test_fcs_has_3_javelins();
void test_fcs_javelin1_exists();
void test_fcs_javelin2_exists();
void test_fcs_javelin3_exists();
void test_hawk_total_stock();
void test_hawk_has_3_loaders();
void test_patriot_tracking_radar();
void test_hawk_tracking_radar();
void test_javelin_ir_seeker();

// --- Game Config ---
void test_config_singleton();
void test_config_default_level();
void test_config_set_level();
void test_config_clamp_level_low();
void test_config_clamp_level_high();
void test_config_level1_max_aircraft();
void test_config_level1_no_stealth();
void test_config_level1_no_iff_errors();
void test_config_level1_speed_slow();
void test_config_level1_friendly_ratio();
void test_config_level5_max_aircraft();
void test_config_level5_stealth_enabled();
void test_config_level5_iff_errors();
void test_config_level5_speed_fast();
void test_config_level5_fewer_friendlies();
void test_config_max_aircraft_increases();
void test_config_speed_increases();
void test_config_friendly_ratio_decreases();
void test_config_spawn_interval_positive();
void test_config_score_strategic_bomber();
void test_config_score_fighter();
void test_config_score_recon_drone();
void test_config_score_higher_at_higher_level();

// --- Airspace Manager ---
void test_airspace_empty();
void test_airspace_add_zone();
void test_airspace_remove_zone();
void test_airspace_get_zone();
void test_airspace_get_nonexistent();
void test_airspace_activate_deactivate();
void test_airspace_nfz_contains();
void test_airspace_nfz_outside();
void test_airspace_nfz_altitude_above();
void test_airspace_corridor_match();
void test_airspace_corridor_wrong_heading();
void test_airspace_default_init();
void test_airspace_zone_type_strings();
void test_airspace_azimuth_wraparound();

// --- Data Cards ---
void test_patriot_equipment_card();
void test_patriot_card_radar();
void test_patriot_card_guidance();
void test_hawk_equipment_card();
void test_hawk_card_guidance();
void test_javelin_equipment_card();
void test_javelin_card_guidance();
void test_all_equipment_cards();
void test_patriot_troop_strength();
void test_hawk_troop_strength();
void test_javelin_troop_strength();
void test_equipment_card_format();
void test_troop_card_format();
void test_intel_card_format();

// --- Threat Board ---
void test_threat_board_empty();
void test_threat_board_max_threats();
void test_threat_board_detects_hostile();
void test_threat_board_ignores_friendly();
void test_threat_board_limits_to_5();
void test_threat_board_closer_ranks_higher();
void test_threat_board_inbound_ranks_higher();
void test_threat_inbound_north_heading_south();
void test_threat_outbound_north_heading_north();
void test_threat_board_is_on_board();
void test_threat_board_not_on_board();
void test_threat_board_get_by_track_id();
void test_threat_board_format();
void test_threat_entry_format_line();
void test_threat_entry_format_card();
void test_threat_time_to_territory_inbound();
void test_threat_closing_rate_positive_inbound();
void test_threat_get_invalid_rank();
void test_threat_board_empty_format();

// --- Battalion HQ ---
void test_hq_construction();
void test_hq_init();
void test_hq_init_position();
void test_hq_relocate_starts();
void test_hq_relocate_goes_offline();
void test_hq_relocate_status();
void test_hq_cannot_relocate_twice();
void test_hq_relocate_completes();
void test_hq_relocate_updates_position();
void test_hq_relocate_time();
void test_hq_setting_up_phase();
void test_hq_data();
void test_hq_data_format();
void test_hq_status_string();
void test_hq_relocating_status_string();
void test_hq_personnel_count();

// ============================================================================
// Function pointer map — maps test name string to callable
//
// When adding a new test, add one MAPTEST line here AND the forward
// declaration above. Then add the test name to test_manifest.json.
// ============================================================================

using TestFunc = std::function<void()>;
using TestMap = std::map<std::string, TestFunc>;

#define MAPTEST(func) { #func, func }

static TestMap buildTestMap()
{
    return {
        // Cocos2d-x Validation
        MAPTEST(test_cocos2dx_build_flag),
        MAPTEST(test_cocos2dx_headers_available),
        MAPTEST(test_cocos2dx_director_handle),
        MAPTEST(test_cocos2dx_node_creation),
        MAPTEST(test_cocos2dx_scene_creation),
        MAPTEST(test_cocos2dx_drawnode_creation),
        MAPTEST(test_cocos2dx_label_creation),
        MAPTEST(test_cocos2dx_installation_path),
        MAPTEST(test_stub_mode_functional),
        // GameTypes
        MAPTEST(test_radar_constants),
        MAPTEST(test_radar_range_nm),
        MAPTEST(test_nm_km_conversion),
        MAPTEST(test_radar_sweep_rate),
        MAPTEST(test_patriot_constants),
        MAPTEST(test_hawk_constants),
        MAPTEST(test_scoring_constants),
        MAPTEST(test_javelin_constants),
        MAPTEST(test_hawk_stock_constants),
        MAPTEST(test_territory_radius),
        MAPTEST(test_polar_to_cartesian_north),
        MAPTEST(test_polar_to_cartesian_east),
        MAPTEST(test_polar_to_cartesian_south),
        MAPTEST(test_polar_to_cartesian_scale),
        MAPTEST(test_track_id_formatting),
        MAPTEST(test_track_id_formatting_large),
        MAPTEST(test_altitude_string_flight_level),
        MAPTEST(test_altitude_string_low),
        MAPTEST(test_classification_strings),
        MAPTEST(test_classification_friendly),
        MAPTEST(test_classification_pending),
        MAPTEST(test_classification_unknown),
        // Aircraft
        MAPTEST(test_aircraft_construction),
        MAPTEST(test_aircraft_initial_position),
        MAPTEST(test_aircraft_initial_azimuth),
        MAPTEST(test_aircraft_initial_altitude),
        MAPTEST(test_aircraft_initial_alive),
        MAPTEST(test_aircraft_initial_iff_pending),
        MAPTEST(test_aircraft_initial_track_id),
        MAPTEST(test_aircraft_friendly_flag),
        MAPTEST(test_aircraft_hostile_flag),
        MAPTEST(test_aircraft_moves_on_update),
        MAPTEST(test_aircraft_range_decreases_inbound),
        MAPTEST(test_aircraft_no_update_when_dead),
        MAPTEST(test_aircraft_destroy),
        MAPTEST(test_aircraft_territory_penetration),
        MAPTEST(test_aircraft_territory_penetration_inside),
        MAPTEST(test_friendly_no_territory_penetration),
        MAPTEST(test_rcs_strategic_bomber),
        MAPTEST(test_rcs_stealth_fighter),
        MAPTEST(test_rcs_stealth_smaller_than_bomber),
        MAPTEST(test_track_data_populated),
        MAPTEST(test_track_data_altitude),
        MAPTEST(test_track_data_iff_maps_to_classification),
        MAPTEST(test_iff_set_hostile),
        MAPTEST(test_iff_set_friendly),
        MAPTEST(test_threat_score_hostile),
        MAPTEST(test_threat_score_friendly_zero),
        MAPTEST(test_threat_score_closer_is_higher),
        MAPTEST(test_type_name_bomber),
        MAPTEST(test_type_name_stealth),
        MAPTEST(test_sweep_timer_initial),
        MAPTEST(test_sweep_timer_increments),
        MAPTEST(test_sweep_timer_reset),
        // IFF System
        MAPTEST(test_iff_construction),
        MAPTEST(test_iff_interrogation_starts),
        MAPTEST(test_iff_not_interrogating_unknown_track),
        MAPTEST(test_iff_no_duplicate_interrogation),
        MAPTEST(test_iff_requires_track_id),
        MAPTEST(test_iff_null_aircraft_ignored),
        MAPTEST(test_iff_pending_before_complete),
        MAPTEST(test_iff_completes_after_interval),
        MAPTEST(test_iff_hostile_identified),
        MAPTEST(test_iff_friendly_identified),
        MAPTEST(test_iff_skip_already_identified),
        MAPTEST(test_iff_multiple_aircraft),
        MAPTEST(test_iff_multiple_both_interrogating),
        MAPTEST(test_iff_multiple_both_resolve),
        MAPTEST(test_iff_multiple_friendly_resolved),
        MAPTEST(test_iff_dead_aircraft_no_result),
        MAPTEST(test_iff_error_rate_setter),
        // Track Manager
        MAPTEST(test_track_manager_empty),
        MAPTEST(test_track_add),
        MAPTEST(test_track_add_increments_id),
        MAPTEST(test_track_add_sets_aircraft_id),
        MAPTEST(test_track_add_null_rejected),
        MAPTEST(test_track_count_after_add),
        MAPTEST(test_track_get_by_id),
        MAPTEST(test_track_get_nonexistent),
        MAPTEST(test_track_get_aircraft_by_id),
        MAPTEST(test_track_get_aircraft_is_same),
        MAPTEST(test_track_get_all),
        MAPTEST(test_track_remove),
        MAPTEST(test_track_remove_nonexistent_no_crash),
        MAPTEST(test_track_update_removes_dead),
        MAPTEST(test_track_hostile_count),
        MAPTEST(test_track_friendly_count),
        MAPTEST(test_track_hostile_ids),
        MAPTEST(test_track_reset),
        MAPTEST(test_track_reset_resets_ids),
        // Missile Battery
        MAPTEST(test_patriot_construction),
        MAPTEST(test_patriot_type),
        MAPTEST(test_patriot_initial_ready),
        MAPTEST(test_patriot_initial_missiles),
        MAPTEST(test_patriot_no_track_assigned),
        MAPTEST(test_hawk_construction),
        MAPTEST(test_hawk_type),
        MAPTEST(test_hawk_initial_missiles),
        MAPTEST(test_can_engage_in_range),
        MAPTEST(test_cannot_engage_out_of_range),
        MAPTEST(test_cannot_engage_too_close),
        MAPTEST(test_cannot_engage_too_high_for_hawk),
        MAPTEST(test_cannot_engage_dead_target),
        MAPTEST(test_cannot_engage_null),
        MAPTEST(test_hawk_can_engage_low_alt),
        MAPTEST(test_patriot_cannot_engage_very_low),
        MAPTEST(test_engage_success),
        MAPTEST(test_engage_consumes_missile),
        MAPTEST(test_engage_changes_status),
        MAPTEST(test_engage_sets_track_id),
        MAPTEST(test_cannot_engage_while_engaged),
        MAPTEST(test_engagement_produces_result),
        MAPTEST(test_engagement_result_is_hit_or_miss),
        MAPTEST(test_battery_returns_to_ready_after_engagement),
        MAPTEST(test_clear_engagement_result),
        MAPTEST(test_reload_after_all_missiles_spent),
        MAPTEST(test_reload_completes),
        MAPTEST(test_battery_data_populated),
        MAPTEST(test_battery_data_range),
        MAPTEST(test_battery_data_missiles),
        MAPTEST(test_abort_tracking),
        MAPTEST(test_javelin_construction),
        MAPTEST(test_javelin_type),
        MAPTEST(test_javelin_initial_missiles),
        MAPTEST(test_javelin_can_engage_close_target),
        MAPTEST(test_javelin_cannot_engage_high_alt),
        MAPTEST(test_javelin_ir_seeker_no_missile_tracking),
        MAPTEST(test_hawk_missile_stock),
        MAPTEST(test_hawk_loaders),
        MAPTEST(test_patriot_tracking_radar_type),
        MAPTEST(test_hawk_tracking_radar_type),
        MAPTEST(test_relocate_sets_offline),
        MAPTEST(test_relocate_is_relocating),
        MAPTEST(test_relocate_cannot_while_engaged),
        MAPTEST(test_relocate_cannot_while_already_relocating),
        MAPTEST(test_relocate_completes_patriot),
        MAPTEST(test_relocate_completes_javelin),
        MAPTEST(test_relocate_updates_position),
        MAPTEST(test_relocate_clears_target),
        MAPTEST(test_engagement_count_increments),
        MAPTEST(test_hit_miss_count_adds_up),
        // Aircraft Generator
        MAPTEST(test_generator_construction),
        MAPTEST(test_generator_init),
        MAPTEST(test_generator_set_level),
        MAPTEST(test_generator_no_immediate_spawn),
        MAPTEST(test_generator_spawns_eventually),
        MAPTEST(test_generator_respects_max_count),
        MAPTEST(test_generator_spawned_aircraft_valid_range),
        MAPTEST(test_generator_spawned_aircraft_valid_range_upper),
        MAPTEST(test_generator_spawned_aircraft_valid_azimuth),
        MAPTEST(test_generator_spawned_aircraft_azimuth_upper),
        MAPTEST(test_generator_spawned_aircraft_positive_speed),
        MAPTEST(test_generator_spawned_aircraft_positive_altitude),
        MAPTEST(test_generator_spawned_aircraft_alive),
        MAPTEST(test_generator_reset),
        MAPTEST(test_generator_level1_max),
        MAPTEST(test_generator_level5_allows_more),
        // Fire Control System
        MAPTEST(test_fcs_construction),
        MAPTEST(test_fcs_has_3_patriots),
        MAPTEST(test_fcs_has_3_hawks),
        MAPTEST(test_fcs_get_patriot1),
        MAPTEST(test_fcs_get_hawk2),
        MAPTEST(test_fcs_get_nonexistent),
        MAPTEST(test_fcs_patriot_designations),
        MAPTEST(test_fcs_patriot2_exists),
        MAPTEST(test_fcs_patriot3_exists),
        MAPTEST(test_fcs_hawk1_exists),
        MAPTEST(test_fcs_hawk3_exists),
        MAPTEST(test_fcs_all_battery_data),
        MAPTEST(test_fcs_all_batteries_ready),
        MAPTEST(test_fcs_available_batteries_in_range),
        MAPTEST(test_fcs_no_batteries_for_nonexistent_track),
        MAPTEST(test_fcs_reset),
        MAPTEST(test_fcs_update_no_crash),
        MAPTEST(test_fcs_no_results_initially),
        MAPTEST(test_fcs_has_3_javelins),
        MAPTEST(test_fcs_javelin1_exists),
        MAPTEST(test_fcs_javelin2_exists),
        MAPTEST(test_fcs_javelin3_exists),
        MAPTEST(test_hawk_total_stock),
        MAPTEST(test_hawk_has_3_loaders),
        MAPTEST(test_patriot_tracking_radar),
        MAPTEST(test_hawk_tracking_radar),
        MAPTEST(test_javelin_ir_seeker),
        // Game Config
        MAPTEST(test_config_singleton),
        MAPTEST(test_config_default_level),
        MAPTEST(test_config_set_level),
        MAPTEST(test_config_clamp_level_low),
        MAPTEST(test_config_clamp_level_high),
        MAPTEST(test_config_level1_max_aircraft),
        MAPTEST(test_config_level1_no_stealth),
        MAPTEST(test_config_level1_no_iff_errors),
        MAPTEST(test_config_level1_speed_slow),
        MAPTEST(test_config_level1_friendly_ratio),
        MAPTEST(test_config_level5_max_aircraft),
        MAPTEST(test_config_level5_stealth_enabled),
        MAPTEST(test_config_level5_iff_errors),
        MAPTEST(test_config_level5_speed_fast),
        MAPTEST(test_config_level5_fewer_friendlies),
        MAPTEST(test_config_max_aircraft_increases),
        MAPTEST(test_config_speed_increases),
        MAPTEST(test_config_friendly_ratio_decreases),
        MAPTEST(test_config_spawn_interval_positive),
        MAPTEST(test_config_score_strategic_bomber),
        MAPTEST(test_config_score_fighter),
        MAPTEST(test_config_score_recon_drone),
        MAPTEST(test_config_score_higher_at_higher_level),
        // Airspace Manager
        MAPTEST(test_airspace_empty),
        MAPTEST(test_airspace_add_zone),
        MAPTEST(test_airspace_remove_zone),
        MAPTEST(test_airspace_get_zone),
        MAPTEST(test_airspace_get_nonexistent),
        MAPTEST(test_airspace_activate_deactivate),
        MAPTEST(test_airspace_nfz_contains),
        MAPTEST(test_airspace_nfz_outside),
        MAPTEST(test_airspace_nfz_altitude_above),
        MAPTEST(test_airspace_corridor_match),
        MAPTEST(test_airspace_corridor_wrong_heading),
        MAPTEST(test_airspace_default_init),
        MAPTEST(test_airspace_zone_type_strings),
        MAPTEST(test_airspace_azimuth_wraparound),
        // Data Cards
        MAPTEST(test_patriot_equipment_card),
        MAPTEST(test_patriot_card_radar),
        MAPTEST(test_patriot_card_guidance),
        MAPTEST(test_hawk_equipment_card),
        MAPTEST(test_hawk_card_guidance),
        MAPTEST(test_javelin_equipment_card),
        MAPTEST(test_javelin_card_guidance),
        MAPTEST(test_all_equipment_cards),
        MAPTEST(test_patriot_troop_strength),
        MAPTEST(test_hawk_troop_strength),
        MAPTEST(test_javelin_troop_strength),
        MAPTEST(test_equipment_card_format),
        MAPTEST(test_troop_card_format),
        MAPTEST(test_intel_card_format),
        // Threat Board
        MAPTEST(test_threat_board_empty),
        MAPTEST(test_threat_board_max_threats),
        MAPTEST(test_threat_board_detects_hostile),
        MAPTEST(test_threat_board_ignores_friendly),
        MAPTEST(test_threat_board_limits_to_5),
        MAPTEST(test_threat_board_closer_ranks_higher),
        MAPTEST(test_threat_board_inbound_ranks_higher),
        MAPTEST(test_threat_inbound_north_heading_south),
        MAPTEST(test_threat_outbound_north_heading_north),
        MAPTEST(test_threat_board_is_on_board),
        MAPTEST(test_threat_board_not_on_board),
        MAPTEST(test_threat_board_get_by_track_id),
        MAPTEST(test_threat_board_format),
        MAPTEST(test_threat_entry_format_line),
        MAPTEST(test_threat_entry_format_card),
        MAPTEST(test_threat_time_to_territory_inbound),
        MAPTEST(test_threat_closing_rate_positive_inbound),
        MAPTEST(test_threat_get_invalid_rank),
        MAPTEST(test_threat_board_empty_format),
        // Battalion HQ
        MAPTEST(test_hq_construction),
        MAPTEST(test_hq_init),
        MAPTEST(test_hq_init_position),
        MAPTEST(test_hq_relocate_starts),
        MAPTEST(test_hq_relocate_goes_offline),
        MAPTEST(test_hq_relocate_status),
        MAPTEST(test_hq_cannot_relocate_twice),
        MAPTEST(test_hq_relocate_completes),
        MAPTEST(test_hq_relocate_updates_position),
        MAPTEST(test_hq_relocate_time),
        MAPTEST(test_hq_setting_up_phase),
        MAPTEST(test_hq_data),
        MAPTEST(test_hq_data_format),
        MAPTEST(test_hq_status_string),
        MAPTEST(test_hq_relocating_status_string),
        MAPTEST(test_hq_personnel_count),
    };
}

#undef MAPTEST

// ============================================================================
// Minimal JSON manifest parser
//
// Reads test_manifest.json to determine suite groupings. Format:
// { "suites": [ { "name": "...", "tests": ["...", ...] }, ... ] }
//
// No external dependencies — hand-rolled parser for this specific schema.
// ============================================================================

struct ManifestSuite {
    std::string name;
    std::vector<std::string> tests;
};

static std::string trimQuotes(const std::string& s)
{
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
        return s.substr(1, s.size() - 2);
    return s;
}

static std::vector<ManifestSuite> loadManifest(const std::string& path)
{
    std::vector<ManifestSuite> result;

    std::ifstream f(path);
    if (!f.is_open()) return result;

    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());

    // Simple state machine parser for our specific JSON structure
    // We look for "name": "..." and "tests": ["...", ...]
    size_t pos = 0;
    auto skipWs = [&]() {
        while (pos < content.size() && (content[pos] == ' ' || content[pos] == '\t' ||
               content[pos] == '\n' || content[pos] == '\r')) pos++;
    };

    auto readString = [&]() -> std::string {
        skipWs();
        if (pos >= content.size() || content[pos] != '"') return "";
        pos++; // skip opening quote
        std::string s;
        while (pos < content.size() && content[pos] != '"') {
            if (content[pos] == '\\' && pos + 1 < content.size()) {
                pos++;
                s += content[pos];
            } else {
                s += content[pos];
            }
            pos++;
        }
        if (pos < content.size()) pos++; // skip closing quote
        return s;
    };

    // Find "suites" array
    auto findKey = [&](const std::string& key) -> bool {
        std::string needle = "\"" + key + "\"";
        size_t found = content.find(needle, pos);
        if (found == std::string::npos) return false;
        pos = found + needle.size();
        skipWs();
        if (pos < content.size() && content[pos] == ':') pos++;
        skipWs();
        return true;
    };

    if (!findKey("suites")) return result;
    if (pos >= content.size() || content[pos] != '[') return result;
    pos++; // skip [

    // Read suite objects
    while (pos < content.size()) {
        skipWs();
        if (content[pos] == ']') break;
        if (content[pos] == ',') { pos++; continue; }
        if (content[pos] != '{') { pos++; continue; }
        pos++; // skip {

        ManifestSuite suite;
        while (pos < content.size() && content[pos] != '}') {
            skipWs();
            if (content[pos] == ',') { pos++; continue; }
            std::string key = readString();
            skipWs();
            if (pos < content.size() && content[pos] == ':') pos++;
            skipWs();

            if (key == "name") {
                suite.name = readString();
            } else if (key == "tests") {
                if (pos < content.size() && content[pos] == '[') {
                    pos++; // skip [
                    while (pos < content.size() && content[pos] != ']') {
                        skipWs();
                        if (content[pos] == ',') { pos++; continue; }
                        if (content[pos] == '"') {
                            suite.tests.push_back(readString());
                        } else {
                            pos++;
                        }
                    }
                    if (pos < content.size()) pos++; // skip ]
                }
            } else {
                // Skip unknown value
                if (pos < content.size() && content[pos] == '"') readString();
            }
        }
        if (pos < content.size()) pos++; // skip }

        if (!suite.name.empty()) {
            result.push_back(suite);
        }
    }

    return result;
}

static std::string findManifestPath(const char* argv0)
{
    // Try several locations relative to the executable
    std::vector<std::string> searchPaths = {
        "tests/test_manifest.json",
        "../tests/test_manifest.json",
        "../../tests/test_manifest.json",
    };

    // Also try relative to argv0
    if (argv0) {
        std::string dir(argv0);
        size_t slash = dir.rfind('/');
        if (slash != std::string::npos) {
            dir = dir.substr(0, slash + 1);
            searchPaths.push_back(dir + "tests/test_manifest.json");
            searchPaths.push_back(dir + "../tests/test_manifest.json");
        }
    }

    for (const auto& path : searchPaths) {
        std::ifstream f(path);
        if (f.is_open()) return path;
    }
    return "";
}

// ============================================================================
// Harness data structures
// ============================================================================

enum class TestStatus { NOT_RUN, PASSED, FAILED };

struct HarnessTest {
    std::string name;
    std::string suite;
    TestFunc func;
    bool enabled;
    TestStatus status;
    std::string failMessage;
};

struct HarnessSuite {
    std::string name;
    bool expanded;
    bool enabled;
    int passed;
    int failed;
    int total;
    int firstTestIndex;
};

// Global harness state
static std::vector<HarnessTest> g_tests;
static std::vector<HarnessSuite> g_suites;
static std::vector<int> g_displayRows;
static int g_cursor = 0;
static int g_scrollOffset = 0;
static int g_totalPassed = 0;
static int g_totalFailed = 0;
static int g_totalRun = 0;
static bool g_showFailuresOnly = false;
static std::string g_searchFilter;
static std::string g_lastRunMessage;
static std::string g_manifestPath;

// ============================================================================
// Build display list
// ============================================================================
static void buildDisplayRows()
{
    g_displayRows.clear();

    for (int s = 0; s < (int)g_suites.size(); s++) {
        bool suiteHasMatch = g_searchFilter.empty();
        if (!suiteHasMatch) {
            for (int t = g_suites[s].firstTestIndex;
                 t < g_suites[s].firstTestIndex + g_suites[s].total; t++) {
                if (g_tests[t].name.find(g_searchFilter) != std::string::npos ||
                    g_tests[t].suite.find(g_searchFilter) != std::string::npos) {
                    suiteHasMatch = true;
                    break;
                }
            }
        }
        if (!suiteHasMatch) continue;
        if (g_showFailuresOnly && g_suites[s].failed == 0 && g_suites[s].passed > 0) continue;

        g_displayRows.push_back(-(s + 1));

        if (g_suites[s].expanded) {
            for (int t = g_suites[s].firstTestIndex;
                 t < g_suites[s].firstTestIndex + g_suites[s].total; t++) {
                if (!g_searchFilter.empty() &&
                    g_tests[t].name.find(g_searchFilter) == std::string::npos) continue;
                if (g_showFailuresOnly && g_tests[t].status != TestStatus::FAILED) continue;
                g_displayRows.push_back(t);
            }
        }
    }
}

// ============================================================================
// Initialize harness from JSON manifest + function map
// ============================================================================
static bool initHarness(const std::string& manifestPath, const TestMap& testMap)
{
    auto manifest = loadManifest(manifestPath);
    if (manifest.empty()) return false;

    int testIndex = 0;
    int skipped = 0;

    for (const auto& ms : manifest) {
        HarnessSuite hs;
        hs.name = ms.name;
        hs.expanded = false;
        hs.enabled = true;
        hs.passed = 0;
        hs.failed = 0;
        hs.firstTestIndex = testIndex;

        int suiteCount = 0;
        for (const auto& testName : ms.tests) {
            auto it = testMap.find(testName);
            if (it == testMap.end()) {
                skipped++;
                continue;
            }

            HarnessTest ht;
            ht.name = testName;
            ht.suite = ms.name;
            ht.func = it->second;
            ht.enabled = true;
            ht.status = TestStatus::NOT_RUN;
            g_tests.push_back(ht);
            testIndex++;
            suiteCount++;
        }

        hs.total = suiteCount;
        if (suiteCount > 0) {
            g_suites.push_back(hs);
        }
    }

    if (skipped > 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "WARNING: %d tests in manifest not found in binary", skipped);
        g_lastRunMessage = msg;
    }

    buildDisplayRows();
    return !g_suites.empty();
}

// ============================================================================
// Run tests
// ============================================================================
static void runTests(bool runAll)
{
    auto& runner = mbc_test::TestRunner::instance();

    g_totalPassed = 0;
    g_totalFailed = 0;
    g_totalRun = 0;

    for (auto& s : g_suites) { s.passed = 0; s.failed = 0; }
    for (auto& t : g_tests) { t.status = TestStatus::NOT_RUN; t.failMessage.clear(); }

    FILE* devnull = fopen("/dev/null", "w");
    int savedStdout = dup(fileno(stdout));
    if (devnull) {
        dup2(fileno(devnull), fileno(stdout));
        fclose(devnull);
    }

    for (int s = 0; s < (int)g_suites.size(); s++) {
        for (int t = g_suites[s].firstTestIndex;
             t < g_suites[s].firstTestIndex + g_suites[s].total; t++) {
            if (!runAll && !g_tests[t].enabled) continue;

            runner.reset();
            runner.setQuiet(true);
            runner.beginSuite(g_suites[s].name);
            g_tests[t].func();
            runner.endSuite();

            auto& suites = runner.getSuites();
            if (!suites.empty()) {
                for (const auto& result : suites[0].results) {
                    if (result.passed) {
                        g_tests[t].status = TestStatus::PASSED;
                        g_suites[s].passed++;
                        g_totalPassed++;
                    } else {
                        g_tests[t].status = TestStatus::FAILED;
                        g_tests[t].failMessage = result.message;
                        g_suites[s].failed++;
                        g_totalFailed++;
                    }
                    g_totalRun++;
                }
            }
        }
    }

    dup2(savedStdout, fileno(stdout));
    close(savedStdout);
    runner.setQuiet(false);

    char msg[128];
    snprintf(msg, sizeof(msg), "RAN %d TESTS: %d PASSED, %d FAILED",
             g_totalRun, g_totalPassed, g_totalFailed);
    g_lastRunMessage = msg;
    buildDisplayRows();
}

// ============================================================================
// ncurses drawing
// ============================================================================

#define CP_NORMAL    1
#define CP_PASS      2
#define CP_FAIL      3
#define CP_HEADER    4
#define CP_SELECTED  5
#define CP_DISABLED  6
#define CP_STATUS    7
#define CP_SEARCH    8

static void initColors()
{
    start_color();
    use_default_colors();
    init_pair(CP_NORMAL,   COLOR_GREEN,  -1);
    init_pair(CP_PASS,     COLOR_GREEN,  -1);
    init_pair(CP_FAIL,     COLOR_RED,    -1);
    init_pair(CP_HEADER,   COLOR_CYAN,   -1);
    init_pair(CP_SELECTED, COLOR_BLACK,  COLOR_GREEN);
    init_pair(CP_DISABLED, COLOR_YELLOW, -1);
    init_pair(CP_STATUS,   COLOR_YELLOW, -1);
    init_pair(CP_SEARCH,   COLOR_CYAN,   -1);
}

static void drawScreen()
{
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    clear();

    // Header
    attron(A_BOLD | COLOR_PAIR(CP_HEADER));
    mvprintw(0, 0, "%-*s", maxX, " MISSILE BATTERY COMMAND -- TEST HARNESS (AN/TSQ-73)");
    attroff(A_BOLD | COLOR_PAIR(CP_HEADER));

    // Status bar
    attron(COLOR_PAIR(CP_STATUS));
    int totalTests = (int)g_tests.size();
    int enabledCount = 0;
    for (const auto& t : g_tests) { if (t.enabled) enabledCount++; }
    char statusBar[256];
    snprintf(statusBar, sizeof(statusBar),
        " %d suites | %d tests (%d enabled) | %s | manifest: %s",
        (int)g_suites.size(), totalTests, enabledCount,
        g_lastRunMessage.empty() ? "not run" : g_lastRunMessage.c_str(),
        g_manifestPath.c_str());
    mvprintw(1, 0, "%-*s", maxX, statusBar);
    attroff(COLOR_PAIR(CP_STATUS));

    int headerLines = 3;
    if (!g_searchFilter.empty()) {
        attron(COLOR_PAIR(CP_SEARCH));
        mvprintw(2, 0, " FILTER: %s", g_searchFilter.c_str());
        attroff(COLOR_PAIR(CP_SEARCH));
        headerLines = 4;
    }

    int footerLines = 3;
    int listHeight = maxY - headerLines - footerLines;

    if (g_cursor < g_scrollOffset) g_scrollOffset = g_cursor;
    if (g_cursor >= g_scrollOffset + listHeight) g_scrollOffset = g_cursor - listHeight + 1;
    if (g_scrollOffset < 0) g_scrollOffset = 0;

    for (int row = 0; row < listHeight && (row + g_scrollOffset) < (int)g_displayRows.size(); row++) {
        int y = headerLines + row;
        int idx = g_displayRows[row + g_scrollOffset];
        bool isSelected = (row + g_scrollOffset == g_cursor);

        if (isSelected) attron(A_REVERSE);

        if (idx < 0) {
            int si = -(idx + 1);
            auto& suite = g_suites[si];
            const char* arrow = suite.expanded ? "[-]" : "[+]";
            const char* enableMark = suite.enabled ? "*" : " ";

            char line[256];
            if (suite.passed + suite.failed > 0) {
                const char* resultStr = (suite.failed == 0) ? "PASS" : "FAIL";
                int cp = (suite.failed == 0) ? CP_PASS : CP_FAIL;
                snprintf(line, sizeof(line), " %s %s %-28s %d/%d  ",
                         arrow, enableMark, suite.name.c_str(),
                         suite.passed, suite.passed + suite.failed);
                attron(A_BOLD);
                if (!isSelected) attron(COLOR_PAIR(cp));
                mvprintw(y, 0, "%-*s", maxX - 6, line);
                printw("%s", resultStr);
                if (!isSelected) attroff(COLOR_PAIR(cp));
                attroff(A_BOLD);
            } else {
                snprintf(line, sizeof(line), " %s %s %-28s ---",
                         arrow, enableMark, suite.name.c_str());
                attron(A_BOLD);
                if (!isSelected && !suite.enabled) attron(COLOR_PAIR(CP_DISABLED));
                else if (!isSelected) attron(COLOR_PAIR(CP_NORMAL));
                mvprintw(y, 0, "%-*s", maxX, line);
                if (!isSelected && !suite.enabled) attroff(COLOR_PAIR(CP_DISABLED));
                else if (!isSelected) attroff(COLOR_PAIR(CP_NORMAL));
                attroff(A_BOLD);
            }
        } else {
            auto& test = g_tests[idx];
            const char* statusMark;
            int cp = CP_NORMAL;
            switch (test.status) {
                case TestStatus::PASSED:  statusMark = "PASS"; cp = CP_PASS; break;
                case TestStatus::FAILED:  statusMark = "FAIL"; cp = CP_FAIL; break;
                case TestStatus::NOT_RUN: statusMark = " -- "; cp = CP_NORMAL; break;
            }
            const char* enableMark = test.enabled ? "[x]" : "[ ]";
            char line[256];
            snprintf(line, sizeof(line), "     %s %-42s", enableMark, test.name.c_str());
            if (!isSelected) attron(COLOR_PAIR(test.enabled ? cp : CP_DISABLED));
            mvprintw(y, 0, "%-*s", maxX - 6, line);
            printw("%s", statusMark);
            if (!isSelected) attroff(COLOR_PAIR(test.enabled ? cp : CP_DISABLED));
        }

        if (isSelected) attroff(A_REVERSE);
    }

    if ((int)g_displayRows.size() > listHeight) {
        int scrollPercent = g_scrollOffset * 100 / std::max(1, (int)g_displayRows.size() - listHeight);
        attron(COLOR_PAIR(CP_STATUS));
        mvprintw(headerLines + listHeight - 1, maxX - 8, " %3d%% ", scrollPercent);
        attroff(COLOR_PAIR(CP_STATUS));
    }

    // Detail panel for failed tests
    int detailY = maxY - footerLines;
    if (g_cursor >= 0 && g_cursor < (int)g_displayRows.size()) {
        int idx = g_displayRows[g_cursor];
        if (idx >= 0 && g_tests[idx].status == TestStatus::FAILED) {
            attron(COLOR_PAIR(CP_FAIL));
            std::string msg = g_tests[idx].failMessage;
            if ((int)msg.size() > maxX - 2) msg = msg.substr(0, maxX - 5) + "...";
            mvprintw(detailY, 0, "%-*s", maxX, (" ERR: " + msg).c_str());
            attroff(COLOR_PAIR(CP_FAIL));
        }
    }

    // Footer
    attron(A_BOLD | COLOR_PAIR(CP_HEADER));
    mvprintw(maxY - 2, 0, "%-*s", maxX,
        " r=Run Enabled  R=Run All  SPACE=Toggle  ENTER=Expand  a=All On  n=All Off");
    mvprintw(maxY - 1, 0, "%-*s", maxX,
        " f=Failures Only  /=Search  ESC=Clear Filter  q=Quit");
    attroff(A_BOLD | COLOR_PAIR(CP_HEADER));

    refresh();
}

// ============================================================================
// Input handling
// ============================================================================

static void handleSearch()
{
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    char searchBuf[128] = {};
    int pos = 0;

    attron(COLOR_PAIR(CP_SEARCH) | A_BOLD);
    mvprintw(maxY - 1, 0, "%-*s", maxX, " SEARCH: ");
    attroff(COLOR_PAIR(CP_SEARCH) | A_BOLD);
    move(maxY - 1, 9);
    curs_set(1);
    echo();

    int ch;
    while ((ch = getch()) != '\n' && ch != 27) {
        if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (pos > 0) { pos--; searchBuf[pos] = '\0'; }
        } else if (pos < 126 && ch >= 32 && ch < 127) {
            searchBuf[pos++] = (char)ch;
            searchBuf[pos] = '\0';
        }
        attron(COLOR_PAIR(CP_SEARCH) | A_BOLD);
        mvprintw(maxY - 1, 0, " SEARCH: %-*s", maxX - 10, searchBuf);
        attroff(COLOR_PAIR(CP_SEARCH) | A_BOLD);
    }

    noecho();
    curs_set(0);

    if (ch != 27) g_searchFilter = searchBuf;
    g_cursor = 0;
    g_scrollOffset = 0;
    buildDisplayRows();
}

static void toggleCurrentItem()
{
    if (g_cursor < 0 || g_cursor >= (int)g_displayRows.size()) return;
    int idx = g_displayRows[g_cursor];

    if (idx < 0) {
        int si = -(idx + 1);
        g_suites[si].enabled = !g_suites[si].enabled;
        bool newState = g_suites[si].enabled;
        for (int t = g_suites[si].firstTestIndex;
             t < g_suites[si].firstTestIndex + g_suites[si].total; t++) {
            g_tests[t].enabled = newState;
        }
    } else {
        g_tests[idx].enabled = !g_tests[idx].enabled;
        for (auto& suite : g_suites) {
            bool anyEnabled = false;
            for (int t = suite.firstTestIndex; t < suite.firstTestIndex + suite.total; t++) {
                if (g_tests[t].enabled) { anyEnabled = true; break; }
            }
            suite.enabled = anyEnabled;
        }
    }
}

static void expandCollapseCurrentItem()
{
    if (g_cursor < 0 || g_cursor >= (int)g_displayRows.size()) return;
    int idx = g_displayRows[g_cursor];
    if (idx < 0) {
        int si = -(idx + 1);
        g_suites[si].expanded = !g_suites[si].expanded;
        buildDisplayRows();
    }
}

static void setAllEnabled(bool enabled)
{
    for (auto& t : g_tests) t.enabled = enabled;
    for (auto& s : g_suites) s.enabled = enabled;
}

// ============================================================================
// Main
// ============================================================================
int main(int argc, char** argv)
{
    TestMap testMap = buildTestMap();

    // Find manifest
    g_manifestPath = findManifestPath(argv[0]);
    if (g_manifestPath.empty()) {
        std::cerr << "ERROR: Cannot find tests/test_manifest.json" << std::endl;
        std::cerr << "Run from project root or build directory." << std::endl;
        return 1;
    }

    if (!initHarness(g_manifestPath, testMap)) {
        std::cerr << "ERROR: Failed to load test manifest from " << g_manifestPath << std::endl;
        return 1;
    }

    // Batch mode
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--batch") {
            auto& runner = mbc_test::TestRunner::instance();
            runner.reset();
            for (auto& suite : g_suites) {
                runner.beginSuite(suite.name);
                for (int t = suite.firstTestIndex;
                     t < suite.firstTestIndex + suite.total; t++) {
                    g_tests[t].func();
                }
                runner.endSuite();
            }
            return runner.printSummary();
        }
    }

    // Interactive ncurses mode
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    if (has_colors()) initColors();

    signal(SIGINT, [](int) { endwin(); exit(0); });

    bool running = true;
    while (running) {
        drawScreen();
        int ch = getch();

        switch (ch) {
            case 'q': case 'Q': running = false; break;
            case KEY_UP: case 'k': if (g_cursor > 0) g_cursor--; break;
            case KEY_DOWN: case 'j':
                if (g_cursor < (int)g_displayRows.size() - 1) g_cursor++; break;
            case KEY_PPAGE: g_cursor = std::max(0, g_cursor - 15); break;
            case KEY_NPAGE: g_cursor = std::min((int)g_displayRows.size() - 1, g_cursor + 15); break;
            case KEY_HOME: g_cursor = 0; break;
            case KEY_END: g_cursor = std::max(0, (int)g_displayRows.size() - 1); break;
            case ' ': toggleCurrentItem(); break;
            case '\n': case KEY_ENTER: expandCollapseCurrentItem(); break;
            case 'r': runTests(false); break;
            case 'R': runTests(true); break;
            case 'a': setAllEnabled(true); break;
            case 'n': setAllEnabled(false); break;
            case 'f':
                g_showFailuresOnly = !g_showFailuresOnly;
                g_cursor = 0; g_scrollOffset = 0;
                buildDisplayRows(); break;
            case '/': handleSearch(); break;
            case 27:
                g_searchFilter.clear(); g_showFailuresOnly = false;
                g_cursor = 0; g_scrollOffset = 0;
                buildDisplayRows(); break;
            default: break;
        }
    }

    endwin();
    return 0;
}
