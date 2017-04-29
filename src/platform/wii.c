
#if defined(__WII__)
#include "../common.h"
#include <ctype.h>
//#include <dlfcn.h>
#include <errno.h>
//#include <fontconfig/fontconfig.h>
#include <fnmatch.h>
#include <locale.h>
#include <unistd.h> 

#include "../config.h"
#include "../localisation/language.h"
#include "../localisation/string_ids.h"
#include "../util/util.h"
#include "platform.h"

int wii_access (const char *pathname, int mode)
{
   struct stat st;

   if (stat(pathname, &st) < 0)
      return -1;
      
   return 0; //With Wii the file/dir is considered always accessible if it exists
}

// See http://syprog.blogspot.ru/2011/12/listing-loaded-shared-objects-in-linux.html
struct lmap {
	void* base_address;
	char* path;
	void* unused;
	struct lmap *next, *prev;
};

struct dummy {
	void* pointers[3];
	struct dummy* ptr;
};

typedef enum { DT_NONE, DT_KDIALOG, DT_ZENITY } dialog_type;

void platform_get_exe_path(utf8 *outPath, size_t outSize)
{
	safe_strcpy(outPath, "/openrct2.dol", outSize);
}

bool platform_check_steam_overlay_attached() {
	return false;
}

/**
* Default directory fallback is:
*   - (command line argument)
*   - $XDG_CONFIG_HOME/OpenRCT2
*   - /home/[uid]/.config/OpenRCT2
*/
void platform_posix_sub_user_data_path(char *buffer, size_t size, const char *homedir) {
	const char *configdir = getenv("XDG_CONFIG_HOME");
	log_verbose("configdir = '%s'", configdir);
	if (configdir == NULL)
	{
		log_verbose("configdir was null, used getuid, now is = '%s'", homedir);
		if (homedir == NULL)
		{
			log_fatal("Couldn't find user data directory");
			exit(-1);
			return;
		}

		safe_strcpy(buffer, homedir, size);
		safe_strcat_path(buffer, ".config", size);
	}
	else
	{
		safe_strcpy(buffer, configdir, size);
	}
	safe_strcat_path(buffer, "OpenRCT2", size);
	path_end_with_separator(buffer, size);
}

/**
* Default directory fallback is:
*   - (command line argument)
*   - <exePath>/data
*   - /usr/local/share/openrct2
*   - /var/lib/openrct2
*   - /usr/share/openrct2
*/
void platform_posix_sub_resolve_openrct_data_path(utf8 *out, size_t size) {
	static const utf8 *searchLocations[] = {
		"../share/openrct2",
#ifdef ORCT2_RESOURCE_DIR
		// defined in CMakeLists.txt
		ORCT2_RESOURCE_DIR,
#endif // ORCT2_RESOURCE_DIR
		"/usr/local/share/openrct2",
		"/var/lib/openrct2",
		"/usr/share/openrct2",
	};
	for (size_t i = 0; i < countof(searchLocations); i++)
	{
		log_verbose("Looking for OpenRCT2 data in %s", searchLocations[i]);
		if (platform_directory_exists(searchLocations[i]))
		{
			safe_strcpy(out, searchLocations[i], size);
			return;
		}
	}
}

