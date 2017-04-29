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

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__)) || defined(__FreeBSD__) || defined(__NDS__) || defined(__WII__)

#include "../common.h"
#include <fat.h>
#include <unistd.h> 

#include <dirent.h>
#include <errno.h>
#include <fnmatch.h>
#include <libgen.h>
#include <locale.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pwd.h>
#include <time.h>
#include "../config.h"
#include "../localisation/language.h"
#include "../OpenRCT2.h"
#include "../util/util.h"
#include "platform.h"
#include "../mem2heap.h"
#include <dirent.h>
#include <sys/time.h>
#include <time.h>
//#include <fts.h>
#include <sys/file.h>
#include <wiiuse/wpad.h>
#include <ogc/machine/processor.h>

// The name of the mutex used to prevent multiple instances of the game from running
#define SINGLE_INSTANCE_MUTEX_NAME "openrct2.lock"

#define FILE_BUFFER_SIZE 4096

utf8 _userDataDirectoryPath[MAX_PATH] = { 0 };
utf8 _openrctDataDirectoryPath[MAX_PATH] = { 0 };

static void *xfb = NULL;
GXRModeObj *rmode = NULL;

void _wrapup_reent(struct _reent * a)
{

}

#define _SHIFTL(v, s, w)	\
    ((u32) (((u32)(v) & ((0x01 << (w)) - 1)) << (s)))
#define _SHIFTR(v, s, w)	\
    ((u32)(((u32)(v) >> (s)) & ((0x01 << (w)) - 1)))

static u32 vdacFlagRegion;
static u32 i2cIdentFirst = 0;
static u32 i2cIdentFlag = 1;
static u32 oldTvStatus = 0x03e7;
static u32 oldDtvStatus = 0x03e7;
static vu32 *_i2cReg = (u32*)0xCD800000;

static inline void __viOpenI2C(u32 channel)
{
	u32 val = ((_i2cReg[49]&~0x8000)|0x4000);
	val |= _SHIFTL(channel,15,1);
	_i2cReg[49] = val;
}

static inline u32 __viSetSCL(u32 channel)
{
	u32 val = (_i2cReg[48]&~0x4000);
	val |= _SHIFTL(channel,14,1);
	_i2cReg[48] = val;
	return 1;
}
static inline u32 __viSetSDA(u32 channel)
{
	u32 val = (_i2cReg[48]&~0x8000);
	val |= _SHIFTL(channel,15,1);
	_i2cReg[48] = val;
	return 1;
}

static inline u32 __viGetSDA()
{
	return _SHIFTR(_i2cReg[50],15,1);
}

static inline void __viCheckI2C()
{
	__viOpenI2C(0);
	udelay(4);

	i2cIdentFlag = 0;
	if(__viGetSDA()!=0) i2cIdentFlag = 1;
}

static u32 __sendSlaveAddress(u8 addr)
{
	u32 i;

	__viSetSDA(i2cIdentFlag^1);
	udelay(2);

	__viSetSCL(0);
	for(i=0;i<8;i++) {
		if(addr&0x80) __viSetSDA(i2cIdentFlag);
		else __viSetSDA(i2cIdentFlag^1);
		udelay(2);

		__viSetSCL(1);
		udelay(2);

		__viSetSCL(0);
		addr <<= 1;
	}

	__viOpenI2C(0);
	udelay(2);

	__viSetSCL(1);
	udelay(2);

	if(i2cIdentFlag==1 && __viGetSDA()!=0) return 0;

	__viSetSDA(i2cIdentFlag^1);
	__viOpenI2C(1);
	__viSetSCL(0);

	return 1;
}

