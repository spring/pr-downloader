
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
}

void IDownloader::Shutdown(){
	delete(httpdl);
	delete(plasmadl);
	delete(rapiddl);
	delete(torrentdl);
}

IDownloader* IDownloader::GetHttpInstance(){
	if (httpdl==NULL)
		httpdl=new CHttpDownloader();
	return httpdl;
}
IDownloader* IDownloader::GetRapidInstance(){
	if (rapiddl==NULL)
		rapiddl=new CRapidDownloader();
	return rapiddl;
}
IDownloader* IDownloader::GetPlasmaInstance(){
	if (plasmadl==NULL)
		plasmadl=new CPlasmaDownloader();
	return plasmadl;
}
IDownloader* IDownloader::GetTorrentInstance(){
	if (torrentdl==NULL)
		torrentdl=new CTorrentDownloader();
	return torrentdl;
}
IDownloader* IDownloader::GetWidgetInstance(){
	if (widgetdl==NULL)
		widgetdl=new CWidgetDownloader();
	return widgetdl;
}