uint16 platform_get_locale_language() {
	/*const char *langString = setlocale(LC_MESSAGES, "");
	if (langString != NULL) {
		// The locale has the following form:
		// language[_territory[.codeset]][@modifier]
		// (see https://www.gnu.org/software/libc/manual/html_node/Locale-Names.html)
		// longest on my system is 29 with codeset and modifier, so 32 for the pattern should be more than enough
		char pattern[32];
		//strip the codeset and modifier part
		int length = strlen(langString);
		{
			for (int i = 0; i < length; ++i) {
				if (langString[i] == '.' || langString[i] == '@') {
					length = i;
					break;
				}
			}
		} //end strip
		memcpy(pattern, langString, length); //copy all until first '.' or '@'
		pattern[length] = '\0';
		//find _ if present
		const char *strip = strchr(pattern, '_');
		if (strip != NULL) {
			// could also use '-', but '?' is more flexible. Maybe LanguagesDescriptors will change.
			// pattern is now "language?territory"
			pattern[strip - pattern] = '?';
		}

		// Iterate through all available languages
		for (int i = 1; i < LANGUAGE_COUNT; ++i) {
			if (!fnmatch(pattern, LanguagesDescriptors[i].locale, 0)) {
				return i;
			}
		}

		//special cases :(
		if (!fnmatch(pattern, "en_CA", 0)) {
			return LANGUAGE_ENGLISH_US;
		}
		else if (!fnmatch(pattern, "zh_CN", 0)) {
			return LANGUAGE_CHINESE_SIMPLIFIED;
		}
		else if (!fnmatch(pattern, "zh_TW", 0)) {
			return LANGUAGE_CHINESE_TRADITIONAL;
		}

		//no exact match found trying only language part
		if (strip != NULL) {
			pattern[strip - pattern] = '*';
			pattern[strip - pattern + 1] = '\0'; // pattern is now "language*"
			for (int i = 1; i < LANGUAGE_COUNT; ++i) {
				if (!fnmatch(pattern, LanguagesDescriptors[i].locale, 0)) {
					return i;
				}
			}
		}
	}*/
	return LANGUAGE_ENGLISH_UK;
}

uint8 platform_get_locale_currency() {
	char *langstring = setlocale(LC_MONETARY, "");

	if (langstring == NULL) {
		return platform_get_currency_value(NULL);
	}

	struct lconv *lc = localeconv();

	return platform_get_currency_value(lc->int_curr_symbol);
}

uint8 platform_get_locale_measurement_format() {
	// LC_MEASUREMENT is GNU specific.
/*#ifdef LC_MEASUREMENT
	const char *langstring = setlocale(LC_MEASUREMENT, "");
#else
	const char *langstring = setlocale(LC_ALL, "");
#endif

	if (langstring != NULL) {
		//using https://en.wikipedia.org/wiki/Metrication#Chronology_and_status_of_conversion_by_country as reference
		if (!fnmatch("*_US*", langstring, 0) || !fnmatch("*_MM*", langstring, 0) || !fnmatch("*_LR*", langstring, 0)) {
			return MEASUREMENT_FORMAT_IMPERIAL;
		}
	}*/
	return MEASUREMENT_FORMAT_METRIC;
}

static void execute_cmd(char *command, int *exit_value, char *buf, size_t *buf_size) {
	/*FILE *f;
	size_t n_chars;

	log_verbose("executing \"%s\"...\n", command);
	f = popen(command, "r");

	if (buf && buf_size) {
		n_chars = fread(buf, 1, *buf_size, f);

		// some commands may return a new-line terminated result, trim that
		if (n_chars > 0 && buf[n_chars - 1] == '\n') {
			buf[n_chars - 1] = '\0';
		}
		// make sure string is null-terminated
		if (n_chars == *buf_size) {
			n_chars--;
		}
		buf[n_chars] = '\0';

		// account for null terminator
		*buf_size = n_chars + 1;
	}
	else {
		fflush(f);
	}

	if (exit_value)
		*exit_value = pclose(f);
	else
		pclose(f);*/
}

static dialog_type get_dialog_app(char *cmd, size_t *cmd_size) {
	int exit_value;
	size_t size;
	dialog_type dtype;

	/*
	* prefer zenity as it offers more required features, e.g., overwrite
	* confirmation and selecting only existing files
	*/

	dtype = DT_ZENITY;
	size = *cmd_size;
	execute_cmd("which zenity", &exit_value, cmd, &size);

	if (exit_value != 0) {
		dtype = DT_KDIALOG;
		size = *cmd_size;
		execute_cmd("which kdialog", &exit_value, cmd, &size);

		if (exit_value != 0) {
			log_error("no dialog (zenity or kdialog) found\n");
			return DT_NONE;
		}
	}

	cmd[size - 1] = '\0';

	*cmd_size = size;

	return dtype;
}