static u32 __VISendI2CData(u8 addr,void *val,u32 len)
{
	u8 c;
	s32 i,j;
	u32 level,ret;

	if(i2cIdentFirst==0) {
		__viCheckI2C();
		i2cIdentFirst = 1;
	}

	_CPU_ISR_Disable(level);

	__viOpenI2C(1);
	__viSetSCL(1);

	__viSetSDA(i2cIdentFlag);
	udelay(4);

	ret = __sendSlaveAddress(addr);
	if(ret==0) {
		_CPU_ISR_Restore(level);
		return 0;
	}

	__viOpenI2C(1);
	for(i=0;i<len;i++) {
		c = ((u8*)val)[i];
		for(j=0;j<8;j++) {
			if(c&0x80) __viSetSDA(i2cIdentFlag);
			else __viSetSDA(i2cIdentFlag^1);
			udelay(2);

			__viSetSCL(1);
			udelay(2);
			__viSetSCL(0);

			c <<= 1;
		}
		__viOpenI2C(0);
		udelay(2);
		__viSetSCL(1);
		udelay(2);

		if(i2cIdentFlag==1 && __viGetSDA()!=0) {
			_CPU_ISR_Restore(level);
			return 0;
		}

		__viSetSDA(i2cIdentFlag^1);
		__viOpenI2C(1);
		__viSetSCL(0);
	}

	__viOpenI2C(1);
	__viSetSDA(i2cIdentFlag^1);
	udelay(2);
	__viSetSDA(i2cIdentFlag);

	_CPU_ISR_Restore(level);
	return 1;
}

static void __VIWriteI2CRegister8(u8 reg, u8 data)
{
	u8 buf[2];
	buf[0] = reg;
	buf[1] = data;
	__VISendI2CData(0xe0,buf,2);
	udelay(2);
}

GXRModeObj mPalVideoMode = 
{
    VI_TVMODE_PAL_INT,      // viDisplayMode
    640,             // fbWidth
    464,             // efbHeight
    464,             // xfbHeight
    (VI_MAX_WIDTH_PAL - 640)/2,         // viXOrigin
    (VI_MAX_HEIGHT_PAL - 464)/2,        // viYOrigin
    640,             // viWidth
    464,             // viHeight
    VI_XFBMODE_DF,   // xFBmode
    GX_FALSE,        // field_rendering
    GX_FALSE,        // aa

    // sample points arranged in increasing Y order
	{
		{6,6},{6,6},{6,6},  // pix 0, 3 sample points, 1/12 units, 4 bits each
		{6,6},{6,6},{6,6},  // pix 1
		{6,6},{6,6},{6,6},  // pix 2
		{6,6},{6,6},{6,6}   // pix 3
	},
    // vertical filter[7], 1/64 units, 6 bits each
	{
		 0,         // line n-1
		 0,         // line n-1
		21,         // line n
		22,         // line n
		21,         // line n
		 0,         // line n+1
		 0          // line n+1
	}
};

/**
 * The function that is called directly from the host application (rct2.exe)'s WinMain.
 * This will be removed when OpenRCT2 can be built as a stand alone application.
 */
int main(int argc, const char **argv)
{
	//defaultExceptionHandler();
	VIDEO_Init();

	// This function initialises the attached controllers
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetIdleTimeout(120);
	
	// Obtain the preferred video mode from the system
	// This will correspond to the settings in the Wii menu
	rmode = &mPalVideoMode;//VIDEO_GetPreferredMode(NULL);

	// Allocate memory for the display in the uncached region
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	memset(xfb, COLOR_BLACK, rmode->fbWidth * rmode->xfbHeight * 2);
	
	// Initialise the console, required for printf
	//console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
	
	// Set up the video registers with the chosen mode
	VIDEO_Configure(rmode);

	__VIWriteI2CRegister8(0x03, 0);
	
	// Tell the video hardware where our display memory is
	VIDEO_SetNextFramebuffer(xfb);
	
	// Make the display visible
	VIDEO_SetBlack(FALSE);

	// Flush the video register changes to the hardware
	VIDEO_Flush();

	// Wait for Video setup to complete
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	mem2heap_init();

	fatInitDefault();
	core_init();

	//int run_game = cmdline_run(argv, argc);
	//if (run_game == 1)
	//{
		openrct2_launch();
	//}

	exit(gExitCode);
	return gExitCode;
}

void platform_get_date_utc(rct2_date *out_date)
{
	assert(out_date != NULL);
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = gmtime(&rawtime);
	out_date->day = timeinfo->tm_mday;
	out_date->month = timeinfo->tm_mon + 1;
	out_date->year = timeinfo->tm_year + 1900;
	out_date->day_of_week = timeinfo->tm_wday;
}

