#pragma region Copyright (c) 2014-2016 OpenRCT2 Developers
/*****************************************************************************
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * OpenRCT2 is the work of many authors, a full list can be found in contributors.md
 * For more information, visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * A full copy of the GNU General Public License can be found in licence.txt
 *****************************************************************************/
#pragma endregion

#ifndef _SCENARIO_H_
#define _SCENARIO_H_

#include "../rct2/addresses.h"
#include "../common.h"
#include "../management/award.h"
#include "../management/finance.h"
#include "../management/news_item.h"
#include "../management/research.h"
#include "../ride/ride.h"
#include "../ride/ride_ratings.h"
#include "../object.h"
#include "../platform/platform.h"
#include "../world/banner.h"
#include "../world/map.h"
#include "../world/map_animation.h"
#include "../world/sprite.h"

#pragma pack(push, 1)
/**
 * SV6/SC6 header chunk
 * size: 0x20
 */
typedef struct rct_s6_header {
	uint8 type;					// 0x00
	uint8 classic_flag;			// 0x01
	uint16 num_packed_objects;	// 0x02
	uint32 version;				// 0x04
	uint32 magic_number;		// 0x08
	uint8 pad_0C[0x14];

#ifdef __cplusplus
	void swapEndianness()
	{
		num_packed_objects = SDL_SwapLE16(num_packed_objects);
		version = SDL_SwapLE32(version);
		magic_number = SDL_SwapLE32(magic_number);
	}
#endif
} rct_s6_header;
assert_struct_size(rct_s6_header, 0x20);

/**
 * SC6 information chunk
 * size: 0x198
 */
typedef struct rct_s6_info {
	uint8 editor_step;
	uint8 category;				// 0x01
	uint8 objective_type;		// 0x02
	uint8 objective_arg_1;		// 0x03
	sint32 objective_arg_2;		// 0x04
	sint16 objective_arg_3;		// 0x08
	uint8 pad_00A[0x3E];
	char name[64];				// 0x48
	char details[256];			// 0x88
	rct_object_entry entry;		// 0x188

#ifdef __cplusplus
	void swapEndianness()
	{
		objective_arg_2 = SDL_SwapLE32(objective_arg_2);
		objective_arg_3 = SDL_SwapLE16(objective_arg_3);
		entry.swapEndianness();
	}
#endif
} rct_s6_info;
assert_struct_size(rct_s6_info, 0x198);

/**
 * Scenario scores file header.
 * size: 0x10
 */
typedef struct rct_scenario_scores_header {
	uint32 var_0;
	uint32 var_4;
	uint32 var_8;
	uint32 scenario_count;		// 0x0C

#ifdef __cplusplus
	void swapEndianness()
	{
		var_0 = SDL_SwapLE32(var_0);
		var_4 = SDL_SwapLE32(var_4);
		var_8 = SDL_SwapLE32(var_8);
		scenario_count = SDL_SwapLE32(scenario_count);
	}
#endif
} rct_scenario_scores_header;
assert_struct_size(rct_scenario_scores_header, 16);

typedef enum scenario_source {
	SCENARIO_SOURCE_RCT1,
	SCENARIO_SOURCE_RCT1_AA,
	SCENARIO_SOURCE_RCT1_LL,
	SCENARIO_SOURCE_RCT2,
	SCENARIO_SOURCE_RCT2_WW,
	SCENARIO_SOURCE_RCT2_TT,
	SCENARIO_SOURCE_REAL,
	SCENARIO_SOURCE_OTHER
} scenario_source;

/**
 * Scenario basic structure, mainly for scenario select
 * size: 0x02B0
 */