bool platform_open_common_file_dialog(utf8 *outFilename, file_dialog_desc *desc, size_t outSize) {
	int exit_value;
	char executable[MAX_PATH];
	char cmd[MAX_PATH];
	char result[MAX_PATH];
	size_t size;
	dialog_type dtype;
	char *action = NULL;
	char *flags = NULL;
	char filter[MAX_PATH] = { 0 };
	char filterPatternRegex[64];

	size = MAX_PATH;
	dtype = get_dialog_app(executable, &size);

	switch (dtype) {
	case DT_KDIALOG:
		switch (desc->type) {
		case FD_OPEN:
			action = "--getopenfilename";
			break;
		case FD_SAVE:
			action = "--getsavefilename";
			break;
		}

		{
			bool first = true;
			for (int j = 0; j < countof(desc->filters); j++) {
				if (desc->filters[j].pattern && desc->filters[j].name) {
					char filterTemp[100] = { 0 };
					if (first) {
						snprintf(filterTemp, countof(filterTemp), "%s | %s", desc->filters[j].pattern, desc->filters[j].name);
						first = false;
					}
					else {
						snprintf(filterTemp, countof(filterTemp), "\\n%s | %s", desc->filters[j].pattern, desc->filters[j].name);
					}
					safe_strcat(filter, filterTemp, countof(filter));
				}
			}
			char filterTemp[100] = { 0 };
			if (first) {
				snprintf(filterTemp, countof(filterTemp), "*|%s", (char *)language_get_string(STR_ALL_FILES));
			}
			else {
				snprintf(filterTemp, countof(filterTemp), "\\n*|%s", (char *)language_get_string(STR_ALL_FILES));
			}
			safe_strcat(filter, filterTemp, countof(filter));

			// kdialog wants filters space-delimited and we don't expect ';' anywhere else,
			// this is much easier and quicker to do than being overly careful about replacing
			// it only where truly needed.
			int filterSize = strlen(filter);
			for (int i = 0; i < filterSize + 3; i++) {
				if (filter[i] == ';') {
					filter[i] = ' ';
				}
			}
		}

		snprintf(cmd, MAX_PATH, "%s --title \"%s\" %s ~ \"%s\"", executable, desc->title, action, filter);
		break;
	case DT_ZENITY:
		action = "--file-selection";
		switch (desc->type) {
		case FD_SAVE:
			flags = "--confirm-overwrite --save";
			break;
		case FD_OPEN:
			flags = "";
			break;
		}

		// Zenity seems to be case sensitive, while Kdialog isn't.
		for (int j = 0; j < countof(desc->filters); j++) {
			if (desc->filters[j].pattern && desc->filters[j].name) {
				int regexIterator = 0;
				for (int i = 0; i <= strlen(desc->filters[j].pattern); i++) {
					if (isalpha(desc->filters[j].pattern[i])) {
						filterPatternRegex[regexIterator + 0] = '[';
						filterPatternRegex[regexIterator + 1] = (char)toupper(desc->filters[j].pattern[i]);
						filterPatternRegex[regexIterator + 2] = (char)tolower(desc->filters[j].pattern[i]);
						filterPatternRegex[regexIterator + 3] = ']';
						regexIterator += 3;
					}
					else if (desc->filters[j].pattern[i] == ';') {
						filterPatternRegex[regexIterator] = ' ';
					}
					else {
						filterPatternRegex[regexIterator] = (char)desc->filters[j].pattern[i];
					}
					regexIterator++;
				}
				filterPatternRegex[regexIterator + 1] = 0;

				char filterTemp[100] = { 0 };
				snprintf(filterTemp, countof(filterTemp), " --file-filter=\"%s | %s\"", desc->filters[j].name, filterPatternRegex);
				safe_strcat(filter, filterTemp, countof(filter));
			}
		}
		char filterTemp[100] = { 0 };
		snprintf(filterTemp, countof(filterTemp), " --file-filter=\"%s | *\"", (char *)language_get_string(STR_ALL_FILES));
		safe_strcat(filter, filterTemp, countof(filter));

		snprintf(cmd, MAX_PATH, "%s %s %s --title=\"%s\" / %s", executable, action, flags, desc->title, filter);
		break;
	default: return 0;
	}

	size = MAX_PATH;
	execute_cmd(cmd, &exit_value, result, &size);

	if (exit_value != 0) {
		return 0;
	}

	result[size - 1] = '\0';
	log_verbose("filename = %s", result);

	if (desc->type == FD_OPEN && access(result, F_OK) == -1) {
		char msg[MAX_PATH];

		snprintf(msg, MAX_PATH, "\"%s\" not found: %s, please choose another file\n", result, strerror(errno));
		platform_show_messagebox(msg);

		return platform_open_common_file_dialog(outFilename, desc, outSize);
	}
	else
		if (desc->type == FD_SAVE && access(result, F_OK) != -1 && dtype == DT_KDIALOG) {
			snprintf(cmd, MAX_PATH, "%s --yesno \"Overwrite %s?\"", executable, result);

			size = MAX_PATH;
			execute_cmd(cmd, &exit_value, 0, 0);

			if (exit_value != 0) {
				return 0;
			}
		}

	safe_strcpy(outFilename, result, outSize);

	return 1;
}

