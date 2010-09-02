#ifndef I_DOWNLOADER_H
#define I_DOWNLOADER_H

#include <list>
#include <string>

class IDownload{
public:
	IDownload(const std::string& url, const std::string& filename);
	/**
		add a mirror to the download specified
	*/
	bool addMirror(const std::string& url);

private:
	std::list<std::string> mirror;
	std::string url;
	std::string filename;
};

class IDownloader{
public:
	static IDownloader& GetHttpInstance() {
		return *httpdl;
	}
	static IDownloader& GetRapidInstance() {
		return *httpdl;
	}
	static IDownloader& GetPlasmaInstance() {
		return *plasmadl;
	}
	static IDownloader& GetTorrentInstance() {
		return *torrentdl;
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
	virtual const IDownload* search(const std::string& name)=0;

private:
	std::list<IDownload*> downloads;
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
