#ifndef I_DOWNLOADER_H
#define I_DOWNLOADER_H

#include <list>
#include <string>

#include <stdio.h>
#include "Download.h"

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
	virtual ~IDownloader() {};

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
		search for a string at the downloader
	*/
	virtual bool search(std::list<IDownload*>& result, const std::string& name="", const IDownload::category=IDownload::CAT_NONE)=0;

private:
	static IDownloader* httpdl;
	static IDownloader* plasmadl;
	static IDownloader* rapiddl;
	static IDownloader* widgetdl;

};

#define httpDownload IDownloader::GetHttpInstance()
#define plasmaDownload IDownloader::GetPlasmaInstance()
#define rapidDownload IDownloader::GetRapidInstance()
#define torrentDownload IDownloader::GetTorrentInstance()
#define widgetDownload IDownloader::GetWidgetInstance()


#endif
