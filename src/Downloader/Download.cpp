/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "Download.h"
#include "Logger.h"
#include "FileSystem/IHash.h"
#include "Mirror.h"

#include <string>
#include <list>
#include <stdio.h>

IDownload::IDownload(const std::string& name, category cat)
{
	this->name=name;
	this->cat=cat;
	this->downloaded=false;
	this->hash=NULL;
	this->size=-1;
	this->state=IDownload::STATE_NONE;
	piecesize=0;
}

IDownload::~IDownload()
{
	for(unsigned i=0; i<pieces.size(); i++) {
		delete pieces[i].sha;
	}
	pieces.clear();
	for(unsigned i=0; i<mirrors.size(); i++) {
		delete mirrors[i];
	}
	if (hash!=NULL)
		delete hash;
	hash=NULL;
}

const std::string IDownload::getCat(category cat)
{
	const char* cats[]= {"none","maps","mods","luawidgets","aibots","lobbyclients","media","other","replays","springinstallers","tools"};
	return cats[cat];
}

const std::string IDownload::getUrl()
{
	const std::string empty="";
	if (!mirrors.empty())
		return mirrors[0]->url;
	return empty;
}

Mirror* IDownload::getMirror(unsigned i)
{
	assert(i<mirrors.size());
	return mirrors[i];
}

Mirror* IDownload::getFastestMirror()
{
	int max=-1;
	int pos=-1;
	for(unsigned i=0; i<mirrors.size(); i++) {
		if(mirrors[i]->status==Mirror::STATUS_UNKNOWN) //prefer mirrors with unknown status
			return mirrors[i];
		if ((mirrors[i]->status!=Mirror::STATUS_BROKEN) && (mirrors[i]->maxspeed>max)) {
			max=mirrors[i]->maxspeed;
			pos=i;
		}
	}
	if (pos<0)
		return NULL;
	return mirrors[pos];
}

int IDownload::getMirrorCount()
{
	return mirrors.size();
}

bool IDownload::addMirror(const std::string& url)
{
	LOG_DEBUG("%s",url.c_str());
	Mirror* mirror=new Mirror(url);
	this->mirrors.push_back(mirror);
	return true;
}

bool IDownload::addDepend(const std::string& depend)
{
	this->depend.push_back(depend);
	return true;
}