utf8 *platform_open_directory_browser(utf8 *title) {
	size_t size;
	dialog_type dtype;
	int exit_value;
	char cmd[MAX_PATH];
	char executable[MAX_PATH];
	char result[MAX_PATH];
	char *return_value;

	size = MAX_PATH;
	dtype = get_dialog_app(executable, &size);

	switch (dtype) {
	case DT_KDIALOG:
		snprintf(cmd, MAX_PATH, "%s --title \"%s\" --getexistingdirectory /", executable, title);
		break;
	case DT_ZENITY:
		snprintf(cmd, MAX_PATH, "%s --title=\"%s\" --file-selection --directory /", executable, title);
		break;
	default: return 0;
	}

	size = MAX_PATH;
	execute_cmd(cmd, &exit_value, result, &size);

	if (exit_value != 0) {
		return NULL;
	}

	result[size - 1] = '\0';

	return_value = _strdup(result);

	return return_value;
}

void platform_show_messagebox(char *message) {
	size_t size;
	dialog_type dtype;
	char cmd[MAX_PATH];
	char executable[MAX_PATH];

	log_verbose(message);

	size = MAX_PATH;
	dtype = get_dialog_app(executable, &size);

	switch (dtype) {
	case DT_KDIALOG:
		snprintf(cmd, MAX_PATH, "%s --title \"OpenRCT2\" --msgbox \"%s\"", executable, message);
		break;
	case DT_ZENITY:
		snprintf(cmd, MAX_PATH, "%s --title=\"OpenRCT2\" --info --text=\"%s\"", executable, message);
		break;
	default:
		//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "OpenRCT2", message, gWindow);
		return;
	}

	size = MAX_PATH;
	execute_cmd(cmd, 0, 0, 0);
}

bool platform_get_font_path(TTFFontDescriptor *font, utf8 *buffer, size_t size)
{
	/*assert(buffer != NULL);
	assert(font != NULL);

	log_verbose("Looking for font %s with FontConfig.", font->font_name);
	FcConfig* config = FcInitLoadConfigAndFonts();
	if (!config)
	{
		log_error("Failed to initialize FontConfig library");
		FcFini();
		return false;
	}
	FcPattern* pat = FcNameParse((const FcChar8*)font->font_name);

	FcConfigSubstitute(config, pat, FcMatchPattern);
	FcDefaultSubstitute(pat);

	bool found = false;
	FcResult result = FcResultNoMatch;
	FcPattern* match = FcFontMatch(config, pat, &result);

	if (match)
	{
		FcChar8* filename = NULL;
		if (FcPatternGetString(match, FC_FILE, 0, &filename) == FcResultMatch)
		{
			found = true;
			safe_strcpy(buffer, (utf8*)filename, size);
			log_verbose("FontConfig provided font %s", filename);
		}
		FcPatternDestroy(match);
	}
	else {
		log_warning("Failed to find required font.");
	}

	FcPatternDestroy(pat);
	FcConfigDestroy(config);
	FcFini();
	return found;*/
	return false;
}

void reverse(char str[], int length)
{
	int start = 0;
	int end = length - 1;
	while (start < end)
	{
		char tmp = *(str + start);
		*(str + start) = *(str + end);
		*(str + end) = tmp;
		start++;
		end--;
	}
}

