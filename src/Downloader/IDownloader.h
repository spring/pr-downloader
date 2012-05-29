/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef I_DOWNLOADER_H
#define I_DOWNLOADER_H

#include "Download.h"

#include <list>
#include <string>
#include <stdio.h>

class IDownloader
{
public:
	static IDownloader* GetHttpInstance();
	static IDownloader* GetRapidInstance();
	static IDownloader* GetPlasmaInstance();
	static IDownloader* GetTorrentInstance();
	static IDownloader* GetWidgetInstance();

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
	virtual bool download(IDownload* dl)=0;
	/**
		download all downloads in list
		@return returns true, when all downloads were successfull
	*/
	virtual bool download(std::list<IDownload*>& download);


	/**
	*	search for a string at the downloader
	*	NOTE: the caller has to free the list, IDownload is allocated for each result!
	*	@see freeResult
	*/
	virtual bool search(std::list<IDownload*>& result, const std::string& name="", const IDownload::category=IDownload::CAT_NONE)=0;

	/**
	*	free's a result list
	*/
	static void freeResult(std::list<IDownload*>& list);

private:
	static IDownloader* httpdl;
	static IDownloader* plasmadl;
	static IDownloader* rapiddl;
	static IDownloader* widgetdl;

};

#define httpDownload IDownloader::GetHttpInstance()
#define plasmaDownload IDownloader::GetPlasmaInstance()
#define rapidDownload IDownloader::GetRapidInstance()
#define widgetDownload IDownloader::GetWidgetInstance()


#endif