void platform_get_time_utc(rct2_time *out_time)
{
	assert(out_time != NULL);
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = gmtime(&rawtime);
	out_time->second = timeinfo->tm_sec;
	out_time->minute = timeinfo->tm_min;
	out_time->hour = timeinfo->tm_hour;
}

void platform_get_date_local(rct2_date *out_date)
{
	assert(out_date != NULL);
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	out_date->day = timeinfo->tm_mday;
	out_date->month = timeinfo->tm_mon + 1;
	out_date->year = timeinfo->tm_year + 1900;
	out_date->day_of_week = timeinfo->tm_wday;
}

void platform_get_time_local(rct2_time *out_time)
{
	assert(out_time != NULL);
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	out_time->second = timeinfo->tm_sec;
	out_time->minute = timeinfo->tm_min;
	out_time->hour = timeinfo->tm_hour;
}

static size_t platform_utf8_to_multibyte(const utf8 *path, char *buffer, size_t buffer_size)
{
	/*int len = strlen(path);
	if (len > buffer_size - 1) {
		//truncated = true;
		len = buffer_size - 1;
	}
	strncpy(buffer, path, len);
	buffer[len] = '\0';*/
	wchar_t *wpath = utf8_to_widechar(path);
	setlocale(LC_CTYPE, "");
	size_t len = wcstombs(NULL, wpath, 0);
	bool truncated = false;
	if (len > buffer_size - 1) {
		truncated = true;
		len = buffer_size - 1;
	}
	wcstombs(buffer, wpath, len);
	buffer[len] = '\0';
	if (truncated)
		log_warning("truncated string %s", buffer);
	free(wpath);
	//int len = 0;
	//while(*path != 0)
	//{
	//	*buffer++ = *path++;
	//	len++;
	//}
	//*buffer = 0;
	return len;
}

bool platform_file_exists(const utf8 *path)
{
	char buffer[MAX_PATH];
	platform_utf8_to_multibyte(path, buffer, MAX_PATH);
	bool exists = access(buffer, F_OK) != -1;
	log_verbose("file '%s' exists = %i", buffer, exists);
	return exists;
}

bool platform_directory_exists(const utf8 *path)
{
	char buffer[MAX_PATH];
	platform_utf8_to_multibyte(path, buffer, MAX_PATH);
	struct stat dirinfo;
	int result = stat(buffer, &dirinfo);
	log_verbose("checking dir %s, result = %d, is_dir = %d", buffer, result, S_ISDIR(dirinfo.st_mode));
	if ((result != 0) || !S_ISDIR(dirinfo.st_mode))
	{
		return false;
	}
	return true;
}

bool platform_original_game_data_exists(const utf8 *path)
{
	log_verbose("platform_original_game_data_exists(%s)", path);
	char buffer[MAX_PATH];
	platform_utf8_to_multibyte(path, buffer, MAX_PATH);
	char checkPath[MAX_PATH];
	safe_strcpy(checkPath, buffer, MAX_PATH);
	safe_strcat_path(checkPath, "Data", MAX_PATH);
	safe_strcat_path(checkPath, "g1.dat", MAX_PATH);
	return platform_file_exists(checkPath);
}

static mode_t getumask()
{
	//mode_t mask = umask(0);
	//umask(mask);
	//return 0777 & ~mask; // Keep in mind 0777 is octal
	return 777;
}

bool platform_ensure_directory_exists(const utf8 *path)
{
	mode_t mask = getumask();
	char buffer[MAX_PATH];
	platform_utf8_to_multibyte(path, buffer, MAX_PATH);

	log_verbose("Create directory: %s", buffer);
	for (char *p = buffer + 1; *p != '\0'; p++) {
		if (*p == '/') {
			// Temporarily truncate
			*p = '\0';

			log_verbose("mkdir(%s)", buffer);
			/*if (mkdir(buffer, mask) != 0) {
				if (errno != EEXIST) {
					return false;
				}
			}*/

			// Restore truncation
			*p = '/';
		}
	}

	log_verbose("mkdir(%s)", buffer);
	/*if (mkdir(buffer, mask) != 0) {
		if (errno != EEXIST) {
			return false;
		}
	}*/

	return true;
}

