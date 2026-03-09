// ============================================================================
// Missile Battery Command — Interactive Test Harness (ncurses)
//
// AN/TSQ-73 Test Console: browse suites, enable/disable tests, run
// selectively, and review pass/fail results in a terminal UI.
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
#include <algorithm>
#include <sstream>
#include <cstring>
#include <csignal>

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
// Test registration
// ============================================================================
#define REG(suite, func) reg.registerTest(suite, #func, func)

static void registerAllTests()
{
    auto& reg = mbc_test::TestRegistry::instance();

    // Cocos2d-x Validation
    REG("Cocos2d-x Validation", test_cocos2dx_build_flag);
    REG("Cocos2d-x Validation", test_cocos2dx_headers_available);
    REG("Cocos2d-x Validation", test_cocos2dx_director_handle);
    REG("Cocos2d-x Validation", test_cocos2dx_node_creation);
    REG("Cocos2d-x Validation", test_cocos2dx_scene_creation);
    REG("Cocos2d-x Validation", test_cocos2dx_drawnode_creation);
    REG("Cocos2d-x Validation", test_cocos2dx_label_creation);
    REG("Cocos2d-x Validation", test_cocos2dx_installation_path);
    REG("Cocos2d-x Validation", test_stub_mode_functional);

    // GameTypes
    REG("GameTypes", test_radar_constants);
    REG("GameTypes", test_radar_range_nm);
    REG("GameTypes", test_nm_km_conversion);
    REG("GameTypes", test_radar_sweep_rate);
    REG("GameTypes", test_patriot_constants);
    REG("GameTypes", test_hawk_constants);
    REG("GameTypes", test_scoring_constants);
    REG("GameTypes", test_javelin_constants);
    REG("GameTypes", test_hawk_stock_constants);
    REG("GameTypes", test_territory_radius);
    REG("GameTypes", test_polar_to_cartesian_north);
    REG("GameTypes", test_polar_to_cartesian_east);
    REG("GameTypes", test_polar_to_cartesian_south);
    REG("GameTypes", test_polar_to_cartesian_scale);
    REG("GameTypes", test_track_id_formatting);
    REG("GameTypes", test_track_id_formatting_large);
    REG("GameTypes", test_altitude_string_flight_level);
    REG("GameTypes", test_altitude_string_low);
    REG("GameTypes", test_classification_strings);
    REG("GameTypes", test_classification_friendly);
    REG("GameTypes", test_classification_pending);
    REG("GameTypes", test_classification_unknown);

    // Aircraft
    REG("Aircraft", test_aircraft_construction);
    REG("Aircraft", test_aircraft_initial_position);
    REG("Aircraft", test_aircraft_initial_azimuth);
    REG("Aircraft", test_aircraft_initial_altitude);
    REG("Aircraft", test_aircraft_initial_alive);
    REG("Aircraft", test_aircraft_initial_iff_pending);
    REG("Aircraft", test_aircraft_initial_track_id);
    REG("Aircraft", test_aircraft_friendly_flag);
    REG("Aircraft", test_aircraft_hostile_flag);
    REG("Aircraft", test_aircraft_moves_on_update);
    REG("Aircraft", test_aircraft_range_decreases_inbound);
    REG("Aircraft", test_aircraft_no_update_when_dead);
    REG("Aircraft", test_aircraft_destroy);
    REG("Aircraft", test_aircraft_territory_penetration);
    REG("Aircraft", test_aircraft_territory_penetration_inside);
    REG("Aircraft", test_friendly_no_territory_penetration);
    REG("Aircraft", test_rcs_strategic_bomber);
    REG("Aircraft", test_rcs_stealth_fighter);
    REG("Aircraft", test_rcs_stealth_smaller_than_bomber);
    REG("Aircraft", test_track_data_populated);
    REG("Aircraft", test_track_data_altitude);
    REG("Aircraft", test_track_data_iff_maps_to_classification);
    REG("Aircraft", test_iff_set_hostile);
    REG("Aircraft", test_iff_set_friendly);
    REG("Aircraft", test_threat_score_hostile);
    REG("Aircraft", test_threat_score_friendly_zero);
    REG("Aircraft", test_threat_score_closer_is_higher);
    REG("Aircraft", test_type_name_bomber);
    REG("Aircraft", test_type_name_stealth);
    REG("Aircraft", test_sweep_timer_initial);
    REG("Aircraft", test_sweep_timer_increments);
    REG("Aircraft", test_sweep_timer_reset);

    // IFF System
    REG("IFF System", test_iff_construction);
    REG("IFF System", test_iff_interrogation_starts);
    REG("IFF System", test_iff_not_interrogating_unknown_track);
    REG("IFF System", test_iff_no_duplicate_interrogation);
    REG("IFF System", test_iff_requires_track_id);
    REG("IFF System", test_iff_null_aircraft_ignored);
    REG("IFF System", test_iff_pending_before_complete);
    REG("IFF System", test_iff_completes_after_interval);
    REG("IFF System", test_iff_hostile_identified);
    REG("IFF System", test_iff_friendly_identified);
    REG("IFF System", test_iff_skip_already_identified);
    REG("IFF System", test_iff_multiple_aircraft);
    REG("IFF System", test_iff_multiple_both_interrogating);
    REG("IFF System", test_iff_multiple_both_resolve);
    REG("IFF System", test_iff_multiple_friendly_resolved);
    REG("IFF System", test_iff_dead_aircraft_no_result);
    REG("IFF System", test_iff_error_rate_setter);

    // Track Manager
    REG("Track Manager", test_track_manager_empty);
    REG("Track Manager", test_track_add);
    REG("Track Manager", test_track_add_increments_id);
    REG("Track Manager", test_track_add_sets_aircraft_id);
    REG("Track Manager", test_track_add_null_rejected);
    REG("Track Manager", test_track_count_after_add);
    REG("Track Manager", test_track_get_by_id);
    REG("Track Manager", test_track_get_nonexistent);
    REG("Track Manager", test_track_get_aircraft_by_id);
    REG("Track Manager", test_track_get_aircraft_is_same);
    REG("Track Manager", test_track_get_all);
    REG("Track Manager", test_track_remove);
    REG("Track Manager", test_track_remove_nonexistent_no_crash);
    REG("Track Manager", test_track_update_removes_dead);
    REG("Track Manager", test_track_hostile_count);
    REG("Track Manager", test_track_friendly_count);
    REG("Track Manager", test_track_hostile_ids);
    REG("Track Manager", test_track_reset);
    REG("Track Manager", test_track_reset_resets_ids);

    // Missile Battery
    REG("Missile Battery", test_patriot_construction);
    REG("Missile Battery", test_patriot_type);
    REG("Missile Battery", test_patriot_initial_ready);
    REG("Missile Battery", test_patriot_initial_missiles);
    REG("Missile Battery", test_patriot_no_track_assigned);
    REG("Missile Battery", test_hawk_construction);
    REG("Missile Battery", test_hawk_type);
    REG("Missile Battery", test_hawk_initial_missiles);
    REG("Missile Battery", test_can_engage_in_range);
    REG("Missile Battery", test_cannot_engage_out_of_range);
    REG("Missile Battery", test_cannot_engage_too_close);
    REG("Missile Battery", test_cannot_engage_too_high_for_hawk);
    REG("Missile Battery", test_cannot_engage_dead_target);
    REG("Missile Battery", test_cannot_engage_null);
    REG("Missile Battery", test_hawk_can_engage_low_alt);
    REG("Missile Battery", test_patriot_cannot_engage_very_low);
    REG("Missile Battery", test_engage_success);
    REG("Missile Battery", test_engage_consumes_missile);
    REG("Missile Battery", test_engage_changes_status);
    REG("Missile Battery", test_engage_sets_track_id);
    REG("Missile Battery", test_cannot_engage_while_engaged);
    REG("Missile Battery", test_engagement_produces_result);
    REG("Missile Battery", test_engagement_result_is_hit_or_miss);
    REG("Missile Battery", test_battery_returns_to_ready_after_engagement);
    REG("Missile Battery", test_clear_engagement_result);
    REG("Missile Battery", test_reload_after_all_missiles_spent);
    REG("Missile Battery", test_reload_completes);
    REG("Missile Battery", test_battery_data_populated);
    REG("Missile Battery", test_battery_data_range);
    REG("Missile Battery", test_battery_data_missiles);
    REG("Missile Battery", test_abort_tracking);
    REG("Missile Battery", test_javelin_construction);
    REG("Missile Battery", test_javelin_type);
    REG("Missile Battery", test_javelin_initial_missiles);
    REG("Missile Battery", test_javelin_can_engage_close_target);
    REG("Missile Battery", test_javelin_cannot_engage_high_alt);
    REG("Missile Battery", test_javelin_ir_seeker_no_missile_tracking);
    REG("Missile Battery", test_hawk_missile_stock);
    REG("Missile Battery", test_hawk_loaders);
    REG("Missile Battery", test_patriot_tracking_radar_type);
    REG("Missile Battery", test_hawk_tracking_radar_type);
    REG("Missile Battery", test_relocate_sets_offline);
    REG("Missile Battery", test_relocate_is_relocating);
    REG("Missile Battery", test_relocate_cannot_while_engaged);
    REG("Missile Battery", test_relocate_cannot_while_already_relocating);
    REG("Missile Battery", test_relocate_completes_patriot);
    REG("Missile Battery", test_relocate_completes_javelin);
    REG("Missile Battery", test_relocate_updates_position);
    REG("Missile Battery", test_relocate_clears_target);
    REG("Missile Battery", test_engagement_count_increments);
    REG("Missile Battery", test_hit_miss_count_adds_up);

    // Aircraft Generator
    REG("Aircraft Generator", test_generator_construction);
    REG("Aircraft Generator", test_generator_init);
    REG("Aircraft Generator", test_generator_set_level);
    REG("Aircraft Generator", test_generator_no_immediate_spawn);
    REG("Aircraft Generator", test_generator_spawns_eventually);
    REG("Aircraft Generator", test_generator_respects_max_count);
    REG("Aircraft Generator", test_generator_spawned_aircraft_valid_range);
    REG("Aircraft Generator", test_generator_spawned_aircraft_valid_range_upper);
    REG("Aircraft Generator", test_generator_spawned_aircraft_valid_azimuth);
    REG("Aircraft Generator", test_generator_spawned_aircraft_azimuth_upper);
    REG("Aircraft Generator", test_generator_spawned_aircraft_positive_speed);
    REG("Aircraft Generator", test_generator_spawned_aircraft_positive_altitude);
    REG("Aircraft Generator", test_generator_spawned_aircraft_alive);
    REG("Aircraft Generator", test_generator_reset);
    REG("Aircraft Generator", test_generator_level1_max);
    REG("Aircraft Generator", test_generator_level5_allows_more);

    // Fire Control System
    REG("Fire Control System", test_fcs_construction);
    REG("Fire Control System", test_fcs_has_3_patriots);
    REG("Fire Control System", test_fcs_has_3_hawks);
    REG("Fire Control System", test_fcs_get_patriot1);
    REG("Fire Control System", test_fcs_get_hawk2);
    REG("Fire Control System", test_fcs_get_nonexistent);
    REG("Fire Control System", test_fcs_patriot_designations);
    REG("Fire Control System", test_fcs_patriot2_exists);
    REG("Fire Control System", test_fcs_patriot3_exists);
    REG("Fire Control System", test_fcs_hawk1_exists);
    REG("Fire Control System", test_fcs_hawk3_exists);
    REG("Fire Control System", test_fcs_all_battery_data);
    REG("Fire Control System", test_fcs_all_batteries_ready);
    REG("Fire Control System", test_fcs_available_batteries_in_range);
    REG("Fire Control System", test_fcs_no_batteries_for_nonexistent_track);
    REG("Fire Control System", test_fcs_reset);
    REG("Fire Control System", test_fcs_update_no_crash);
    REG("Fire Control System", test_fcs_no_results_initially);
    REG("Fire Control System", test_fcs_has_3_javelins);
    REG("Fire Control System", test_fcs_javelin1_exists);
    REG("Fire Control System", test_fcs_javelin2_exists);
    REG("Fire Control System", test_fcs_javelin3_exists);
    REG("Fire Control System", test_hawk_total_stock);
    REG("Fire Control System", test_hawk_has_3_loaders);
    REG("Fire Control System", test_patriot_tracking_radar);
    REG("Fire Control System", test_hawk_tracking_radar);
    REG("Fire Control System", test_javelin_ir_seeker);

    // Game Config
    REG("Game Config", test_config_singleton);
    REG("Game Config", test_config_default_level);
    REG("Game Config", test_config_set_level);
    REG("Game Config", test_config_clamp_level_low);
    REG("Game Config", test_config_clamp_level_high);
    REG("Game Config", test_config_level1_max_aircraft);
    REG("Game Config", test_config_level1_no_stealth);
    REG("Game Config", test_config_level1_no_iff_errors);
    REG("Game Config", test_config_level1_speed_slow);
    REG("Game Config", test_config_level1_friendly_ratio);
    REG("Game Config", test_config_level5_max_aircraft);
    REG("Game Config", test_config_level5_stealth_enabled);
    REG("Game Config", test_config_level5_iff_errors);
    REG("Game Config", test_config_level5_speed_fast);
    REG("Game Config", test_config_level5_fewer_friendlies);
    REG("Game Config", test_config_max_aircraft_increases);
    REG("Game Config", test_config_speed_increases);
    REG("Game Config", test_config_friendly_ratio_decreases);
    REG("Game Config", test_config_spawn_interval_positive);
    REG("Game Config", test_config_score_strategic_bomber);
    REG("Game Config", test_config_score_fighter);
    REG("Game Config", test_config_score_recon_drone);
    REG("Game Config", test_config_score_higher_at_higher_level);

    // Airspace Manager
    REG("Airspace Manager", test_airspace_empty);
    REG("Airspace Manager", test_airspace_add_zone);
    REG("Airspace Manager", test_airspace_remove_zone);
    REG("Airspace Manager", test_airspace_get_zone);
    REG("Airspace Manager", test_airspace_get_nonexistent);
    REG("Airspace Manager", test_airspace_activate_deactivate);
    REG("Airspace Manager", test_airspace_nfz_contains);
    REG("Airspace Manager", test_airspace_nfz_outside);
    REG("Airspace Manager", test_airspace_nfz_altitude_above);
    REG("Airspace Manager", test_airspace_corridor_match);
    REG("Airspace Manager", test_airspace_corridor_wrong_heading);
    REG("Airspace Manager", test_airspace_default_init);
    REG("Airspace Manager", test_airspace_zone_type_strings);
    REG("Airspace Manager", test_airspace_azimuth_wraparound);

    // Data Cards
    REG("Data Cards", test_patriot_equipment_card);
    REG("Data Cards", test_patriot_card_radar);
    REG("Data Cards", test_patriot_card_guidance);
    REG("Data Cards", test_hawk_equipment_card);
    REG("Data Cards", test_hawk_card_guidance);
    REG("Data Cards", test_javelin_equipment_card);
    REG("Data Cards", test_javelin_card_guidance);
    REG("Data Cards", test_all_equipment_cards);
    REG("Data Cards", test_patriot_troop_strength);
    REG("Data Cards", test_hawk_troop_strength);
    REG("Data Cards", test_javelin_troop_strength);
    REG("Data Cards", test_equipment_card_format);
    REG("Data Cards", test_troop_card_format);
    REG("Data Cards", test_intel_card_format);

    // Threat Board
    REG("Threat Board", test_threat_board_empty);
    REG("Threat Board", test_threat_board_max_threats);
    REG("Threat Board", test_threat_board_detects_hostile);
    REG("Threat Board", test_threat_board_ignores_friendly);
    REG("Threat Board", test_threat_board_limits_to_5);
    REG("Threat Board", test_threat_board_closer_ranks_higher);
    REG("Threat Board", test_threat_board_inbound_ranks_higher);
    REG("Threat Board", test_threat_inbound_north_heading_south);
    REG("Threat Board", test_threat_outbound_north_heading_north);
    REG("Threat Board", test_threat_board_is_on_board);
    REG("Threat Board", test_threat_board_not_on_board);
    REG("Threat Board", test_threat_board_get_by_track_id);
    REG("Threat Board", test_threat_board_format);
    REG("Threat Board", test_threat_entry_format_line);
    REG("Threat Board", test_threat_entry_format_card);
    REG("Threat Board", test_threat_time_to_territory_inbound);
    REG("Threat Board", test_threat_closing_rate_positive_inbound);
    REG("Threat Board", test_threat_get_invalid_rank);
    REG("Threat Board", test_threat_board_empty_format);

    // Battalion HQ
    REG("Battalion HQ", test_hq_construction);
    REG("Battalion HQ", test_hq_init);
    REG("Battalion HQ", test_hq_init_position);
    REG("Battalion HQ", test_hq_relocate_starts);
    REG("Battalion HQ", test_hq_relocate_goes_offline);
    REG("Battalion HQ", test_hq_relocate_status);
    REG("Battalion HQ", test_hq_cannot_relocate_twice);
    REG("Battalion HQ", test_hq_relocate_completes);
    REG("Battalion HQ", test_hq_relocate_updates_position);
    REG("Battalion HQ", test_hq_relocate_time);
    REG("Battalion HQ", test_hq_setting_up_phase);
    REG("Battalion HQ", test_hq_data);
    REG("Battalion HQ", test_hq_data_format);
    REG("Battalion HQ", test_hq_status_string);
    REG("Battalion HQ", test_hq_relocating_status_string);
    REG("Battalion HQ", test_hq_personnel_count);
}

