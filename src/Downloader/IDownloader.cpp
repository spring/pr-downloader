
#include "IDownloader.h"

#include "Http/HttpDownloader.h"
#include "Rapid/RapidDownloader.h"
#include "Plasma/PlasmaDownloader.h"
#include "Torrent/TorrentDownloader.h"
#include "Widget/WidgetDownloader.h"

class IDownloader;

IDownloader* IDownloader::httpdl=NULL;
IDownloader* IDownloader::plasmadl=NULL;
IDownloader* IDownloader::rapiddl=NULL;
IDownloader* IDownloader::torrentdl=NULL;
IDownloader* IDownloader::widgetdl=NULL;

IDownload::IDownload(const std::string& url, const std::string& name, category cat){
	this->url=url;
	this->name=name;
	this->cat=cat;
}


bool IDownload::addMirror(const std::string& url){
	this->mirror.push_back(url);
	return true;
}

bool IDownload::addDepend(const std::string& depend){
	this->depend.push_back(depend);
	return true;
}

const IDownload* IDownloader::addDownload(const std::string& url, const std::string& filename){
	IDownload* dl=new IDownload(url,filename);
	downloads.push_back(dl);
	return dl;
}

bool IDownloader::removeDownload(IDownload& download){
	this->downloads.remove(&download);
	return true;
}

void IDownloader::Initialize(){
 	httpdl=new CHttpDownloader();
	plasmadl=new CPlasmaDownloader();
	rapiddl=new CRapidDownloader();
	torrentdl=new CTorrentDownloader();
	widgetdl=new CWidgetDownloader();
}

void IDownloader::Shutdown(){
	delete(httpdl);
	delete(plasmadl);
	delete(rapiddl);
	delete(torrentdl);
}