bool platform_directory_delete(const utf8 *path)
{
	/*log_verbose("Recursively deleting directory %s", path);

	FTS *ftsp;
	FTSENT *p, *chp;

	// fts_open only accepts non const paths, so we have to take a copy
	char* ourPath = _strdup(path);

	utf8* const patharray[2] = {ourPath, NULL};
	if ((ftsp = fts_open(patharray, FTS_COMFOLLOW | FTS_LOGICAL | FTS_NOCHDIR, NULL)) == NULL) {
		log_error("fts_open returned NULL");
		free(ourPath);
		return false;
	}

	chp = fts_children(ftsp, 0);
	if (chp == NULL) {
		log_verbose("No files to traverse, deleting directory %s", path);
		if (remove(path) != 0)
		{
			log_error("Failed to remove %s, errno = %d", path, errno);
		}
		free(ourPath);
		return true; // No files to traverse
	}

	while ((p = fts_read(ftsp)) != NULL) {
		switch (p->fts_info) {
			case FTS_DP: // Directory postorder, which means
						 // the directory is empty

			case FTS_F:  // File
				if(remove(p->fts_path)) {
					log_error("Could not remove %s", p->fts_path);
					fts_close(ftsp);
					free(ourPath);
					return false;
				}
				break;
			case FTS_ERR:
				log_error("Error traversing %s", path);
				fts_close(ftsp);
				free(ourPath);
				return false;
		}
	}

	free(ourPath);
	fts_close(ftsp);

	return true;*/
	return false;
}

bool platform_lock_single_instance()
{
	/*char pidFilePath[MAX_PATH];

	safe_strcpy(pidFilePath, _userDataDirectoryPath, sizeof(pidFilePath));
	safe_strcat_path(pidFilePath, SINGLE_INSTANCE_MUTEX_NAME, sizeof(pidFilePath));

	// We will never close this file manually. The operating system will
	// take care of that, because flock keeps the lock as long as the
	// file is open and closes it automatically on file close.
	// This is intentional.
	int pidFile = open(pidFilePath, O_CREAT | O_RDWR, 0666);

	if (pidFile == -1) {
		log_warning("Cannot open lock file for writing.");
		return false;
	}
	if (flock(pidFile, LOCK_EX | LOCK_NB) == -1) {
		if (errno == EWOULDBLOCK) {
			log_warning("Another OpenRCT2 session has been found running.");
			return false;
		}
		log_error("flock returned an uncatched errno: %d", errno);
		return false;
	}*/
	return true;
}

typedef struct enumerate_file_info {
	char active;
	char pattern[MAX_PATH];
	struct dirent **fileListTemp;
	char **paths;
	int cnt;
	int handle;
	void* data;
} enumerate_file_info;
static enumerate_file_info _enumerateFileInfoList[8] = { 0 };

char *g_file_pattern;

static int winfilter(const struct dirent *d)
{
	/*int entry_length = strnlen(d->d_name, MAX_PATH);
	char *name_upper = malloc(entry_length + 1);
	if (name_upper == NULL)
	{
		log_error("out of memory");
		return 0;
	}
	for (int i = 0; i < entry_length; i++)
	{
		name_upper[i] = (char)toupper(d->d_name[i]);
	}
	name_upper[entry_length] = '\0';
	bool match = fnmatch(g_file_pattern, name_upper, FNM_PATHNAME) == 0;
	//log_verbose("trying matching filename %s, result = %d", name_upper, match);
	free(name_upper);
	return match;*/
	return 0;
}

