/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "unitsync.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <clocale>
#include <set>
#include <dirent.h>

#include <boost/algorithm/string.hpp>
#include <iterator>
#include "c_api.h"
#include "image.h"
#include "springbundle.h"
#include "unitsync_cache.h"

#include "lslutils/config.h"
#include "lslutils/debug.h"
#include "lslutils/conversion.h"
#include "lslutils/misc.h"
#include "lslutils/globalsmanager.h"
#include "lslutils/thread.h"

#ifndef WIN32
#include <sys/stat.h>
#endif

#define LOCK_UNITSYNC boost::mutex::scoped_lock lock_criticalsection(m_lock)

#define ASYNC_LOAD 0 //FIXME: repair/set to 1!
#if ASYNC_LOAD
#define TRY_LOCK(ret)                                                   \
	boost::mutex::scoped_try_lock lock_criticalsection(m_lock);     \
	if (!lock_criticalsection.owns_lock()) {                        \
		LslDebug("Lock failed: %s:%d", __FUNCTION__, __LINE__); \
		return ret;                                             \
	}
#else
#define TRY_LOCK(ret)
#endif

#define ASSERT_EXCEPTION(cond, msg)               \
	do {                                      \
		if (!(cond)) {                    \
			LSL_THROW(unitsync, msg); \
		}                                 \
	} while (0)

