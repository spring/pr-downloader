/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "Download.h"
#include "Logger.h"
#include "FileSystem/IHash.h"
#include "FileSystem/File.h"
#include "Mirror.h"
#include "IDownloadsObserver.h"

#include <string>
#include <list>
#include <stdio.h>

IDownloadsObserver* observer=NULL;

void SetDownloadsObserver(IDownloadsObserver* ob)
{
	observer=ob;
}

void ObserverAdd(IDownload* id)
{
	if(observer!=NULL)
		observer->Add(id);
}

void ObserverRemove(IDownload* id)
{
	if(observer!=NULL)
		observer->Remove(id);
}

IDownload::IDownload(const std::string& name,const std::string& origin_name, category cat):
	cat(cat),
	name(name),
	origin_name(origin_name),
	downloaded(false),
	piecesize(0),
	hash(NULL),
	file(NULL),
	size(-1),
	state(IDownload::STATE_NONE)
{
	ObserverAdd(this);
}

IDownload::~IDownload()
{
	ObserverRemove(this);

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
	if (file!=NULL) {
		delete file;
		file = NULL;
	}
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

Mirror* IDownload::getMirror(unsigned i) const
{
	assert(i<mirrors.size());
	return mirrors[i];
}

Mirror* IDownload::getFastestMirror()
{
	int max=-1;
	int pos=-1;
	for(unsigned i=0; i<mirrors.size(); i++) {
		if(mirrors[i]->status==Mirror::STATUS_UNKNOWN) { //prefer mirrors with unknown status
			mirrors[i]->status=Mirror::STATUS_OK; //set status to ok, to not use it again
			LOG_DEBUG("Mirror %d: status unknown", i);
			return mirrors[i];
		}
		if ((mirrors[i]->status!=Mirror::STATUS_BROKEN) && (mirrors[i]->maxspeed>max)) {
			max=mirrors[i]->maxspeed;
			pos=i;
		}
		LOG_DEBUG("Mirror %d: (%d): %s", i, mirrors[i]->maxspeed, mirrors[i]->url.c_str());
	}
	if (pos<0) {
		LOG_DEBUG("no mirror selected");
		return NULL;
	}
	LOG_DEBUG("Fastest mirror %d: (%d): %s", pos, mirrors[pos]->maxspeed, mirrors[pos]->url.c_str());
	return mirrors[pos];
}

int IDownload::getMirrorCount() const
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

unsigned int IDownload::getProgress() const
{
	if (file==NULL)
		return -1;
	int res = 0;
	if(pieces.empty()) {
		res=file->GetPieceSize();
	} else {
		for(unsigned i=0; i<pieces.size(); i++) {
			switch(pieces[i].state) {
			case IDownload::STATE_FINISHED:
				res += file->GetPieceSize(i);
				break;
			case IDownload::STATE_DOWNLOADING:
				res += file->GetPiecePos(i);
				break;
			default:
				break;
			}
		}
	}
	return res;
}


