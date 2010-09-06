#ifndef I_DOWNLOADER_H
#define I_DOWNLOADER_H

#include <list>
#include <string>

#include <stdio.h>

class IDownload{
public:
	IDownload(const std::string& url, const std::string& filename);
	/**
		add a mirror to the download specified
	*/
	bool addMirror(const std::string& url);
	bool addDepend(const std::string& depend);
	std::list<std::string> mirror;
	std::string url; //url to download
	std::string name; //name, in most cases the filename to save to
	std::string depend;
};

class IDownloader{
protected:
	std::list<IDownload*> downloads;
public:
	static IDownloader* GetHttpInstance() {
		return httpdl;
	}
	static IDownloader* GetRapidInstance() {
		return rapiddl;
	}
	static IDownloader* GetPlasmaInstance() {
		return plasmadl;
	}
	static IDownloader* GetTorrentInstance() {
		return  torrentdl;
	}

	//IDownloader(){};
	//Initialize all Downloaders
	static void Initialize();
	//Shutdown all Downloaders
	static void Shutdown();
	virtual ~IDownloader(){};

	/**
		start specific download, or start all if parameter = NULL
		download could also be a new download
	*/
	virtual void start(IDownload* download = NULL)=0;

	/**
		add a download to the download list
	*/
	virtual const IDownload* addDownload(const std::string& url, const std::string& filename="")=0;

	/**
		remove the specified download
	*/
	virtual bool removeDownload(IDownload& download)=0;
	/**
		search for a string at the downloader
	*/
	virtual std::list<IDownload>* search(const std::string& name="")=0;

private:
	static IDownloader* httpdl;
	static IDownloader* plasmadl;
	static IDownloader* rapiddl;
	static IDownloader* torrentdl;

};

#define httpDownload IDownloader::GetHttpInstance()
#define plasmaDownload IDownloader::GetPlasmaInstance()
#define rapidDownload IDownloader::GetRapidInstance()
#define torrentDownload IDownloader::GetTorrentInstance()


#endif
