/*
	header for pr-downloader	
*/

enum category{
	CAT_NONE=0,
	CAT_MAP,
	CAT_GAME,
	CAT_LUAWIDGET,
	CAT_AIBOT,
	CAT_LOBBYCLIENT,
	CAT_MEDIA,
	CAT_OTHER,
	CAT_REPLAY,
	CAT_SPRINGINSTALLER,
	CAT_TOOL
}
/**
	@return download id or <0 on error
*/
extern int downloadGame(char* name, char* springhash);
extern int downloadMap(char* name,char* springhash);

extern char* getErrorMsg(int errorid);

/**
	@return count of current downloads
*/
extern int downloadGetCount();
/**
	@return returns the download id
*/
extern int downloadGetId(int idx);

/**
	deletes finished downloads from download list
	@return if true is returned, download ids
*/
extern bool downloadCleanup();
/**
	@return true if a download is finished
*/
extern bool downloadFinished(int downloadid);

/**
	@return output filename of a download, returns NULL if unknown
*/
extern char* downloadFileName(int downloadid);

/**
	cancels a download
*/
extern void downloadCancel(int downloadid);
extern int downloadGetSize(int downloadid);
extern int downloadGetDownloaded(int downloadid);