// Implementation of itoa()
char* SDL_ltoa(int num, char* str, int base)
{
	int i = 0;
	bool isNegative = false;

	/* Handle 0 explicitely, otherwise empty string is printed for 0 */
	if (num == 0)
	{
		str[i++] = '0';
		str[i] = '\0';
		return str;
	}

	// In standard itoa(), negative numbers are handled only with 
	// base 10. Otherwise numbers are considered unsigned.
	if (num < 0 && base == 10)
	{
		isNegative = true;
		num = -num;
	}

	// Process individual digits
	while (num != 0)
	{
		int rem = num % base;
		str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
		num = num / base;
	}

	// If number is negative, append '-'
	if (isNegative)
		str[i++] = '-';

	str[i] = '\0'; // Append string terminator

				   // Reverse the string
	reverse(str, i);

	return str;
}

/*
 * The DIRSIZ macro is the minimum record length which will hold the directory
 * entry.  This requires the amount of space in struct dirent without the
 * d_name field, plus enough space for the name and a terminating nul byte
 * (dp->d_namlen + 1), rounded up to a 4 byte boundary.
 */
#undef DIRSIZ
#define DIRSIZ(dp)		sizeof(struct dirent)//					\
	//((sizeof(struct dirent) - sizeof(dp)->d_name) +			\
	    //(((dp)->d_namlen + 1 + 3) &~ 3))

int
scandir(dirname, namelist, select, dcomp)
	const char *dirname;
	struct dirent ***namelist;
	int (*select) __P((const struct dirent *));
	int (*dcomp) __P((const void *, const void *));
{
	register struct dirent *d, *p, **names;
	register size_t nitems;
	struct stat stb;
	long arraysz;
	DIR *dirp;

	if ((dirp = opendir(dirname)) == NULL)
		return(-1);
	//if (fstat(dirp->dd_fd, &stb) < 0)
	//	return(-1);

	/*
	 * estimate the array size by taking the size of the directory file
	 * and dividing it by a multiple of the minimum size entry. 
	 */
	arraysz = 10;//(stb.st_size / 24);
	names = (struct dirent **)malloc(arraysz * sizeof(struct dirent *));
	if (names == NULL)
		return(-1);

	nitems = 0;
	while ((d = readdir(dirp)) != NULL) {
		if (select != NULL && !(*select)(d))
			continue;	/* just selected names */
		/*
		 * Make a minimum size copy of the data
		 */
		p = (struct dirent *)malloc(DIRSIZ(d));
		if (p == NULL)
			return(-1);
		p->d_ino = d->d_ino;
		p->d_type = d->d_type;
		//p->d_reclen = d->d_reclen;
		//p->d_namlen = d->d_namlen;
		bcopy(d->d_name, p->d_name, sizeof(p->d_name));//p->d_namlen + 1);
		/*
		 * Check to make sure the array has space left and
		 * realloc the maximum size.
		 */
		if (++nitems >= arraysz) {
			//if (fstat(dirp->dd_fd, &stb) < 0)
			//	return(-1);	/* just might have grown */
			arraysz *= 2;//stb.st_size / 12;
			names = (struct dirent **)realloc((char *)names,
				arraysz * sizeof(struct dirent *));
			if (names == NULL)
				return(-1);
		}
		names[nitems-1] = p;
	}
	closedir(dirp);
	if (nitems && dcomp != NULL)
		qsort(names, nitems, sizeof(struct dirent *), dcomp);
	*namelist = names;
	return(nitems);
}

/*
 * Alphabetic order comparison routine for those who want it.
 */
int
alphasort(d1, d2)
	const void *d1;
	const void *d2;
{
	return(strcmp((*(struct dirent **)d1)->d_name,
	    (*(struct dirent **)d2)->d_name));
}

char* strdup(const char *str)
{
	size_t siz;
	char *copy;
	siz = strlen(str) + 1;
	if ((copy = malloc(siz)) == NULL)
		return(NULL);
	(void)memcpy(copy, str, siz);
	return(copy);
}

#define SISR_ERRORMASK(chn)			(0x0f000000>>((chn)<<3))
static void sio_debug_print(const char* message)
{
	char c;
	while((c = *message++) != 0)
	{
		SI_Transfer(0, &c, 1, NULL, 0, NULL, 0);
		SI_Sync();
	}
}


void platform_print(const char* message)
{
	printf(message);
	sio_debug_print(message);
	sio_debug_print("\r\n");
}

#endif