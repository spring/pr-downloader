#include "IDownloader.h"

#include "Http/HttpDownloader.h"
#include "Rapid/RapidDownloader.h"
#include "Plasma/PlasmaDownloader.h"
#include "Widget/WidgetDownloader.h"
#include "Util.h"

class IDownloader;

IDownloader* IDownloader::httpdl=NULL;
IDownloader* IDownloader::plasmadl=NULL;
IDownloader* IDownloader::rapiddl=NULL;
IDownloader* IDownloader::widgetdl=NULL;

IDownload::IDownload(const std::string& url, const std::string& name, category cat){
	this->url=url;
	this->name=name;
	this->cat=cat;
	this->downloaded=false;
}


bool IDownload::addMirror(const std::string& url){
	DEBUG_LINE("%s",url.c_str());
	this->mirror.push_back(url);
	return true;
}

bool IDownload::addDepend(const std::string& depend){
	this->depend.push_back(depend);
	return true;
}

void IDownloader::Initialize(){
}

void IDownloader::Shutdown(){
	delete(httpdl);
	delete(plasmadl);
	delete(rapiddl);
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
IDownloader* IDownloader::GetWidgetInstance(){
	if (widgetdl==NULL)
		widgetdl=new CWidgetDownloader();
	return widgetdl;
}