int platform_enumerate_files_begin(const utf8 *pattern)
{
	/*char npattern[MAX_PATH];
	platform_utf8_to_multibyte(pattern, npattern, MAX_PATH);
	enumerate_file_info *enumFileInfo;
	log_verbose("begin file search, pattern: %s", npattern);

	char *file_name = strrchr(npattern, *PATH_SEPARATOR);
	char *dir_name;
	if (file_name != NULL)
	{
		dir_name = strndup(npattern, file_name - npattern);
		file_name = &file_name[1];
	} else {
		file_name = npattern;
		dir_name = strdup(".");
	}


	int pattern_length = strlen(file_name);
	g_file_pattern = strndup(file_name, pattern_length);
	for (int j = 0; j < pattern_length; j++)
	{
		g_file_pattern[j] = (char)toupper(g_file_pattern[j]);
	}
	log_verbose("looking for file matching %s", g_file_pattern);
	int cnt;
	for (int i = 0; i < countof(_enumerateFileInfoList); i++) {
		enumFileInfo = &_enumerateFileInfoList[i];
		if (!enumFileInfo->active) {
			safe_strcpy(enumFileInfo->pattern, npattern, sizeof(enumFileInfo->pattern));
			cnt = scandir(dir_name, &enumFileInfo->fileListTemp, winfilter, alphasort);
			if (cnt < 0)
			{
				break;
			}
			log_verbose("found %d files matching in dir '%s'", cnt, dir_name);
			enumFileInfo->cnt = cnt;
			enumFileInfo->paths = malloc(cnt * sizeof(char *));
			char **paths = enumFileInfo->paths;
			// 256 is size of dirent.d_name
			const int dir_name_len = strnlen(dir_name, MAX_PATH);
			for (int idx = 0; idx < cnt; idx++)
			{
				struct dirent *d = enumFileInfo->fileListTemp[idx];
				const int entry_len = strnlen(d->d_name, MAX_PATH);
				// 1 for separator, 1 for trailing null
				size_t path_len = sizeof(char) * min(MAX_PATH, entry_len + dir_name_len + 2);
				paths[idx] = malloc(path_len);
				log_verbose("dir_name: %s", dir_name);
				safe_strcpy(paths[idx], dir_name, path_len);
				safe_strcat_path(paths[idx], d->d_name, path_len);
				log_verbose("paths[%d] = %s", idx, paths[idx]);
			}
			enumFileInfo->handle = 0;
			enumFileInfo->active = 1;
			free(dir_name);
			free(g_file_pattern);
			g_file_pattern = NULL;
			return i;
		}
	}

	free(dir_name);
	free(g_file_pattern);
	g_file_pattern = NULL;*/
	return -1;
}

bool platform_enumerate_files_next(int handle, file_info *outFileInfo)
{

	/*if (handle < 0)
	{
		return false;
	}
	enumerate_file_info *enumFileInfo = &_enumerateFileInfoList[handle];
	bool result;

	if (enumFileInfo->handle < enumFileInfo->cnt) {
		result = true;
	} else {
		result = false;
	}

	if (result) {
		int entryIdx = enumFileInfo->handle++;
		struct stat fileInfo;
		log_verbose("trying handle %d", entryIdx);
		char *fileName = enumFileInfo->paths[entryIdx];
		int statRes;
		statRes = stat(fileName, &fileInfo);
		if (statRes == -1) {
			log_error("failed to stat file '%s'! errno = %i", fileName, errno);
			return false;
		}
		outFileInfo->path = basename(fileName);
		outFileInfo->size = fileInfo.st_size;
		outFileInfo->last_modified = fileInfo.st_mtime;
		return true;
	} else {
		return false;
	}*/
	return false;
}

void platform_enumerate_files_end(int handle)
{
	if (handle < 0)
	{
		return;
	}
	enumerate_file_info *enumFileInfo = &_enumerateFileInfoList[handle];
	int cnt = enumFileInfo->cnt;
	for (int i = 0; i < cnt; i++) {
		free(enumFileInfo->fileListTemp[i]);
		free(enumFileInfo->paths[i]);
	}
	free(enumFileInfo->fileListTemp);
	free(enumFileInfo->paths);
	// FIXME: this here could have a bug
	enumFileInfo->fileListTemp = NULL;
	enumFileInfo->handle = 0;
	enumFileInfo->active = 0;
}

static int dirfilter(const struct dirent *d)
{
	if (d->d_name[0] == '.') {
		return 0;
	}
#if defined(_DIRENT_HAVE_D_TYPE) || defined(DT_UNKNOWN)
	if (d->d_type == DT_DIR || d->d_type == DT_LNK)
	{
		return 1;
	} else {
		return 0;
	}
#else
#error implement dirfilter!
#endif // defined(_DIRENT_HAVE_D_TYPE) || defined(DT_UNKNOWN)
}