typedef struct rct_scenario_basic {
	char path[256];				// 0x0000
	uint8 category;				// 0x0100
	uint8 pad_0101[0x1F];
	sint8 objective_type;		// 0x0120
	sint8 objective_arg_1;		// 0x0121
	sint32 objective_arg_2;		// 0x0122
	sint16 objective_arg_3;		// 0x0126
	char name[64];				// 0x0128
	char details[256];			// 0x0168
	sint32 flags;				// 0x0268
	money32 company_value;		// 0x026C
	char completed_by[64];		// 0x0270
	// uint8 source_game;			// new in OpenRCT2
	// sint16 source_index;		// new in OpenRCT2

#ifdef __cplusplus
	void swapEndianness()
	{
		objective_arg_2 = SDL_SwapLE32(objective_arg_2);
		objective_arg_3 = SDL_SwapLE16(objective_arg_3);
		flags = SDL_SwapLE32(flags);
		company_value = SDL_SwapLE32(company_value);
	}
#endif
} rct_scenario_basic;
assert_struct_size(rct_scenario_basic, 0x02B0);

typedef struct rct_stex_entry {
	rct_string_id scenario_name;	// 0x00
	rct_string_id park_name;		// 0x02
	rct_string_id details;			// 0x04
	uint8 var_06;

#ifdef __cplusplus
	void swapEndianness()
	{
		scenario_name = SDL_SwapLE16(scenario_name);
		park_name = SDL_SwapLE16(park_name);
		details = SDL_SwapLE16(details);
	}
#endif
} rct_stex_entry;
assert_struct_size(rct_stex_entry, 7);

#define g_stexEntries ((rct_stex_entry**)object_entry_groups[OBJECT_TYPE_SCENARIO_TEXT].chunks)

