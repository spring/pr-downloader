/*
        header for pr-downloader
*/

#ifndef PR_DOWNLOADER_H
#define PR_DOWNLOADER_H

#include "Downloader/DownloadEnum.h"

#define NAME_LEN 1024
struct downloadInfo
{
	char filename[NAME_LEN];
	bool validated;
	int speed;
	DownloadEnum::Category cat;
};
/**
        downloads all downloads that where added with @DownloadAdd
        clears search results
*/
extern int DownloadStart();

/**
        adds a download by url without searching
*/
extern int DownloadAddByUrl(DownloadEnum::Category cat, const char* filename,
			    const char* url);

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
extern int DownloadSearch(DownloadEnum::Category category, const char* name);

/**
*	get info about a result / current download
*/
extern bool DownloadGetInfo(int id, downloadInfo& info);

/**
*	Initialize the lib
*/
extern void DownloadInit();
/**
*	shut down the lib
*/
extern void DownloadShutdown();

enum CONFIG {
	CONFIG_FILESYSTEM_WRITEPATH = 1, // const char, sets the output directory
	CONFIG_FETCH_DEPENDS,		 // bool, automaticly fetch depending files
	CONFIG_RAPID_FORCEUPDATE,	// bool, always fetch repo files
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
* @param deletebroken files
*/
extern bool DownloadRapidValidate(bool deletebroken);

/**
* dump contents of a sdp
*/
extern bool DownloadDumpSDP(const char* path);

/**
* control printing to stdout
*/
extern void DownloadDisableLogging(bool disableLogging);

typedef void (*IDownloaderProcessUpdateListener)(int done, int size);

extern void SetDownloadListener(IDownloaderProcessUpdateListener listener);

/*
 * Calculate hash and return it in base64 format.
 * Accepted values for type are:
 *   0 - md5
*/
extern char* CalcHash(const char* str, int size, int type);
#endif