namespace LSL
{

Unitsync::Unitsync()
    : m_cache_thread(new WorkerThread)
    , supportsManualUnLoad(false) //new style fetching (>= spring 101.0)
{
}


enum ASYNC_EVENTS {
	ASYNC_MAP_IMAGE_EVT = 1,
	ASYNC_MAP_IMAGE_SCALED_EVT,
	ASYNC_MAP_EX_EVT,
	ASYNC_UNITSYNC_LOADED_EVT,
};

Unitsync::~Unitsync()
{
	ClearCache();
	delete m_cache_thread;
	m_cache_thread = NULL;
	Cache::FreeInstance();
}

bool CompareStringNoCase(const std::string& first, const std::string& second)
{
	//this doesn't compile on osx
	/*
	static std::locale l("C");
	static boost::is_iless il(l);
	try {
		//this fails on certain names
		return il(first,second);
	}
	catch(...) {
*/
	return first < second;
	//	}
}

bool Unitsync::LoadUnitSyncLib(const std::string& unitsyncloc)
{
	LOCK_UNITSYNC;
	ClearCache();
	const bool ret = susynclib().Load(unitsyncloc);
	if (!ret) {
		return false;
	}
	const std::string datadir = LSL::Util::config().GetDataDir();
	const std::string curdatadir = susynclib().GetSpringDataDir();
	if (datadir != curdatadir) {
		LslWarning("Reloading unitsync due to datadir change: %s -> %s", curdatadir.c_str(), datadir.c_str());
		SetSpringDataPath(datadir);
		susynclib().Load(unitsyncloc);
	}
	supportsManualUnLoad = LSL::susynclib().GetSpringConfigInt("UnitsyncAutoUnLoadMapsIsSupported", 0) != 0;
	if (supportsManualUnLoad) {
		LslDebug("Unitsync supports manual loading of archives (faster, yey!)");
		LSL::usync().SetSpringConfigInt("UnitsyncAutoUnLoadMaps", 1);
	} else {
		LslDebug("Unitsync doesn't support manual loading of archives :-/");
	}


	m_cache_path = LSL::Util::config().GetCachePath();
	PopulateArchiveList();
	return true;
}

void Unitsync::ClearCache()
{
	m_maps_list.clear();
	m_mods_list.clear();
	m_mod_array.clear();
	m_map_array.clear();
	m_unsorted_mod_array.clear();
	m_unsorted_map_array.clear();
	lslcache.clear();
	m_map_gameoptions.clear();
	m_game_gameoptions.clear();
	m_datapaths.clear();
}

void Unitsync::FetchUnitsyncErrors(const std::string& prefix)
{
	auto errors = susynclib().GetUnitsyncErrors();
	std::string pre = prefix;
	if (!prefix.empty()) {
		pre += " ";
	}
	for (const std::string error : errors) {
		LslWarning("Unitsync: %s%s", pre.c_str(), error.c_str());
	}
}

static std::string GetGameInfo(int index, const std::string keyname)
{
	const int count = susynclib().GetPrimaryModInfoCount(index);
	for (int i = 0; i < count; i++) {
		const std::string key = Util::SafeString(susynclib().GetInfoKey(i));
		if (key == keyname) {
			return Util::SafeString(susynclib().GetInfoValueString(i));
		}
	}
	return "";
}

void Unitsync::PopulateArchiveList()
{
	GetSpringDataPaths();
	const int numMaps = susynclib().GetMapCount();
	for (int i = 0; i < numMaps; i++) {
		std::string name, archivename;
		try {
			const int count = susynclib().GetMapArchiveCount(i);
			if (count > 0) {
				archivename = susynclib().GetMapArchiveName(0);
			}
			name = susynclib().GetMapName(i);
			//PrefetchMap( name ); // DEBUG
		} catch (...) {
			continue;
		}
		assert(!name.empty());
		if (!archivename.empty())
			m_maps_archive_name[name] = archivename;
		m_map_array.push_back(name);
		FetchUnitsyncErrors(name);
	}
	const int numMods = susynclib().GetPrimaryModCount();
	for (int i = 0; i < numMods; i++) {
		std::string name, archivename;
		try {
			const int count = susynclib().GetPrimaryModArchiveCount(i);
			if (count > 0) {
				archivename = susynclib().GetPrimaryModArchive(i);
			}
			name = GetGameInfo(i, "name");
		} catch (...) {
			continue;
		}
		assert(!name.empty());
		if (!archivename.empty())
			m_mods_archive_name[name] = archivename;
		m_mod_array.push_back(name);
		FetchUnitsyncErrors(name);
	}
	m_unsorted_mod_array = m_mod_array;
	m_unsorted_map_array = m_map_array;
	std::sort(m_map_array.begin(), m_map_array.end(), &CompareStringNoCase);
	std::sort(m_mod_array.begin(), m_mod_array.end(), &CompareStringNoCase);
}

void Unitsync::FreeUnitSyncLib()
{
	LOCK_UNITSYNC;
	supportsManualUnLoad = false;
	susynclib().Unload();
}


bool Unitsync::IsLoaded() const
{
	return susynclib().IsLoaded();
}


std::string Unitsync::GetSpringVersion() const
{

	std::string ret;
	try {
		ret = susynclib().GetSpringVersion();
	} catch (...) {
	}
	return ret;
}


StringVector Unitsync::GetGameList() const
{
	TRY_LOCK(StringVector())
	return m_mod_array;
}

bool Unitsync::GameExists(const std::string& gamename, const std::string& hash)
{
	TRY_LOCK(false)
	LocalArchivesVector::const_iterator itor = m_mods_archive_name.find(gamename);
	if (itor == m_mods_archive_name.end() || itor->second.empty())
		return false;

	assert(!itor->second.empty()); //empty hashes are invalid

	if (hash.empty())
		return true;
	return GetGameHash(gamename) == hash;
}

std::string Unitsync::GetGameHash(const std::string& name)
{
	LocalArchivesVector::const_iterator itor = m_mods_list.find(name);
	if (itor != m_mods_list.end()) {
		return itor->second;
	}
	const unsigned int modhash = susynclib().GetPrimaryModChecksumFromName(name);
	assert(modhash>0);
	const std::string strhash = LSL::Util::ToUIntString(modhash);
	m_mods_list[name] = strhash;
	return strhash;
}

UnitsyncGame Unitsync::GetGame(const std::string& gamename)
{
	UnitsyncGame m;
	TRY_LOCK(m);
	m.name = gamename;
	m.hash = m_mods_list[gamename];
	return m;
}


UnitsyncGame Unitsync::GetGame(int index)
{
	UnitsyncGame m;
	TRY_LOCK(m);
	m.name = m_mod_array[index];
	m.hash = m_mods_list[m.name];
	return m;
}

StringVector Unitsync::GetMapList() const
{
	TRY_LOCK(StringVector())
	return m_map_array;
}

StringVector Unitsync::GetGameValidMapList(const std::string& gamename) const
{
	StringVector ret;
	TRY_LOCK(ret)
	try {
		unsigned int mapcount = susynclib().GetValidMapCount(gamename);
		for (unsigned int i = 0; i < mapcount; i++)
			ret.push_back(susynclib().GetValidMapName(i));
	} catch (Exceptions::unitsync& e) {
	}
	return ret;
}

bool Unitsync::MapExists(const std::string& mapname, const std::string& hash)
{
	assert(!mapname.empty());
	TRY_LOCK(false)
	LocalArchivesVector::const_iterator itor = m_maps_archive_name.find(mapname);
	if (itor == m_maps_archive_name.end())
		return false;
	assert(!itor->second.empty()); //empty hashes are invalid
	if (hash.empty())
		return true;

	return GetMapHash(mapname) == hash;
}

std::string Unitsync::GetMapHash(const std::string& name)
{
	LocalArchivesVector::const_iterator itor = m_maps_list.find(name);
	if (itor != m_maps_list.end()) {
		return itor->second;
	}
	const unsigned int modhash = susynclib().GetMapChecksumFromName(name);
	assert(modhash > 0);
	const std::string strhash = LSL::Util::ToUIntString(modhash);
	m_maps_list[name] = strhash;
	return strhash;
}

UnitsyncMap Unitsync::GetMap(int index)
{
	UnitsyncMap m;
	TRY_LOCK(m)
	if (index < 0)
		return m;
	m.name = m_map_array[index];
	m.hash = m_maps_list[m.name];
	m.info = _GetMapInfoEx(m.name);
	assert(!m.hash.empty());
	return m;
}

void GetOptionEntry(const int i, GameOptions& ret)
{
	//all section values for options are converted to lower case
	//since usync returns the key of section type keys lower case
	//otherwise comapring would be a real hassle
	const std::string key = susynclib().GetOptionKey(i);
	const std::string name = susynclib().GetOptionName(i);
	const std::string section_str = boost::algorithm::to_lower_copy(susynclib().GetOptionSection(i));
	const std::string optiondesc = susynclib().GetOptionDesc(i);
	const int opttype = susynclib().GetOptionType(i);
	switch (opttype) {
		case Enum::opt_float: {
			ret.float_map[key] = mmOptionFloat(name, key, optiondesc, susynclib().GetOptionNumberDef(i),
							   susynclib().GetOptionNumberStep(i),
							   susynclib().GetOptionNumberMin(i), susynclib().GetOptionNumberMax(i),
							   section_str);
			break;
		}
		case Enum::opt_bool: {
			ret.bool_map[key] = mmOptionBool(name, key, optiondesc, susynclib().GetOptionBoolDef(i), section_str);
			break;
		}
		case Enum::opt_string: {
			ret.string_map[key] = mmOptionString(name, key, optiondesc, susynclib().GetOptionStringDef(i), susynclib().GetOptionStringMaxLen(i), section_str);
			break;
		}
		case Enum::opt_list: {
			ret.list_map[key] = mmOptionList(name, key, optiondesc, susynclib().GetOptionListDef(i), section_str);
			const int listItemCount = susynclib().GetOptionListCount(i);
			for (int j = 0; j < listItemCount; ++j) {
				std::string descr = susynclib().GetOptionListItemDesc(i, j);
				ret.list_map[key].addItem(susynclib().GetOptionListItemKey(i, j), susynclib().GetOptionListItemName(i, j), descr);
			}
			break;
		}
		case Enum::opt_section: {
			ret.section_map[key] = mmOptionSection(name, key, optiondesc, section_str);
		}
	}
}

GameOptions Unitsync::GetMapOptions(const std::string& name)
{
	GameOptions ret;

	TRY_LOCK(ret)
	assert(!name.empty());

	auto gameOptionsItor = m_map_gameoptions.find(name);
	if (gameOptionsItor != m_map_gameoptions.end()) {
		return gameOptionsItor->second;
	}

	if (MapExists(name)) {
		const std::string filename = GetMapOptionsPath(name);
		if (!lslcache.Get(filename, ret)) {
			PrefetchMap(name);
			lslcache.Get(filename, ret);
		}
	}
	m_map_gameoptions[name] = ret;

	return ret;
}

StringVector Unitsync::GetMapDeps(const std::string& mapname)
{
	assert(!mapname.empty());
	StringVector ret;
	try {
		ret = susynclib().GetMapDeps(Util::IndexInSequence(m_unsorted_map_array, mapname));
	} catch (Exceptions::unitsync& u) {
	}
	return ret;
}

UnitsyncMap Unitsync::GetMap(const std::string& mapname)
{
	assert(!mapname.empty());
	const int i = Util::IndexInSequence(m_map_array, mapname);
	UnitsyncMap m;
	if (i < 0) {
		LSL_THROWF(unitsync, "Map does not exist: %s", mapname.c_str());
	}
	m.name = m_map_array[i];
	m.hash = GetMapHash(mapname);
	m.info = _GetMapInfoEx(m.name);
	assert(!m.hash.empty());
	return m;
}

GameOptions Unitsync::GetGameOptions(const std::string& name)
{
	assert(!name.empty());
	GameOptions ret;
	if (!GameExists(name))
		return ret;
	TRY_LOCK(ret)
	if (m_game_gameoptions.find(name) != m_game_gameoptions.end()) {
		return m_game_gameoptions[name];
	}
	GetGameHash(name);
	const std::string filename = GetGameOptionsPath(name);
	if (!lslcache.Get(filename, ret)) {
		PrefetchGame(name);
		lslcache.Get(filename, ret);
	}
	m_game_gameoptions[name] = ret;
	return ret;
}

StringVector Unitsync::GetGameDeps(const std::string& gamename) const
{
	assert(!gamename.empty());
	StringVector ret;
	TRY_LOCK(ret)
	try {
		ret = susynclib().GetModDeps(Util::IndexInSequence(m_unsorted_mod_array, gamename));
	} catch (Exceptions::unitsync& u) {
	}
	return ret;
}

StringVector Unitsync::GetSides(const std::string& gamename)
{
	assert(!gamename.empty());
	StringVector ret;
	if (!GameExists(gamename))
		return ret;
	TRY_LOCK(ret);
	const std::string cachefile = GetSidesCachePath(gamename);

	if (!lslcache.Get(cachefile, ret) && (GameExists(gamename))) { // cache file failed, try from lsl
		PrefetchGame(gamename);
		lslcache.Get(cachefile, ret);
	}
	return ret;
}


UnitsyncImage Unitsync::GetSidePicture(const std::string& gamename, const std::string& SideName)
{
	assert(!gamename.empty());
	const std::string cachefile = GetSideImageCachePath(gamename, SideName);
	UnitsyncImage img;
	TRY_LOCK(img);

	if (!lslcache.Get(cachefile, img) && (GameExists(gamename))) { // cache file failed, try from lsl
		PrefetchGame(gamename);
		lslcache.Get(cachefile, img);
	}

	return img;
}

UnitsyncImage Unitsync::GetImage(const std::string& image_path, bool useWhiteAsTransparent) const
{
	const int ini = susynclib().OpenFileVFS(image_path);
	if (!ini) {
		LSL_THROWF(unitsync, "cannot find image %s\n", image_path.c_str());
	}
	const int FileSize = susynclib().FileSizeVFS(ini);
	if (FileSize == 0) {
		susynclib().CloseFileVFS(ini);
		LSL_THROWF(unitsync, "image has size 0 %s\n", image_path.c_str());
	}
	Util::uninitialized_array<char> FileContent(FileSize);
	susynclib().ReadFileVFS(ini, FileContent, FileSize);
	susynclib().CloseFileVFS(ini);
	return UnitsyncImage::FromVfsFileData(FileContent, FileSize, image_path, useWhiteAsTransparent);
}

StringVector Unitsync::GetAIList(const std::string& gamename) const
{
	StringVector ret;
	TRY_LOCK(ret);
	if (gamename.empty())
		return ret;
	int total = susynclib().GetSkirmishAICount(gamename);
	for (int i = 0; i < total; i++) {
		StringVector infos = susynclib().GetAIInfo(i);
		const int namepos = Util::IndexInSequence(infos, "shortName");
		const int versionpos = Util::IndexInSequence(infos, "version");
		std::string ainame;
		if (namepos != lslNotFound)
			ainame += infos[namepos + 1];
		if (versionpos != lslNotFound)
			ainame += " " + infos[versionpos + 1];
		ret.push_back(ainame);
	}
	return ret;
}

void Unitsync::UnSetCurrentArchive()
{
#if ASYNC_LOAD
	LOCK_UNITSYNC;
#endif
	try {
		susynclib().UnSetCurrentMod();
	} catch (std::runtime_error) {
	}
}

StringVector Unitsync::GetAIInfos(int index) const
{
	StringVector ret;
	TRY_LOCK(ret);
	try {
		ret = susynclib().GetAIInfo(index);
	} catch (std::runtime_error) {
	}
	return ret;
}

GameOptions Unitsync::GetAIOptions(const std::string& gamename, int index)
{
	assert(!gamename.empty());
	GameOptions ret;
	TRY_LOCK(ret);
	int count = susynclib().GetAIOptionCount(gamename, index);
	for (int i = 0; i < count; ++i) {
		GetOptionEntry(i, ret);
	}
	return ret;
}

StringVector Unitsync::GetUnitsList(const std::string& gamename)
{
	assert(!gamename.empty());
	StringVector cache;
	if (!GameExists(gamename))
		return cache;
	const std::string cachefile = GetUnitsCacheFilePath(gamename);
	TRY_LOCK(cache)

	if (!lslcache.Get(cachefile, cache)) { //cache read failed
		PrefetchGame(gamename);
		lslcache.Get(cachefile, cache);
	}
	return cache;
}

static std::string GetImageName(ImageType imgtype)
{
	switch (imgtype) {
		case IMAGE_MAP:
			return ".minimap.png";
		case IMAGE_MAP_THUMB:
			return ".minimap_thumb.png";
		case IMAGE_METALMAP:
			return ".metalmap.png";
		case IMAGE_HEIGHTMAP:
			return ".heightmap.png";
		default:
			assert(false);
			return "";
	}
}

UnitsyncImage Unitsync::GetImageFromUS(const std::string& mapname, const MapInfo& info, ImageType imgtype)
{
	if ((info.width <= 0) || (info.height <= 0)) {
		LslWarning("Couldn't load mapimage from %s, missing dependencies?", mapname.c_str());
		return UnitsyncImage(1, 1);
	}
	try {
		UnitsyncImage img;
		//convert and save
		switch (imgtype) {
			case IMAGE_MAP:
			case IMAGE_MAP_THUMB:
				img = susynclib().GetMinimap(mapname);
				break;
			case IMAGE_METALMAP:
				img = susynclib().GetMetalmap(mapname);
				break;
			case IMAGE_HEIGHTMAP:
				img = susynclib().GetHeightmap(mapname);
				break;
		}
		lslSize image_size = lslSize(info.width, info.height).MakeFit(lslSize(img.GetWidth(), img.GetHeight()));
		img.Rescale(image_size.GetWidth(), image_size.GetHeight()); //rescale to keep aspect ratio
		img.Save(GetMapImagePath(mapname, imgtype));
		return img;
	} catch (...) { //we failed horrible, use dummy image
		LslWarning("Couldn't rescale map image from %s, missing dependencies?", mapname.c_str());
		return UnitsyncImage(1, 1);
	}
}

UnitsyncImage Unitsync::GetScaledMapImage(const std::string& mapname, ImageType imgtype, int width, int height)
{
	assert(imgtype != IMAGE_MAP_THUMB || width == 98); //FIXME: allow to set by config
	assert(imgtype != IMAGE_MAP_THUMB || height == 98);

	UnitsyncImage img;

	const std::string cachefile = GetMapImagePath(mapname, imgtype);
	if (!lslcache.Get(cachefile, img)) {
		PrefetchMap(mapname);
		lslcache.Get(cachefile, img);
	}

	const bool rescale = (width > 0) && (height > 0);
	if (rescale && img.isValid()) {
		lslSize image_size = lslSize(img.GetWidth(), img.GetHeight()).MakeFit(lslSize(width, height));
		if ((image_size.GetWidth() != img.GetWidth() || image_size.GetHeight() != img.GetHeight())) {
			img.Rescale(image_size.GetWidth(), image_size.GetHeight());
		}
	}
	return img;
}

MapInfo Unitsync::_GetMapInfoEx(const std::string& mapname)
{
	MapInfo info;
	info.width = 1;
	info.height = 1;
	const std::string cachefile = GetMapInfoPath(mapname);
	StringVector cache;
	if (!lslcache.Get(cachefile, info)) { //cache file failed
		PrefetchMap(mapname);
		lslcache.Get(cachefile, info);
	}

	return info;
}

bool Unitsync::ReloadUnitSyncLib()
{
#if ASYNC_LOAD
	LOCK_UNITSYNC;
#endif
	const std::string path = LSL::Util::config().GetCurrentUsedUnitSync();
	if (path.empty())
		return false;
#if ASYNC_LOAD
	LoadUnitSyncLibAsync(path);
	return true; //FIXME: needs event when successfully loaded
#else
	return LoadUnitSyncLib(path);
#endif
}


void Unitsync::SetSpringDataPath(const std::string& path)
{
	if (!IsLoaded()) {
		return;
	}
#if ASYNC_LOAD
	LOCK_UNITSYNC;
#endif
	if (path.empty()) {
		susynclib().DeleteSpringConfigKey("SpringData");
	} else {
		susynclib().SetSpringConfigString("SpringData", path);
	}
}

bool Unitsync::GetSpringDataPath(std::string& path)
{
	if (IsLoaded() && (!m_datapaths.empty())) {
		path = m_datapaths[0];
	}
	return !path.empty();
}

std::string Unitsync::GetFileCachePath(const std::string& name, bool IsMod, bool usehash) const
{
	assert(!name.empty());
	std::string ret = m_cache_path + name;
	if (!usehash)
		return ret;

	ret += "-";
	try {
		ret += IsMod ? m_mods_list.at(name) : m_maps_list.at(name);
	} catch (std::out_of_range) {
		LslWarning("Hash of %s doesn't exist!", name.c_str());
		assert(false);
	}
	return ret;
}


void Unitsync::GetSpringDataPaths()
{
	m_datapaths.clear();
	const int dirs = susynclib().GetSpringDataDirCount();
	if (dirs < 0) {
		LslWarning("Couldn't get GetSpringDataDirCount(): %d", dirs);
		return;
	}
	m_datapaths.resize(dirs + 1);
	for (int i = 1; i <= dirs; i++) {
		const std::string datadir = susynclib().GetSpringDataDirByIndex(i - 1);
		if (datadir.empty()) {
			continue;
		}
		m_datapaths[i] = datadir;
	}
	m_datapaths[0] = susynclib().GetSpringDataDir();
}

bool Unitsync::GetPlaybackList(std::set<std::string>& ret, bool ReplayType) const
{
	if (!IsLoaded())
		return false;

	struct
	{
		bool operator()(bool isReplayType, const std::string& filename)
		{
			if (isReplayType) {
				return (filename.substr(filename.length() - 4) == ".sdf" ||
					filename.substr(filename.length() - 5) == ".sdfz");
			} else {
				return (filename.substr(filename.length() - 4) == ".ssf");
			}
		}
	} isWantedFile;

	std::string subpath;
	if (ReplayType) {
		subpath = "demos";
	} else {
		subpath = "Saves";
	}
	for (const std::string& datadir : m_datapaths) {
		const std::string& dirname = Util::EnsureDelimiter(datadir) + subpath;
		DIR* dir;
		struct dirent* ent;
		if ((dir = opendir(dirname.c_str())) == NULL) {
			continue;
		}
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			if (ent->d_name[0] == '.') //skip hidden files / . / ..
				continue;
			const std::string& filename = Util::EnsureDelimiter(dirname) + std::string(ent->d_name);
			struct stat sb;
			stat(filename.c_str(), &sb);
			if ((sb.st_mode & S_IFDIR) != 0) { // is dir, skip
				continue;
			}

			if (!isWantedFile(ReplayType, filename)) // compare file ending
				continue;
			ret.insert(filename);
		}
		closedir(dir);
	}
	return true;
}