#undef REG

// ============================================================================
// Harness data structures
// ============================================================================

enum class TestStatus { NOT_RUN, PASSED, FAILED };

struct HarnessTest {
    std::string name;
    std::string suite;
    std::function<void()> func;
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
    int firstTestIndex;  // index into flat test list
};

// Global harness state
static std::vector<HarnessTest> g_tests;
static std::vector<HarnessSuite> g_suites;
static std::vector<int> g_displayRows;  // maps display row -> suite index (negative) or test index
static int g_cursor = 0;
static int g_scrollOffset = 0;
static int g_totalPassed = 0;
static int g_totalFailed = 0;
static int g_totalRun = 0;
static bool g_showFailuresOnly = false;
static std::string g_searchFilter;
static std::string g_lastRunMessage;

// ============================================================================
// Build display list
// ============================================================================
static void buildDisplayRows()
{
    g_displayRows.clear();

    for (int s = 0; s < (int)g_suites.size(); s++) {
        // Check if any tests in this suite match the search filter
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

        // Show failures only filter at suite level
        if (g_showFailuresOnly && g_suites[s].failed == 0 && g_suites[s].passed > 0) continue;

        g_displayRows.push_back(-(s + 1));  // Negative = suite header

        if (g_suites[s].expanded) {
            for (int t = g_suites[s].firstTestIndex;
                 t < g_suites[s].firstTestIndex + g_suites[s].total; t++) {
                // Search filter
                if (!g_searchFilter.empty() &&
                    g_tests[t].name.find(g_searchFilter) == std::string::npos) continue;
                // Failures only filter
                if (g_showFailuresOnly && g_tests[t].status != TestStatus::FAILED) continue;

                g_displayRows.push_back(t);
            }
        }
    }
}

