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

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include "diagnostic.h"

int _log_levels[DIAGNOSTIC_LEVEL_COUNT] = { 1, 1, 1, 0, 1 };
int _log_location_enabled = 1;

const char * _level_strings[] = {
	"FATAL",
	"ERROR",
	"WARNING",
	"VERBOSE",
	"INFO"
};

void diagnostic_log(int diagnosticLevel, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	char buf[1024];
	vsnprintf(buf, 1024, format, args);
	platform_print(buf);
	//printf(buf);
    //vprintf(format, args);
	//char* res;
	//if (vasprintf(&res, format, args) >= 0)
	//{
		//nocashPrint(res);
	//	free(res);
	//}

	va_end(args);

	/*FILE *stream;
	va_list args;

	if (!_log_levels[diagnosticLevel])
		return;

	stream = stderr;

	// Level
	fprintf(stream, "%s: ", _level_strings[diagnosticLevel]);

	// Message
	va_start(args, format);
	vfprintf(stream, format, args);
	va_end(args);

	// Line terminator
	fprintf(stream, "\n");*/
}

void diagnostic_log_with_location(int diagnosticLevel, const char *file, const char *function, int line, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	char buf[1024];
	vsnprintf(buf, 1024, format, args);
	platform_print(buf);
	//printf(buf);
    //vprintf(format, args);
	//char* res;
	//if (vasprintf(&res, format, args) >= 0)
	//{
	//	nocashPrint(res);
	//	free(res);
	//}

	va_end(args);

	/*FILE *stream;
	va_list args;

	if (!_log_levels[diagnosticLevel])
		return;

	stream = stderr;

	// Level and source code information
	if (_log_location_enabled)
		fprintf(stream, "%s[%s:%d (%s)]: ", _level_strings[diagnosticLevel], file, line, function);
	else
		fprintf(stream, "%s: ", _level_strings[diagnosticLevel]);

	// Message
	va_start(args, format);
	vfprintf(stream, format, args);
	va_end(args);

	// Line terminator
	fprintf(stream, "\n");*/
}
