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

#ifndef _RIDE_RATINGS_H_
#define _RIDE_RATINGS_H_

#include "../common.h"
#include "ride.h"

enum {
	RIDE_RATING_STATION_FLAG_NO_ENTRANCE = 1 << 0
};

typedef struct rct_ride_rating_calc_data {
	uint16	proximity_x;
	uint16	proximity_y;
	uint16	proximity_z;
	uint16	proximity_start_x;
	uint16	proximity_start_y;
	uint16	proximity_start_z;
	uint8	current_ride;
	uint8	state;
	uint8	proximity_track_type;
	uint8	proximity_base_height;
	uint16	proximity_total;
	uint16	proximity_scores[26];
	uint16	num_brakes;
	uint16	num_reversers;
	uint16	station_flags;

#ifdef __cplusplus
	void swapEndianness()
	{
		proximity_x = SDL_SwapLE16(proximity_x);
		proximity_y = SDL_SwapLE16(proximity_y);
		proximity_z = SDL_SwapLE16(proximity_z);
		proximity_start_x = SDL_SwapLE16(proximity_start_x);
		proximity_start_y = SDL_SwapLE16(proximity_start_y);
		proximity_start_z = SDL_SwapLE16(proximity_start_z);
		proximity_total = SDL_SwapLE16(proximity_total);
		for(int i = 0; i < (26); i++)
			proximity_scores[i] = SDL_SwapLE16(proximity_scores[i]);
		num_brakes = SDL_SwapLE16(num_brakes);
		num_reversers = SDL_SwapLE16(num_reversers);
		station_flags = SDL_SwapLE16(station_flags);
	}
#endif
} rct_ride_rating_calc_data;

extern rct_ride_rating_calc_data gRideRatingsCalcData;

void ride_ratings_update_all();

#endif
