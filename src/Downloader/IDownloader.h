/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef I_DOWNLOADER_H
#define I_DOWNLOADER_H

#include "Download.h"

#include <list>
#include <string>
#include <stdio.h>

typedef void (*IDownloaderProcessUpdateListener)(int done, int size);

class IDownloader
{
public:
	static IDownloader* GetHttpInstance();
	static IDownloader* GetRapidInstance();

	/**
		Initialize all Downloaders
	*/
	static void Initialize();

	/**
		Shutdown all Downloaders
	*/
	static void Shutdown();
	virtual ~IDownloader() {}

	/**
		download specificed download
		@return returns true, when download was successfull
	*/
	virtual bool download(IDownload* dl, int max_parallel=10);
	/**
		download all downloads in list
		NOTE: either download(IDownload* dl) or download(std::list<IDownload*>& download) has to be implemented!
		@return returns true, when all downloads were successfull
	*/
	virtual bool download(std::list<IDownload*>& download, int max_parallel=10);


	/**
	*	search for a string at the downloader
	*	NOTE: the caller has to free the list, IDownload is allocated for each result!
	*	@see freeResult
	*/
	virtual bool search(std::list<IDownload*>& result, const std::string& name="", const DownloadEnum::Category=DownloadEnum::CAT_NONE)=0;

	/**
	*	free's a result list
	*/
	static void freeResult(std::list<IDownload*>& list);

	virtual bool setOption(const std::string& key, const std::string& value);
	static void setProcessUpdateListener(IDownloaderProcessUpdateListener l);
private:
	static IDownloader* httpdl;
	static IDownloader* rapiddl;

public:
	static IDownloaderProcessUpdateListener listener;
};

#define httpDownload IDownloader::GetHttpInstance()
#define rapidDownload IDownloader::GetRapidInstance()


#endif