// ============================================================================
// Initialize harness from registry
// ============================================================================
static void initHarness()
{
    auto& reg = mbc_test::TestRegistry::instance();
    auto suiteNames = reg.getSuiteNames();

    int testIndex = 0;
    for (const auto& suiteName : suiteNames) {
        auto suiteTests = reg.getTestsForSuite(suiteName);

        HarnessSuite hs;
        hs.name = suiteName;
        hs.expanded = false;
        hs.enabled = true;
        hs.passed = 0;
        hs.failed = 0;
        hs.total = (int)suiteTests.size();
        hs.firstTestIndex = testIndex;
        g_suites.push_back(hs);

        for (auto* rt : suiteTests) {
            HarnessTest ht;
            ht.name = rt->name;
            ht.suite = rt->suite;
            ht.func = rt->func;
            ht.enabled = true;
            ht.status = TestStatus::NOT_RUN;
            g_tests.push_back(ht);
            testIndex++;
        }
    }

    buildDisplayRows();
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

    // Reset all suite counts
    for (auto& s : g_suites) {
        s.passed = 0;
        s.failed = 0;
    }

    // Reset all test statuses
    for (auto& t : g_tests) {
        t.status = TestStatus::NOT_RUN;
        t.failMessage.clear();
    }

    // Redirect stdout to /dev/null during test execution
    FILE* devnull = fopen("/dev/null", "w");
    int savedStdout = dup(fileno(stdout));
    if (devnull) {
        dup2(fileno(devnull), fileno(stdout));
        fclose(devnull);
    }

    // Run each suite
    for (int s = 0; s < (int)g_suites.size(); s++) {
        bool suiteHasTests = false;

        for (int t = g_suites[s].firstTestIndex;
             t < g_suites[s].firstTestIndex + g_suites[s].total; t++) {
            if (!runAll && !g_tests[t].enabled) continue;
            suiteHasTests = true;
        }

        if (!suiteHasTests) continue;

        // Run individual tests with fresh runner state per test
        for (int t = g_suites[s].firstTestIndex;
             t < g_suites[s].firstTestIndex + g_suites[s].total; t++) {
            if (!runAll && !g_tests[t].enabled) continue;

            // Reset runner, run single test in a mini-suite
            runner.reset();
            runner.setQuiet(true);
            runner.beginSuite(g_suites[s].name);
            g_tests[t].func();
            runner.endSuite();

            // Read results
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

    // Restore stdout
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

// Color pairs
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

    // Status bar (line 1)
    attron(COLOR_PAIR(CP_STATUS));
    char statusBar[256];
    int totalTests = (int)g_tests.size();
    int enabledCount = 0;
    for (const auto& t : g_tests) { if (t.enabled) enabledCount++; }

    snprintf(statusBar, sizeof(statusBar),
        " %d suites | %d tests (%d enabled) | Last: %s",
        (int)g_suites.size(), totalTests, enabledCount,
        g_lastRunMessage.empty() ? "not run" : g_lastRunMessage.c_str());
    mvprintw(1, 0, "%-*s", maxX, statusBar);
    attroff(COLOR_PAIR(CP_STATUS));

    // Search bar
    if (!g_searchFilter.empty()) {
        attron(COLOR_PAIR(CP_SEARCH));
        mvprintw(2, 0, " FILTER: %s", g_searchFilter.c_str());
        attroff(COLOR_PAIR(CP_SEARCH));
    }

    int headerLines = g_searchFilter.empty() ? 3 : 4;
    int footerLines = 3;
    int listHeight = maxY - headerLines - footerLines;

    // Ensure cursor is in view
    if (g_cursor < g_scrollOffset) g_scrollOffset = g_cursor;
    if (g_cursor >= g_scrollOffset + listHeight) g_scrollOffset = g_cursor - listHeight + 1;
    if (g_scrollOffset < 0) g_scrollOffset = 0;

    // Draw test list
    for (int row = 0; row < listHeight && (row + g_scrollOffset) < (int)g_displayRows.size(); row++) {
        int y = headerLines + row;
        int idx = g_displayRows[row + g_scrollOffset];
        bool isSelected = (row + g_scrollOffset == g_cursor);

        if (isSelected) attron(A_REVERSE);

        if (idx < 0) {
            // Suite header
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
            // Individual test
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
            snprintf(line, sizeof(line), "     %s %-42s",
                     enableMark, test.name.c_str());

            if (!isSelected) attron(COLOR_PAIR(test.enabled ? cp : CP_DISABLED));
            mvprintw(y, 0, "%-*s", maxX - 6, line);
            printw("%s", statusMark);
            if (!isSelected) attroff(COLOR_PAIR(test.enabled ? cp : CP_DISABLED));
        }

        if (isSelected) attroff(A_REVERSE);
    }

    // Scroll indicator
    if ((int)g_displayRows.size() > listHeight) {
        int scrollPercent = g_scrollOffset * 100 / std::max(1, (int)g_displayRows.size() - listHeight);
        attron(COLOR_PAIR(CP_STATUS));
        mvprintw(headerLines + listHeight - 1, maxX - 8, " %3d%% ", scrollPercent);
        attroff(COLOR_PAIR(CP_STATUS));
    }

    // Detail panel: show failure message if cursor is on a failed test
    int detailY = maxY - footerLines;
    if (g_cursor >= 0 && g_cursor < (int)g_displayRows.size()) {
        int idx = g_displayRows[g_cursor];
        if (idx >= 0 && g_tests[idx].status == TestStatus::FAILED) {
            attron(COLOR_PAIR(CP_FAIL));
            std::string msg = g_tests[idx].failMessage;
            if ((int)msg.size() > maxX - 2) msg = msg.substr(0, maxX - 5) + "...";
            mvprintw(detailY, 0, "%-*s", maxX, (" ERR: " + msg).c_str());
            attroff(COLOR_PAIR(CP_FAIL));
            detailY++;
        }
    }

    // Footer - controls
    attron(A_BOLD | COLOR_PAIR(CP_HEADER));
    mvprintw(maxY - 2, 0, "%-*s",  maxX,
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

    // Show search prompt
    attron(COLOR_PAIR(CP_SEARCH) | A_BOLD);
    mvprintw(maxY - 1, 0, "%-*s", maxX, " SEARCH: ");
    attroff(COLOR_PAIR(CP_SEARCH) | A_BOLD);
    move(maxY - 1, 9);
    curs_set(1);
    echo();

    // Simple character-by-character input
    int ch;
    while ((ch = getch()) != '\n' && ch != 27) {
        if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (pos > 0) {
                pos--;
                searchBuf[pos] = '\0';
            }
        } else if (pos < 126 && ch >= 32 && ch < 127) {
            searchBuf[pos++] = (char)ch;
            searchBuf[pos] = '\0';
        }
        // Redraw search line
        attron(COLOR_PAIR(CP_SEARCH) | A_BOLD);
        mvprintw(maxY - 1, 0, " SEARCH: %-*s", maxX - 10, searchBuf);
        attroff(COLOR_PAIR(CP_SEARCH) | A_BOLD);
    }

    noecho();
    curs_set(0);

    if (ch != 27) {  // Not ESC
        g_searchFilter = searchBuf;
    }

    g_cursor = 0;
    g_scrollOffset = 0;
    buildDisplayRows();
}

static void toggleCurrentItem()
{
    if (g_cursor < 0 || g_cursor >= (int)g_displayRows.size()) return;
    int idx = g_displayRows[g_cursor];

    if (idx < 0) {
        // Toggle entire suite
        int si = -(idx + 1);
        g_suites[si].enabled = !g_suites[si].enabled;
        bool newState = g_suites[si].enabled;
        for (int t = g_suites[si].firstTestIndex;
             t < g_suites[si].firstTestIndex + g_suites[si].total; t++) {
            g_tests[t].enabled = newState;
        }
    } else {
        // Toggle individual test
        g_tests[idx].enabled = !g_tests[idx].enabled;

        // Update suite enabled state (enabled if any test enabled)
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
    // Register all tests
    registerAllTests();
    initHarness();

    // Check for --batch flag (non-interactive mode)
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--batch") {
            // Run all tests in batch mode (non-interactive)
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

    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    if (has_colors()) {
        initColors();
    }

    // Signal handler to restore terminal on SIGINT
    signal(SIGINT, [](int) {
        endwin();
        exit(0);
    });

    // Main loop
    bool running = true;
    while (running) {
        drawScreen();
        int ch = getch();

        switch (ch) {
            case 'q':
            case 'Q':
                running = false;
                break;

            case KEY_UP:
            case 'k':
                if (g_cursor > 0) g_cursor--;
                break;

            case KEY_DOWN:
            case 'j':
                if (g_cursor < (int)g_displayRows.size() - 1) g_cursor++;
                break;

            case KEY_PPAGE:  // Page Up
                g_cursor = std::max(0, g_cursor - 15);
                break;

            case KEY_NPAGE:  // Page Down
                g_cursor = std::min((int)g_displayRows.size() - 1, g_cursor + 15);
                break;

            case KEY_HOME:
                g_cursor = 0;
                break;

            case KEY_END:
                g_cursor = std::max(0, (int)g_displayRows.size() - 1);
                break;

            case ' ':
                toggleCurrentItem();
                break;

            case '\n':
            case KEY_ENTER:
                expandCollapseCurrentItem();
                break;

            case 'r':
                runTests(false);  // Run enabled only
                break;

            case 'R':
                runTests(true);   // Run all
                break;

            case 'a':
                setAllEnabled(true);
                break;

            case 'n':
                setAllEnabled(false);
                break;

            case 'f':
                g_showFailuresOnly = !g_showFailuresOnly;
                g_cursor = 0;
                g_scrollOffset = 0;
                buildDisplayRows();
                break;

            case '/':
                handleSearch();
                break;

            case 27:  // ESC — clear filter
                g_searchFilter.clear();
                g_showFailuresOnly = false;
                g_cursor = 0;
                g_scrollOffset = 0;
                buildDisplayRows();
                break;

            default:
                break;
        }
    }

    endwin();
    return 0;
}
