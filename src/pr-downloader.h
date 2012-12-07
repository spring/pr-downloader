/*
	header for pr-downloader
*/


#ifndef PR_DOWNLOADER_H
#define PR_DOWNLOADER_H

enum category {
	CAT_MAP,
	CAT_GAME,
	CAT_ENGINE,
	CAT_ANY
};

enum downloadtype {
	DL_RAPID,
	DL_HTTP,
	DL_PLASMA,
	DL_ENGINE, //FIXME: also in category
	DL_ANY
};

#define NAME_LEN 1024
struct downloadInfo {
	char filename[NAME_LEN];
	bool validated;
	int speed;
	downloadtype type;
	category cat;
};
/**
	downloads all downloads that where added with @DownloadAdd
	clears search results
*/
extern bool DownloadStart();

/**
	adds a download, see @DownloadSearch & @DownloadGetSearchInfo
*/
extern bool DownloadAdd(unsigned int id);
/**
* search for name
* calling this will overwrite results from the last call
* @return count of results
* @see downloadSearchGetId
*/
extern int DownloadSearch(downloadtype type, category cat, const char* name);

/**
*	get info about a search result
*/
extern bool DownloadGetSearchInfo(int id, downloadInfo& info);

/**
*	Initialize the lib
*/
extern void DownloadInit();
/**
*	shut down the lib
*/
extern void DownloadShutdown();

enum CONFIG {
	CONFIG_FILESYSTEM_WRITEPATH = 1,
};

/**
*	Set an option string
*/
extern bool DownloadSetConfig(CONFIG type, const void* value);

/**
* returns config value, NULL when failed
*/
extern bool DownloadGetConfig(CONFIG type, const void** value);

/**
* validate rapid pool
*/
extern bool DownloadRapidValidate();

/**
* dump contents of a sdp
*/
extern bool DownloadDumpSDP(const char* path);

#endif