// This will be useful for backwards compatibility
typedef struct rct_s6_data {
	// SC6[0]
	rct_s6_header header;

	// SC6[1]
	rct_s6_info info;

	// SC6[2]
	// packed objects

	// SC6[3]
	rct_object_entry objects[721];

	// SC6[4]
	uint16 elapsed_months;
	uint16 current_day;
	uint32 scenario_ticks;
	uint32 scenario_srand_0;
	uint32 scenario_srand_1;

	// SC6[5]
	rct_map_element map_elements[0x30000];

	// SC6[6]
	uint32 next_free_map_element_pointer_index;
	rct_sprite sprites[10000];
	uint16 sprite_lists_head[6];
	uint16 sprite_lists_count[6];
	rct_string_id park_name;
	uint8 pad_013573D6[2];
	uint32 park_name_args;
	money32 initial_cash;
	money32 current_loan;
	uint32 park_flags;
	money16 park_entrance_fee;
	uint16 rct1_park_entrance_x;
	uint16 rct1_park_entrance_y;
	uint8 pad_013573EE[2];
	uint8 rct1_park_entrance_z;
	uint8 pad_013573F1;
	rct2_peep_spawn peep_spawns[2];
	uint8 guest_count_change_modifier;
	uint8 current_research_level;
	uint8 pad_01357400[4];
	uint32 researched_ride_types[8];
	uint32 researched_ride_entries[8];
	uint32 researched_track_types_a[128];
	uint32 researched_track_types_b[128];

	// SC6[7]
	uint16 guests_in_park;
	uint16 guests_heading_for_park;

	// Ignored in scenario
	money32 expenditure_table[224];

	// SC6[8]
	uint16 last_guests_in_park;
	uint8 pad_01357BCA[3];
	uint8 handyman_colour;
	uint8 mechanic_colour;
	uint8 security_colour;

	// Ignored in scenario
	uint32 researched_scenery_items[56];

	// SC6[9]
	uint16 park_rating;

	// Ignored in scenario
	uint8 park_rating_history[32];
	uint8 guests_in_park_history[32];

	// SC6[10]
	uint8 active_research_types;
	uint8 research_progress_stage;
	uint32 last_researched_item_subject;
	uint8 pad_01357CF8[1000];
	uint32 next_research_item;
	uint16 research_progress;
	uint8 next_research_category;
	uint8 next_research_expected_day;
	uint8 next_research_expected_month;
	uint8 guest_initial_happiness;
	uint16 park_size;
	uint16 guest_generation_probability;
	uint16 total_ride_value;
	money32 maximum_loan;
	money16 guest_initial_cash;
	uint8 guest_initial_hunger;
	uint8 guest_initial_thirst;
	uint8 objective_type;
	uint8 objective_year;
	uint8 pad_013580FA[2];
	money32 objective_currency;
	uint16 objective_guests;
	uint8 campaign_weeks_left[20];
	uint8 campaign_ride_index[22];

	// Ignored in scenario
	money32 balance_history[128];

	// SC6[11]
	money32 current_expenditure;
	money32 current_profit;
	uint32 weekly_profit_average_dividend;
	uint16 weekly_profit_average_divisor;
	uint8 pad_0135833A[2];

	// Ignored in scenario
	money32 weekly_profit_history[128];

	// SC6[12]
	money32 park_value;

	// Ignored in scenario
	money32 park_value_history[128];

	// SC6[13]
	money32 completed_company_value;
	uint32 total_admissions;
	money32 income_from_admissions;
	money32 company_value;
	uint8 peep_warning_throttle[16];
	rct_award awards[MAX_AWARDS];
	money16 land_price;
	money16 construction_rights_price;
	uint16 word_01358774;
	uint8 pad_01358776[2];
	uint32 cd_key;
	uint8 pad_0135877C[64];
	uint32 game_version_number;
	money32 completed_company_value_record;
	uint32 loan_hash;
	uint16 ride_count;
	uint8 pad_013587CA[6];
	money32 historical_profit;
	uint8 pad_013587D4[4];
	char scenario_completed_name[32];
	money32 cash;
	uint8 pad_013587FC[50];
	uint16 park_rating_casualty_penalty;
	uint16 map_size_units;
	uint16 map_size_minus_2;
	uint16 map_size;
	uint16 map_max_xy;
	uint32 same_price_throughout;
	uint16 suggested_max_guests;
	uint16 park_rating_warning_days;
	uint8 last_entrance_style;
	uint8 rct1_water_colour;
	uint8 pad_01358842[2];
	rct_research_item research_items[500];
	uint16 map_base_z;
	char scenario_name[64];
	char scenario_description[256];
	uint8 current_interest_rate;
	uint8 pad_0135934B;
	uint32 same_price_throughout_extended;
	uint16 park_entrance_x[4];
	uint16 park_entrance_y[4];
	uint16 park_entrance_z[4];
	uint8 park_entrance_direction[4];
	char scenario_filename[256];
	uint8 saved_expansion_pack_names[3256];
	rct_banner banners[250];
	char custom_strings[0x8000];
	uint32 game_ticks_1;
	rct_ride rides[255];
	uint16 saved_age;
	uint16 saved_view_x;
	uint16 saved_view_y;
	uint8 saved_view_zoom;
	uint8 saved_view_rotation;
	rct_map_animation map_animations[2000];
	uint16 num_map_animations;
	uint8 pad_0138B582[2];
	rct_ride_rating_calc_data ride_ratings_calc_data;
	uint8 pad_0138B5D0[60];
	rct_ride_measurement ride_measurements[8];
	uint32 next_guest_index;
	uint16 grass_and_scenery_tilepos;
	uint32 patrol_areas[204 * 128];
	uint8 staff_modes[204];
	uint8 pad_13CA73E;
	uint8 pad_13CA73F;
	uint8 byte_13CA740;
	uint8 pad_13CA741;
	uint8 byte_13CA742[4]; // unused
	uint8 climate;
	uint8 pad_013CA747;
	uint16 climate_update_timer;
	uint8 current_weather;
	uint8 next_weather;
	uint8 temperature;
	uint8 next_temperature;
	uint8 current_weather_effect;
	uint8 next_weather_effect;
	uint8 current_weather_gloom;
	uint8 next_weather_gloom;
	uint8 current_rain_level;
	uint8 next_rain_level;
	rct_news_item news_items[MAX_NEWS_ITEMS];
	uint8 pad_13CE730[64];
	uint32 rct1_scenario_flags;
	uint16 wide_path_tile_loop_x;
	uint16 wide_path_tile_loop_y;
	uint8 pad_13CE778[434];

#ifdef __cplusplus
	void swapEndianness()
	{
		header.swapEndianness();
		info.swapEndianness();
		for(int i = 0; i < (721); i++)
			objects[i].swapEndianness();
		elapsed_months = SDL_SwapLE16(elapsed_months);
		current_day = SDL_SwapLE16(current_day);
		scenario_ticks = SDL_SwapLE32(scenario_ticks);
		scenario_srand_0 = SDL_SwapLE32(scenario_srand_0);
		scenario_srand_1 = SDL_SwapLE32(scenario_srand_1);
		for(int i = 0; i < (0x30000); i++)
			map_elements[i].swapEndianness();
		next_free_map_element_pointer_index = SDL_SwapLE32(next_free_map_element_pointer_index);
		for(int i = 0; i < (10000); i++)
			sprites[i].swapEndianness();
		for(int i = 0; i < (6); i++)
			sprite_lists_head[i] = SDL_SwapLE16(sprite_lists_head[i]);
		for(int i = 0; i < (6); i++)
			sprite_lists_count[i] = SDL_SwapLE16(sprite_lists_count[i]);
		park_name = SDL_SwapLE16(park_name);
		park_name_args = SDL_SwapLE32(park_name_args);
		initial_cash = SDL_SwapLE32(initial_cash);
		current_loan = SDL_SwapLE32(current_loan);
		park_flags = SDL_SwapLE32(park_flags);
		park_entrance_fee = SDL_SwapLE16(park_entrance_fee);
		rct1_park_entrance_x = SDL_SwapLE16(rct1_park_entrance_x);
		rct1_park_entrance_y = SDL_SwapLE16(rct1_park_entrance_y);
		for(int i = 0; i < (2); i++)
			peep_spawns[i].swapEndianness();
		for(int i = 0; i < (8); i++)
			researched_ride_types[i] = SDL_SwapLE32(researched_ride_types[i]);
		for(int i = 0; i < (8); i++)
			researched_ride_entries[i] = SDL_SwapLE32(researched_ride_entries[i]);
		for(int i = 0; i < (128); i++)
			researched_track_types_a[i] = SDL_SwapLE32(researched_track_types_a[i]);
		for(int i = 0; i < (128); i++)
			researched_track_types_b[i] = SDL_SwapLE32(researched_track_types_b[i]);
		guests_in_park = SDL_SwapLE16(guests_in_park);
		guests_heading_for_park = SDL_SwapLE16(guests_heading_for_park);
		for(int i = 0; i < (224); i++)
			expenditure_table[i] = SDL_SwapLE32(expenditure_table[i]);
		last_guests_in_park = SDL_SwapLE16(last_guests_in_park);
		for(int i = 0; i < (56); i++)
			researched_scenery_items[i] = SDL_SwapLE32(researched_scenery_items[i]);
		park_rating = SDL_SwapLE16(park_rating);
		last_researched_item_subject = SDL_SwapLE32(last_researched_item_subject);
		next_research_item = SDL_SwapLE32(next_research_item);
		research_progress = SDL_SwapLE16(research_progress);
		park_size = SDL_SwapLE16(park_size);
		guest_generation_probability = SDL_SwapLE16(guest_generation_probability);
		total_ride_value = SDL_SwapLE16(total_ride_value);
		maximum_loan = SDL_SwapLE32(maximum_loan);
		guest_initial_cash = SDL_SwapLE16(guest_initial_cash);
		objective_currency = SDL_SwapLE32(objective_currency);
		objective_guests = SDL_SwapLE16(objective_guests);
		for(int i = 0; i < (128); i++)
			balance_history[i] = SDL_SwapLE32(balance_history[i]);
		current_expenditure = SDL_SwapLE32(current_expenditure);
		current_profit = SDL_SwapLE32(current_profit);
		weekly_profit_average_dividend = SDL_SwapLE32(weekly_profit_average_dividend);
		weekly_profit_average_divisor = SDL_SwapLE16(weekly_profit_average_divisor);
		for(int i = 0; i < (128); i++)
			weekly_profit_history[i] = SDL_SwapLE32(weekly_profit_history[i]);
		park_value = SDL_SwapLE32(park_value);
		for(int i = 0; i < (128); i++)
			park_value_history[i] = SDL_SwapLE32(park_value_history[i]);
		completed_company_value = SDL_SwapLE32(completed_company_value);
		total_admissions = SDL_SwapLE32(total_admissions);
		income_from_admissions = SDL_SwapLE32(income_from_admissions);
		company_value = SDL_SwapLE32(company_value);
		for(int i = 0; i < (MAX_AWARDS); i++)
			awards[i].swapEndianness();
		land_price = SDL_SwapLE16(land_price);
		construction_rights_price = SDL_SwapLE16(construction_rights_price);
		word_01358774 = SDL_SwapLE16(word_01358774);
		cd_key = SDL_SwapLE32(cd_key);
		game_version_number = SDL_SwapLE32(game_version_number);
		completed_company_value_record = SDL_SwapLE32(completed_company_value_record);
		loan_hash = SDL_SwapLE32(loan_hash);
		ride_count = SDL_SwapLE16(ride_count);
		historical_profit = SDL_SwapLE32(historical_profit);
		cash = SDL_SwapLE32(cash);
		park_rating_casualty_penalty = SDL_SwapLE16(park_rating_casualty_penalty);
		map_size_units = SDL_SwapLE16(map_size_units);
		map_size_minus_2 = SDL_SwapLE16(map_size_minus_2);
		map_size = SDL_SwapLE16(map_size);
		map_max_xy = SDL_SwapLE16(map_max_xy);
		same_price_throughout = SDL_SwapLE32(same_price_throughout);
		suggested_max_guests = SDL_SwapLE16(suggested_max_guests);
		park_rating_warning_days = SDL_SwapLE16(park_rating_warning_days);
		for(int i = 0; i < (500); i++)
			research_items[i].swapEndianness();
		map_base_z = SDL_SwapLE16(map_base_z);
		same_price_throughout_extended = SDL_SwapLE32(same_price_throughout_extended);
		for(int i = 0; i < (4); i++)
			park_entrance_x[i] = SDL_SwapLE16(park_entrance_x[i]);
		for(int i = 0; i < (4); i++)
			park_entrance_y[i] = SDL_SwapLE16(park_entrance_y[i]);
		for(int i = 0; i < (4); i++)
			park_entrance_z[i] = SDL_SwapLE16(park_entrance_z[i]);
		for(int i = 0; i < (250); i++)
			banners[i].swapEndianness();
		game_ticks_1 = SDL_SwapLE32(game_ticks_1);
		for(int i = 0; i < (255); i++)
			rides[i].swapEndianness();
		saved_age = SDL_SwapLE16(saved_age);
		saved_view_x = SDL_SwapLE16(saved_view_x);
		saved_view_y = SDL_SwapLE16(saved_view_y);
		for(int i = 0; i < (2000); i++)
			map_animations[i].swapEndianness();
		num_map_animations = SDL_SwapLE16(num_map_animations);
		ride_ratings_calc_data.swapEndianness();
		for(int i = 0; i < (8); i++)
			ride_measurements[i].swapEndianness();
		next_guest_index = SDL_SwapLE32(next_guest_index);
		grass_and_scenery_tilepos = SDL_SwapLE16(grass_and_scenery_tilepos);
		for(int i = 0; i < (204 * 128); i++)
			patrol_areas[i] = SDL_SwapLE32(patrol_areas[i]);
		climate_update_timer = SDL_SwapLE16(climate_update_timer);
		for(int i = 0; i < (MAX_NEWS_ITEMS); i++)
			news_items[i].swapEndianness();
		rct1_scenario_flags = SDL_SwapLE32(rct1_scenario_flags);
		wide_path_tile_loop_x = SDL_SwapLE16(wide_path_tile_loop_x);
		wide_path_tile_loop_y = SDL_SwapLE16(wide_path_tile_loop_y);
	}
#endif
} rct_s6_data;
assert_struct_size(rct_s6_data, 0x46b44a);
#pragma pack(pop)

