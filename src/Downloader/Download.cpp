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

IDownload::IDownload(const std::string& name,const std::string& origin_name, category cat, download_type typ ):
	cat(cat),
	dltype(typ),
	name(name),
	origin_name(origin_name),
	downloaded(false),
	piecesize(0),
	hash(NULL),
	file(NULL),
	size(-1),
	progress(0),
	state(IDownload::STATE_NONE),
	parallel_downloads(0),
	write_only_from(NULL)
{
	if(observer!=NULL)
		observer->Add(this);
}

IDownload::~IDownload()
{
	if(observer!=NULL)
		observer->Remove(this);

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

static std::map<int, std::string> categories;

void IDownload::initCategories()
{
	if (!categories.empty()) { //not initialized yet
		return;
	}
	categories[CAT_NONE] = "none";
	categories[CAT_MAPS] ="map";
	categories[CAT_GAMES] ="game";
	categories[CAT_LUAWIDGETS] ="luawidgets";
	categories[CAT_AIBOTS] = "aibots";
	categories[CAT_LOBBYCLIENTS] = "lobbyclients";
	categories[CAT_MEDIA] = "media";
	categories[CAT_OTHER] = "other";
	categories[CAT_REPLAYS] = "replays";
	categories[CAT_SPRINGINSTALLERS] = "springinstallers";
	categories[CAT_TOOLS] = "tools";
	categories[CAT_ENGINE_LINUX] = "engine_linux";
	categories[CAT_ENGINE_LINUX64] = "engine_linux64";
	categories[CAT_ENGINE_WINDOWS] =  "engine_windows";
	categories[CAT_ENGINE_MACOSX] = "engine_macosx";
	categories[CAT_COUNT] = "count";
}

const std::string IDownload::getCat(category cat)
{
	IDownload::initCategories();
	return categories[cat];
}

IDownload::category IDownload::getCatFromStr(const std::string& str)
{
	IDownload::initCategories();
	for(const auto &myPair: categories) {
		if (myPair.second == str) {
			return (IDownload::category)myPair.first;
		}
	}
	LOG_ERROR("Unknown category: %s", str.c_str());
	return CAT_NONE;
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
	if (origin_name.empty()) {
		origin_name = url;
	}
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
	if ( dltype == TYP_RAPID || dltype == TYP_HTTP )
		return progress;
	return 0;

}