int platform_enumerate_directories_begin(const utf8 *directory)
{
	/*char npattern[MAX_PATH];
	int length = platform_utf8_to_multibyte(directory, npattern, MAX_PATH) + 1;
	enumerate_file_info *enumFileInfo;
	log_verbose("begin directory listing, path: %s", npattern);

	// TODO: add some checking for stringness and directoryness

	int cnt;
	for (int i = 0; i < countof(_enumerateFileInfoList); i++) {
		enumFileInfo = &_enumerateFileInfoList[i];
		if (!enumFileInfo->active) {
			safe_strcpy(enumFileInfo->pattern, npattern, length);
			cnt = scandir(npattern, &enumFileInfo->fileListTemp, dirfilter, alphasort);
			if (cnt < 0)
			{
				break;
			}
			log_verbose("found %d files in dir '%s'", cnt, npattern);
			enumFileInfo->cnt = cnt;
			enumFileInfo->paths = malloc(cnt * sizeof(char *));
			char **paths = enumFileInfo->paths;
			// 256 is size of dirent.d_name
			const int dir_name_len = strnlen(npattern, MAX_PATH);
			for (int idx = 0; idx < cnt; idx++)
			{
				struct dirent *d = enumFileInfo->fileListTemp[idx];
				const int entry_len = strnlen(d->d_name, MAX_PATH);
				// 1 for separator, 1 for trailing null
				size_t path_len = sizeof(char) * min(MAX_PATH, entry_len + dir_name_len + 2);
				paths[idx] = malloc(path_len);
				log_verbose("dir_name: %s", npattern);
				safe_strcpy(paths[idx], npattern, path_len);
				safe_strcat_path(paths[idx], d->d_name, path_len);
				log_verbose("paths[%d] = %s", idx, paths[idx]);
			}
			enumFileInfo->handle = 0;
			enumFileInfo->active = 1;
			return i;
		}
	}*/

	return -1;
}

bool platform_enumerate_directories_next(int handle, utf8 *path)
{
	/*if (handle < 0)
	{
		return false;
	}

	bool result;
	enumerate_file_info *enumFileInfo = &_enumerateFileInfoList[handle];

	log_verbose("handle = %d", handle);
	if (enumFileInfo->handle < enumFileInfo->cnt) {
		result = true;
	} else {
		result = false;
	}

	if (result) {
		int entryIdx = enumFileInfo->handle++;
		struct stat fileInfo;
		char *fileName = enumFileInfo->paths[entryIdx];
		int statRes;
		statRes = stat(fileName, &fileInfo);
		if (statRes == -1) {
			log_error("failed to stat file '%s'! errno = %i", fileName, errno);
			return false;
		}
		// so very, very wrong
		safe_strcpy(path, basename(fileName), MAX_PATH);
		path_end_with_separator(path, MAX_PATH);
		return true;
	} else {
		return false;
	}*/
	return false;
}

void platform_enumerate_directories_end(int handle)
{
	if (handle < 0)
	{
		return;
	}
	enumerate_file_info *enumFileInfo = &_enumerateFileInfoList[handle];
	int cnt = enumFileInfo->cnt;
	for (int i = 0; i < cnt; i++) {
		free(enumFileInfo->fileListTemp[i]);
		free(enumFileInfo->paths[i]);
	}
	free(enumFileInfo->fileListTemp);
	free(enumFileInfo->paths);
	// FIXME: this here could have a bug
	enumFileInfo->fileListTemp = NULL;
	enumFileInfo->handle = 0;
	enumFileInfo->active = 0;
}

int platform_get_drives(){
	// POSIX systems do not know drives. Return 0.
	return 0;
}

