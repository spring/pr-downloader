#ifndef I_DOWNLOADER_H
#define I_DOWNLOADER_H

#include <list>
#include <string>

#include <stdio.h>

class IDownload{
public:
	enum category{
		CAT_NONE=0,
		CAT_MAPS,
		CAT_MODS,
		CAT_LUAWIDGETS,
		CAT_AIBOTS,
		CAT_LOBBYCLIENTS,
		CAT_MEDIA,
		CAT_OTHER,
		CAT_REPLAYS,
		CAT_SPRINGINSTALLERS,
		CAT_TOOLS
	}cat;
	IDownload(const std::string& url, const std::string& filename, category cat=CAT_NONE);
	/**
		add a mirror to the download specified
	*/
	bool addMirror(const std::string& url);
	bool addDepend(const std::string& depend);
	std::list<std::string> mirror;
	std::string url; //url to download
	std::string name; //name, in most cases the filename to save to
	std::list<std::string> depend; //list of all depends
	bool downloaded; //file was downloaded?
	/**
		returns the string name of a category
	*/
	const std::string getCat(int cat){
		const char* cats[]={"none","maps","mods","luawidgets","aibots","lobbyclients","media","other","replays","springinstallers","tools"};
		return cats[cat];
	}
};

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
		download specificed downloads
		@return returns true, when download was successfull
	*/
	virtual bool download(IDownload& dl)=0;
	bool download(std::list<IDownload>& download){
		std::list<IDownload>::iterator it;
		bool res=true;
		for(it=download.begin();it!=download.end();++it){
			(*it).downloaded=this->download(*it);
			if(!(*it).downloaded){
				res=false;
			}
		}
		return res;
	}



	/**
		search for a string at the downloader
	*/
	virtual std::list<IDownload>* search(const std::string& name="", IDownload::category=IDownload::CAT_NONE)=0;

private:
	static IDownloader* httpdl;
	static IDownloader* plasmadl;
	static IDownloader* rapiddl;
	static IDownloader* torrentdl;
	static IDownloader* widgetdl;

};

#define httpDownload IDownloader::GetHttpInstance()
#define plasmaDownload IDownloader::GetPlasmaInstance()
#define rapidDownload IDownloader::GetRapidInstance()
#define torrentDownload IDownloader::GetTorrentInstance()
#define widgetDownload IDownloader::GetWidgetInstance()


#endif
