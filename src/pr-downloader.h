/*
	header for pr-downloader
*/

enum category {
	CAT_MAP,
	CAT_GAME,
};
#define NAME_LEN 1024
struct downloadInfo {
	char filename[NAME_LEN];
	bool validated;
	int speed;
};
/**
	@return download id or <0 on error
*/
extern bool download(int id);
/**
* search for name
* calling this will overwrite results from the last call
* @return count of results
* @see downloadSearchGetId
*/
extern int downloadSearch(category cat, char* name);

/**
*	get info about a search result
*/
extern bool downloadGetSearchInfo(downloadInfo* info, int id);

/**
*	Initialize the lib
*/
extern void downloadInit();
/**
*	shut down the lib
*/
extern void downloadShutdown();
/**
*	Set an option string
*/
extern bool downloadSetStr(char* name, char* value);