enum {
	SCENARIO_FLAGS_VISIBLE = (1 << 0),
	SCENARIO_FLAGS_COMPLETED = (1 << 1),
	SCENARIO_FLAGS_SIXFLAGS = (1 << 2)
};

enum {
	S6_TYPE_SAVEDGAME,
	S6_TYPE_SCENARIO
};

#define S6_RCT2_VERSION 120001
#define S6_MAGIC_NUMBER 0x00031144

enum {
	// RCT2 categories (keep order)
	SCENARIO_CATEGORY_BEGINNER,
	SCENARIO_CATEGORY_CHALLENGING,
	SCENARIO_CATEGORY_EXPERT,
	SCENARIO_CATEGORY_REAL,
	SCENARIO_CATEGORY_OTHER,

	// OpenRCT2 categories
	SCENARIO_CATEGORY_DLC,
	SCENARIO_CATEGORY_BUILD_YOUR_OWN,

	SCENARIO_CATEGORY_COUNT
};

enum {
	OBJECTIVE_NONE,
	OBJECTIVE_GUESTS_BY,
	OBJECTIVE_PARK_VALUE_BY,
	OBJECTIVE_HAVE_FUN,
	OBJECTIVE_BUILD_THE_BEST,
	OBJECTIVE_10_ROLLERCOASTERS,
	OBJECTIVE_GUESTS_AND_RATING,
	OBJECTIVE_MONTHLY_RIDE_INCOME,
	OBJECTIVE_10_ROLLERCOASTERS_LENGTH,
	OBJECTIVE_FINISH_5_ROLLERCOASTERS,
	OBJECTIVE_REPLAY_LOAN_AND_PARK_VALUE,
	OBJECTIVE_MONTHLY_FOOD_INCOME
};

