#include "pr-downloader/Download.h"
#include <string>
#include <list>

IDownload::IDownload(const std::string& name, category cat){
	this->name=name;
	this->cat=cat;
	this->downloaded=false;
}

const std::string& IDownload::getCat(category cat){
		const char* cats[]={"none","maps","mods","luawidgets","aibots","lobbyclients","media","other","replays","springinstallers","tools"};
		return cats[cat];
}

const std::string& IDownload::getUrl(){
	const std::string empty="";
	if (!mirror.empty())
		return mirror.front();
	return empty;
}
