#ifndef I_DOWNLOADER_H
#define I_DOWNLOADER_H

#include <list>
#include <string>

#include <stdio.h>
#include "pr-downloader/Download.h"
#define PR_DOWNLOADER_AGENT "pr-downloader/0.1"


class IDownloader{
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
	virtual ~IDownloader(){};

	/**
		download specificed download
		@return returns true, when download was successfull
	*/
	virtual bool download(IDownload& dl)=0;
	/**
		download all downloads in list
		@return returns true, when all downloads were successfull
	*/
	bool download(std::list<IDownload>& download){
		std::list<IDownload>::iterator it;
		bool res=true;
		for (it=download.begin();it!=download.end();++it){
			if (!(*it).downloaded) //don't download twice
				(*it).downloaded=this->download(*it);
			if (!(*it).downloaded){
				res=false;
			}
		}
		return res;
	}



	/**
		search for a string at the downloader
	*/
	virtual bool search(std::list<IDownload>& result, const std::string& name="", const IDownload::category=IDownload::CAT_NONE)=0;

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