bool Unitsync::FileExists(const std::string& name) const
{
	assert(!name.empty());
	assert(name[0] != '/'); // don't check for real files here, as this is a VERY slow call
	int handle = susynclib().OpenFileVFS(name);
	if (handle == 0)
		return false;
	susynclib().CloseFileVFS(handle);
	return true;
}

std::string Unitsync::GetArchivePath(const std::string& name) const
{
#if ASYNC_LOAD
	LOCK_UNITSYNC;
#endif
	return susynclib().GetArchivePath(name);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Unitsync prefetch/background thread code

namespace
{
class CacheMapImageWorkItem : public WorkItem
{
public:
	Unitsync* m_usync;
	std::string m_mapname;

	void Run()
	{
		m_usync->GetScaledMapImage(m_mapname, m_imgtype, m_width, m_height);
		m_usync->PostEvent(m_mapname);
	}
	CacheMapImageWorkItem(Unitsync* usync, const std::string& mapname, LSL::ImageType imgtype, int w = -1, int h = -1)
	    : m_usync(usync)
	    , m_mapname(mapname)
	    , m_width(w)
	    , m_height(h)
	    , m_imgtype(imgtype)
	{
		assert(usync != nullptr);
	}

private:
	int m_width;
	int m_height;
	LSL::ImageType m_imgtype;
	CacheMapImageWorkItem()
	{
	}
};

class GetMapExAsyncWorkItem : public WorkItem
{
public:
	void Run()
	{
		m_usync->GetMap(m_mapname);
		m_usync->PostEvent(m_mapname);
	}

	GetMapExAsyncWorkItem(Unitsync* usync, const std::string& mapname)
	    : m_usync(usync)
	    , m_mapname(mapname)
	{
	}

private:
	Unitsync* m_usync;
	std::string m_mapname;
};

class LoadUnitSyncLibAsyncWorkItem : public WorkItem
{
public:
	void Run()
	{
		try {
			m_usync->LoadUnitSyncLib(m_unitsyncloc);
		} catch (...) {
			// Event without mapname means some async job failed.
			// This is sufficient for now, we just need symmetry between
			// number of initiated async jobs and number of finished/failed
			// async jobs.
		}
		m_usync->PostEvent("");
	}

private:
	Unitsync* m_usync;
	std::string m_unitsyncloc;

public:
	LoadUnitSyncLibAsyncWorkItem(Unitsync* usync, const std::string& unitsyncLoc)
	    : m_usync(usync)
	    , m_unitsyncloc(unitsyncLoc.c_str())
	{
	}
};
}


void Unitsync::PrefetchMap(const std::string& mapname)
{
	// Use a simple hash based on 3 characters from the mapname
	// (without '.smf') as negative priority for the WorkItems.
	// This ensures WorkItems for the same map are put together,
	// which improves caching performance.

	// Measured improvement: 60% more cache hits while populating replay tab.
	// 50% hits without, 80% hits with this code.  (cache size 20 images)
	assert(!mapname.empty());

	//	const int length = std::max(0, int(mapname.length()) - 4);
	//	const int hash = (mapname[length * 1 / 4] << 16) | (mapname[length * 2 / 4] << 8) | mapname[length * 3 / 4];
	//	const int priority = -hash;

	if (!m_cache_thread) {
		LslDebug("cache thread not initialized %s", "PrefetchMap");
		return;
	}
	if (supportsManualUnLoad) {
		susynclib().AddAllArchives(mapname);
	}


	FetchUnitsyncErrors(mapname);
	GetMapHash(mapname);

	const int index = Util::IndexInSequence(m_unsorted_map_array, mapname);
	ASSERT_EXCEPTION(index >= 0, "Map not found");
	MapInfo info = susynclib().GetMapInfoEx(index);
	lslcache.Set(GetMapInfoPath(mapname), info);

	{
		GameOptions opt;
		const int count = susynclib().GetMapOptionCount(mapname);
		for (int i = 0; i < count; ++i) {
			GetOptionEntry(i, opt);
		}
		lslcache.Set(GetMapOptionsPath(mapname), opt);
	}
	{
		UnitsyncImage img;
		img = GetImageFromUS(mapname, info, IMAGE_MAP);
		lslcache.Set(GetMapImagePath(mapname, IMAGE_MAP), img);

		img.RescaleIfBigger(98, 98);
		lslcache.Set(GetMapImagePath(mapname, IMAGE_MAP_THUMB), img);

		img = GetImageFromUS(mapname, info, IMAGE_METALMAP);
		lslcache.Set(GetMapImagePath(mapname, IMAGE_METALMAP), img);

		img = GetImageFromUS(mapname, info, IMAGE_HEIGHTMAP);
		lslcache.Set(GetMapImagePath(mapname, IMAGE_HEIGHTMAP), img);
	}

	if (supportsManualUnLoad) {
		susynclib().RemoveAllArchives();
	}
}

void Unitsync::PrefetchGame(const std::string& gamename)
{
	assert(!gamename.empty());
	susynclib().SetCurrentMod(gamename);
	GetGameHash(gamename);
	{
		int count = susynclib().GetModOptionCount(gamename);
		GameOptions opt;
		for (int i = 0; i < count; ++i) {
			GetOptionEntry(i, opt);
		}
		FetchUnitsyncErrors(gamename);
		lslcache.Set(GetGameOptionsPath(gamename), opt);
	}
	{
		StringVector sides;
		try {
			sides = susynclib().GetSides(gamename);
			lslcache.Set(GetSidesCachePath(gamename), sides); //store into cachefile
		} catch (Exceptions::unitsync& u) {
			LslWarning("Error in GetSides: %s %s", gamename.c_str(), u.what());
		}
		for (const std::string& side : sides) {
			UnitsyncImage img;
			const std::string ImgName = "sidepics/" + boost::to_lower_copy(side);
			try {
				img = GetImage(ImgName + ".png", false);
			} catch (Exceptions::unitsync& e) {
			}
			if (!img.isValid() || img.GetWidth() < 2 || img.GetHeight() < 2) { //fallback to .bmp when file doesn't exist / is to small
				try {
					img = GetImage(ImgName + ".bmp", true);
				} catch (Exceptions::unitsync& e) {
				}
			}
			img.Save(GetSideImageCachePath(gamename, side));
		}
		FetchUnitsyncErrors(gamename);
	}
	{
		while (susynclib().ProcessUnits() > 0) {
		}
		StringVector units;
		const int unitcount = std::max(0, susynclib().GetUnitCount());
		units.resize(unitcount);
		for (int i = 0; i < unitcount; i++) {
			units[i] = susynclib().GetFullUnitName(i) + " (" + susynclib().GetUnitName(i) + ")";
		}
		FetchUnitsyncErrors(gamename);
		lslcache.Set(GetUnitsCacheFilePath(gamename), units);
	}
	susynclib().UnSetCurrentMod();
}

boost::signals2::connection Unitsync::RegisterEvtHandler(const StringSignalSlotType& handler)
{
	return m_async_ops_complete_sig.connect(handler);
}

void Unitsync::UnregisterEvtHandler(boost::signals2::connection& conn)
{
	conn.disconnect();
}

void Unitsync::PostEvent(const std::string& evt)
{
	m_async_ops_complete_sig(evt);
}

void Unitsync::GetMapImageAsync(const std::string& mapname, ImageType imgtype, int width, int height)
{
	if (mapname.empty())
		return;
	if (!m_cache_thread) {
		LslError("cache thread not initialised");
		return;
	}
	CacheMapImageWorkItem* work = new CacheMapImageWorkItem(this, mapname, imgtype, width, height);
	m_cache_thread->DoWork(work, 100);
}

void Unitsync::GetMapExAsync(const std::string& mapname)
{
	assert(!mapname.empty());
	if (mapname.empty())
		return;

	if (!m_cache_thread) {
		LslDebug("cache thread not initialized %s", "GetMapExAsync");
		return;
	}
	GetMapExAsyncWorkItem* work;
	work = new GetMapExAsyncWorkItem(this, mapname);
	m_cache_thread->DoWork(work, 200 /* higher prio then GetMinimapAsync */);
}

std::string Unitsync::GetTextfileAsString(const std::string& gamename, const std::string& file_path)
{
	assert(!gamename.empty());
	susynclib().SetCurrentMod(gamename);

	int ini = susynclib().OpenFileVFS(file_path);
	if (!ini)
		return std::string();
	int FileSize = susynclib().FileSizeVFS(ini);
	if (FileSize == 0) {
		susynclib().CloseFileVFS(ini);
		return std::string();
	}
	Util::uninitialized_array<char> FileContent(FileSize);
	susynclib().ReadFileVFS(ini, FileContent, FileSize);
	return std::string(FileContent, size_t(FileSize));
}

Unitsync& usync()
{
	static LSL::Util::LineInfo<Unitsync> m(AT);
	static LSL::Util::GlobalObjectHolder<Unitsync, LSL::Util::LineInfo<Unitsync> > m_sync(m);
	return m_sync;
}

void Unitsync::LoadUnitSyncLibAsync(const std::string& filename)
{
	LoadUnitSyncLibAsyncWorkItem* work = new LoadUnitSyncLibAsyncWorkItem(this, filename);
	m_cache_thread->DoWork(work, 500);
}

int Unitsync::GetSpringConfigInt(const std::string& name, int defvalue)
{
	if (IsLoaded())
		return susynclib().GetSpringConfigInt(name, defvalue);
	return defvalue;
}

std::string Unitsync::GetSpringConfigString(const std::string& name, const std::string& defvalue)
{
	if (IsLoaded())
		return susynclib().GetSpringConfigString(name, defvalue);
	return defvalue;
}

float Unitsync::GetSpringConfigFloat(const std::string& name, float defvalue)
{
	if (IsLoaded())
		return susynclib().GetSpringConfigInt(name, defvalue);
	return defvalue;
}

void Unitsync::SetSpringConfigInt(const std::string& name, int value)
{
	return susynclib().SetSpringConfigInt(name, value);
}

void Unitsync::SetSpringConfigString(const std::string& name, const std::string& value)
{
	return susynclib().SetSpringConfigString(name, value);
}

void Unitsync::SetSpringConfigFloat(const std::string& name, float value)
{
	return susynclib().SetSpringConfigFloat(name, value);
}

std::string Unitsync::GetConfigFilePath()
{
	return susynclib().GetConfigFilePath();
}

std::string Unitsync::GetMapImagePath(const std::string& mapname, ImageType imgtype) const
{
	return GetFileCachePath(mapname, false, false) + GetImageName(imgtype);
}

std::string Unitsync::GetMapOptionsPath(const std::string& mapname) const
{
	return GetFileCachePath(mapname, false, true) + ".mapoptions.json";
}

std::string Unitsync::GetMapInfoPath(const std::string& mapname) const
{
	return GetFileCachePath(mapname, false, false) + ".mapinfo.json";
}

std::string Unitsync::GetGameOptionsPath(const std::string& name) const
{
	return GetFileCachePath(name, true, true) + ".gameoptions.json";
}

std::string Unitsync::GetSidesCachePath(const std::string& gamename) const
{
	return GetFileCachePath(gamename, true) + ".sides.json";
}

std::string Unitsync::GetSideImageCachePath(const std::string& gamename, const std::string sidename) const
{
	return GetFileCachePath(gamename, true, false) + "-side-" + sidename + ".png";
}

std::string Unitsync::GetUnitsCacheFilePath(const std::string& gamename) const
{
	return GetFileCachePath(gamename, true) + ".units.json";
}


} // namespace LSL
