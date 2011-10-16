#include "Downloader/Download.h"
#include <string>
#include <list>
#include <stdio.h>
#include "Logger.h"
#include "FileSystem/IHash.h"

IDownload::IDownload(const std::string& name, category cat)
{
	this->name=name;
	this->cat=cat;
	this->downloaded=false;
	this->hash=NULL;
	this->size=-1;
}

IDownload::~IDownload(){
	for(unsigned i=0; i<pieces.size(); i++){
		delete pieces[i].sha;
	}
	pieces.clear();
}

const std::string IDownload::getCat(category cat)
{
	const char* cats[]= {"none","maps","mods","luawidgets","aibots","lobbyclients","media","other","replays","springinstallers","tools"};
	return cats[cat];
}

const std::string IDownload::getUrl()
{
	const std::string empty="";
	if (!mirror.empty())
		return mirror.front();
	return empty;
}

const std::string& IDownload::getMirror(const int i)
{
	int pos=0;
	std::list<std::string>::iterator it;
	for(it=mirror.begin(); it!=mirror.end(); ++it) {
		if(pos==i)
			return *it;
		pos++;
	}
	LOG_ERROR("Invalid index in getMirror: %d\n", i);
	return mirror.front();
}

int IDownload::getMirrorCount()
{
	return mirror.size();
}
