#include "pr-downloader/Download.h"
#include <string>
#include <list>
#include <stdio.h>
#include <sstream>

IDownload::IDownload(const std::string& name, category cat)
{
	this->name=name;
	this->cat=cat;
	this->downloaded=false;
	for(int i=0; i<sizeof(md5);i++)
		md5[i]=0;
}

const std::string& IDownload::getCat(category cat)
{
	const char* cats[]= {"none","maps","mods","luawidgets","aibots","lobbyclients","media","other","replays","springinstallers","tools"};
	return cats[cat];
}

const std::string& IDownload::getUrl()
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
	for(it=mirror.begin();it!=mirror.end(); it++){
		if(pos==i)
			return *it;
		pos++;
	}
	printf("invalid index in getMirror: %d\n", i);
	return mirror.front();
}

int IDownload::getMirrorCount()
{
	return mirror.size();
}

bool IDownload::getRange(std::string& range)
{
	std::list<struct piece>::iterator it;
	int i=0;
	for(it=pieces.begin();it!=pieces.end(); it++){
		if ((*it).state==STATE_NONE){
			std::ostringstream s;
			s << (int)(this->piecesize*i) <<"-"<<std::min(this->piecesize*i+1, this->size);
			range=s.str();
			return true;
		}
		i++;
	}
	return false;
}