bool platform_file_copy(const utf8 *srcPath, const utf8 *dstPath, bool overwrite)
{
	log_verbose("Copying %s to %s", srcPath, dstPath);

	FILE *dstFile;

 	if (overwrite) {
		dstFile = fopen(dstPath, "wb");
	} else {
		// Portability note: check your libc's support for "wbx"
		dstFile = fopen(dstPath, "wbx");
	}

	if (dstFile == NULL) {
		if (errno == EEXIST) {
			log_warning("platform_file_copy: Not overwriting %s, because overwrite flag == false", dstPath);
			return false;
		}

		log_error("Could not open destination file %s for copying", dstPath);
		return false;
	}

	// Open both files and check whether they are opened correctly
	FILE *srcFile = fopen(srcPath, "rb");
	if (srcFile == NULL) {
		fclose(dstFile);
		log_error("Could not open source file %s for copying", srcPath);
		return false;
	}

	size_t amount_read = 0;
	size_t file_offset = 0;

	// Copy file in FILE_BUFFER_SIZE-d chunks
	char* buffer = (char*) malloc(FILE_BUFFER_SIZE);
	while ((amount_read = fread(buffer, FILE_BUFFER_SIZE, 1, srcFile))) {
		fwrite(buffer, amount_read, 1, dstFile);
		file_offset += amount_read;
	}

	// Finish the left-over data from file, which may not be a full
	// FILE_BUFFER_SIZE-d chunk.
	fseek(srcFile, file_offset, SEEK_SET);
	amount_read = fread(buffer, 1, FILE_BUFFER_SIZE, srcFile);
	fwrite(buffer, amount_read, 1, dstFile);

	fclose(srcFile);
	fclose(dstFile);
	free(buffer);

	return true;
}

bool platform_file_move(const utf8 *srcPath, const utf8 *dstPath)
{
	return rename(srcPath, dstPath) == 0;
}

bool platform_file_delete(const utf8 *path)
{
	int ret = unlink(path);
	return ret == 0;
}

static wchar_t *regular_to_wchar(const char* src)
{
	int len = strnlen(src, MAX_PATH);
	wchar_t *w_buffer = malloc((len + 1) * sizeof(wchar_t));
	mbtowc (NULL, NULL, 0);  /* reset mbtowc */

	int max = len;
	int i = 0;
	while (max > 0)
	{
		int length;
		length = mbtowc(&w_buffer[i], &src[i], max);
		if (length < 1)
		{
			w_buffer[i + 1] = '\0';
			break;
		}
		i += length;
		max -= length;
	}
	return w_buffer;
}

/**
 * Default directory fallback is:
 *   - (command line argument)
 *   - <platform dependent>
 */
void platform_resolve_user_data_path()
{
	safe_strcpy(_userDataDirectoryPath, "/usr_data", MAX_PATH);
	/*if (gCustomUserDataPath[0] != 0) {
		if (!platform_ensure_directory_exists(gCustomUserDataPath)) {
			log_error("Failed to create directory \"%s\", make sure you have permissions.", gCustomUserDataPath);
			return;
		}
		char *path;
		if ((path = realpath(gCustomUserDataPath, NULL)) == NULL) {
			log_error("Could not resolve path \"%s\"", gCustomUserDataPath);
			return;
		}

		safe_strcpy(_userDataDirectoryPath, path, MAX_PATH);
		free(path);

		// Ensure path ends with separator
		path_end_with_separator(_userDataDirectoryPath, MAX_PATH);
		log_verbose("User data path resolved to: %s", _userDataDirectoryPath);
		if (!platform_directory_exists(_userDataDirectoryPath)) {
			log_error("Custom user data directory %s does not exist", _userDataDirectoryPath);
		}
		return;
	}

	char buffer[MAX_PATH];
	log_verbose("buffer = '%s'", buffer);

	const char *homedir = getpwuid(getuid())->pw_dir;
	platform_posix_sub_user_data_path(buffer, MAX_PATH, homedir);

	log_verbose("OpenRCT2 user data directory = '%s'", buffer);
	int len = strnlen(buffer, MAX_PATH);
	wchar_t *w_buffer = regular_to_wchar(buffer);
	w_buffer[len] = '\0';
	utf8 *path = widechar_to_utf8(w_buffer);
	free(w_buffer);
	safe_strcpy(_userDataDirectoryPath, path, MAX_PATH);
	free(path);
	log_verbose("User data path resolved to: %s", _userDataDirectoryPath);*/
}

void platform_get_openrct_data_path(utf8 *outPath, size_t outSize)
{
	safe_strcpy(outPath, _openrctDataDirectoryPath, outSize);
}

/**
 * Default directory fallback is:
 *   - (command line argument)
 *   - <exePath>/data
 *   - <platform dependent>
 */