#define AUTOSAVE_PAUSE 0

extern const rct_string_id ScenarioCategoryStringIds[SCENARIO_CATEGORY_COUNT];

#if defined(NO_RCT2)
extern uint32 gScenarioTicks;
#else
#define gScenarioTicks						RCT2_GLOBAL(RCT2_ADDRESS_SCENARIO_TICKS, uint32)
#endif
extern uint32 gScenarioSrand0;
extern uint32 gScenarioSrand1;

extern uint8 gScenarioObjectiveType;
extern uint8 gScenarioObjectiveYear;
extern uint16 gScenarioObjectiveNumGuests;
extern money32 gScenarioObjectiveCurrency;

extern uint16 gScenarioParkRatingWarningDays;
extern money32 gScenarioCompletedCompanyValue;
extern money32 gScenarioCompanyValueRecord;

extern rct_s6_info gS6Info;
extern char gScenarioName[64];
extern char gScenarioDetails[256];
extern char gScenarioCompletedBy[32];
extern char gScenarioSavePath[MAX_PATH];
extern char gScenarioExpansionPacks[3256];
extern int gFirstTimeSave;
extern uint16 gSavedAge;
extern uint32 gLastAutoSaveUpdate;

extern const char *_scenarioFileName;

bool scenario_load_basic(const char *path, rct_s6_header *header, rct_s6_info *info);
bool scenario_load_rw(SDL_RWops * rw);
int scenario_load(const char *path);
int scenario_load_and_play_from_path(const char *path);
void scenario_begin();
void scenario_update();
unsigned int scenario_rand();
unsigned int scenario_rand_max(unsigned int max);
int scenario_prepare_for_save();
int scenario_save(SDL_RWops* rw, int flags);
void scenario_remove_trackless_rides(rct_s6_data *s6);
void scenario_fix_ghosts(rct_s6_data *s6);
void scenario_set_filename(const char *value);
void scenario_failure();
void scenario_success();
void scenario_success_submit_name(const char *name);
void scenario_autosave_check();

#endif