void platform_resolve_openrct_data_path()
{
	safe_strcpy(_openrctDataDirectoryPath, "/openrct2", MAX_PATH);
	/*if (gCustomOpenrctDataPath[0] != 0) {
		// NOTE: second argument to `realpath` is meant to either be NULL or `PATH_MAX`-sized buffer,
		// since our `MAX_PATH` macro is set to some other value, pass NULL to have `realpath` return
		// a `malloc`ed buffer.
		char *resolved_path = realpath(gCustomOpenrctDataPath, NULL);
		if (resolved_path == NULL) {
			log_error("Could not resolve path \"%s\", errno = %d", gCustomOpenrctDataPath, errno);
			return;
		} else {
			safe_strcpy(_openrctDataDirectoryPath, resolved_path, MAX_PATH);
			free(resolved_path);
		}

		path_end_with_separator(_openrctDataDirectoryPath, MAX_PATH);
		return;
	}

	char buffer[MAX_PATH];
	platform_get_exe_path(buffer, sizeof(buffer));

	safe_strcat_path(buffer, "data", MAX_PATH);
	log_verbose("Looking for OpenRCT2 data in %s", buffer);
	if (platform_directory_exists(buffer))
	{
		_openrctDataDirectoryPath[0] = '\0';
		safe_strcpy(_openrctDataDirectoryPath, buffer, MAX_PATH);
		log_verbose("Found OpenRCT2 data in %s", _openrctDataDirectoryPath);
		return;
	}

	platform_posix_sub_resolve_openrct_data_path(_openrctDataDirectoryPath, sizeof(_openrctDataDirectoryPath));
	log_verbose("Trying to use OpenRCT2 data in %s", _openrctDataDirectoryPath);*/
}

void platform_get_user_directory(utf8 *outPath, const utf8 *subDirectory, size_t outSize)
{
	safe_strcpy(_userDataDirectoryPath, "/usr_dir", MAX_PATH);
	/*char buffer[MAX_PATH];
	safe_strcpy(buffer, _userDataDirectoryPath, sizeof(buffer));
	if (subDirectory != NULL && subDirectory[0] != 0) {
		log_verbose("adding subDirectory '%s'", subDirectory);
		safe_strcat_path(buffer, subDirectory, sizeof(buffer));
		path_end_with_separator(buffer, sizeof(buffer));
	}
	int len = strnlen(buffer, MAX_PATH);
	wchar_t *w_buffer = regular_to_wchar(buffer);
	w_buffer[len] = '\0';
	utf8 *path = widechar_to_utf8(w_buffer);
	free(w_buffer);
	safe_strcpy(outPath, path, outSize);
	free(path);
	log_verbose("outPath + subDirectory = '%s'", buffer);*/
}

time_t platform_file_get_modified_time(const utf8* path){
	struct stat buf;
	if (stat(path, &buf) == 0) {
		return buf.st_mtime;
	}
	return 100;
}

uint8 platform_get_locale_temperature_format(){
	// LC_MEASUREMENT is GNU specific.
	/*#ifdef LC_MEASUREMENT
	const char *langstring = setlocale(LC_MEASUREMENT, "");
	#else
	const char *langstring = setlocale(LC_ALL, "");
	#endif

	if(langstring != NULL){
		if (!fnmatch("*_US*", langstring, 0) ||
			!fnmatch("*_BS*", langstring, 0) ||
			!fnmatch("*_BZ*", langstring, 0) ||
			!fnmatch("*_PW*", langstring, 0))
		{
			return TEMPERATURE_FORMAT_F;
		}
	}*/
	return TEMPERATURE_FORMAT_C;
}

datetime64 platform_get_datetime_now_utc()
{
	const datetime64 epochAsTicks = 621355968000000000;

	struct timeval tv;
	gettimeofday(&tv, NULL);

	// Epoch starts from: 1970-01-01T00:00:00Z
	// Convert to ticks from 0001-01-01T00:00:00Z
	uint64 utcEpochTicks = (uint64)tv.tv_sec * 10000000ULL + tv.tv_usec * 10;
	datetime64 utcNow = epochAsTicks + utcEpochTicks;
	return utcNow;
}

utf8* platform_get_username() {
	/*struct passwd* pw = getpwuid(getuid());

	if (pw) {
		return pw->pw_name;
	} else {*/
		return NULL;
	//}
}

void platform_init_window_icon()
{
	// TODO Create a surface with the window icon
	// SDL_SetWindowIcon(gWindow, iconSurface)
}

#endif
