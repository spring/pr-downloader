/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LIBSPRINGLOBBY_HEADERGUARD_SPRINGUNITSYNC_H
#define LIBSPRINGLOBBY_HEADERGUARD_SPRINGUNITSYNC_H

#include "mmoptionmodel.h"
#include "data.h"
#include <lslutils/type_forwards.h>
#include "image.h"

#include <boost/thread/mutex.hpp>
#include <boost/signals2/signal.hpp>
#include <map>
#include <set>

#ifdef HAVE_WX
#include <wx/event.h>
#endif
namespace LSL
{

class UnitsyncImage;
struct GameOptions;
struct CachedMapInfo;
struct SpringMapInfo;
class UnitsyncLib;
class WorkerThread;

#ifdef HAVE_WX
extern const wxEventType UnitSyncAsyncOperationCompletedEvt;
#endif

enum ImageType {
	IMAGE_MAP = 0,
	IMAGE_MAP_THUMB,
	IMAGE_METALMAP,
	IMAGE_HEIGHTMAP,
};

class Unitsync
{
private:
	typedef boost::signals2::signal<void(std::string)>
	    StringSignalType;

public:
	typedef StringSignalType::slot_type
	    StringSignalSlotType;

	Unitsync();
	Unitsync(const Unitsync&) = delete;
	Unitsync& operator=(const Unitsync&) = delete;
	~Unitsync();

	StringVector GetGameList() const;
	bool GameExists(const std::string& gamename, const std::string& hash = "");
	UnitsyncGame GetGame(const std::string& gamename);
	UnitsyncGame GetGame(int index);

	GameOptions GetGameOptions(const std::string& name);
	StringVector GetGameDeps(const std::string& name) const;

	StringVector GetMapList() const;
	StringVector GetGameValidMapList(const std::string& gamename) const;
	bool MapExists(const std::string& mapname, const std::string& hash = "");

	UnitsyncMap GetMap(const std::string& mapname);
	UnitsyncMap GetMap(int index);
	GameOptions GetMapOptions(const std::string& name);

	StringVector GetSides(const std::string& gamename);
	UnitsyncImage GetSidePicture(const std::string& gamename, const std::string& SideName);

	bool LoadUnitSyncLib(const std::string& unitsyncloc);
	void FreeUnitSyncLib();

	bool IsLoaded() const;

	std::string GetSpringVersion() const;
	void UnSetCurrentArchive();

	StringVector GetAIList(const std::string& gamename) const;
	StringVector GetAIInfos(int index) const;
	GameOptions GetAIOptions(const std::string& gamename, int index);


	StringVector GetUnitsList(const std::string& gamename);

	bool ReloadUnitSyncLib();

	void SetSpringDataPath(const std::string& path);
	bool GetSpringDataPath(std::string& path);

	bool GetPlaybackList(std::set<std::string>& files, bool ReplayType = true) const; //savegames otehrwise

	std::string GetArchivePath(const std::string& name) const;

	/// schedule a map for prefetching
	void PrefetchMap(const std::string& mapname);
	void PrefetchGame(const std::string& gamename);

	boost::signals2::connection RegisterEvtHandler(const StringSignalSlotType& handler);
	void UnregisterEvtHandler(boost::signals2::connection& conn);
	void PostEvent(const std::string& evt); // helper for WorkItems

	void LoadUnitSyncLibAsync(const std::string& filename);

	int GetSpringConfigInt(const std::string& name, int defvalue);
	float GetSpringConfigFloat(const std::string& name, float defvalue);
	std::string GetSpringConfigString(const std::string& name, const std::string& defvalue);

	void SetSpringConfigInt(const std::string& name, int value);
	void SetSpringConfigString(const std::string& name, const std::string& value);
	void SetSpringConfigFloat(const std::string& name, float value);
	std::string GetConfigFilePath();

	void GetMapExAsync(const std::string& mapname);
	void GetMapImageAsync(const std::string& mapname, ImageType imgtype, int width, int height);

	//! get a map image, if width/height is set, scale it to the given dimensions
	UnitsyncImage GetScaledMapImage(const std::string& mapname, ImageType imgtype, int width = -1, int height = -1);

	/// fetch all errors from unitsync and push to our error handling
	void FetchUnitsyncErrors(const std::string& prefix);

	//! returns the absolute path of the requested item, creates the file when not exists
	std::string GetMapImagePath(const std::string& mapname, ImageType imgtype) const;
	std::string GetMapOptionsPath(const std::string& mapname) const;
	std::string GetMapInfoPath(const std::string& mapname) const;
	std::string GetGameOptionsPath(const std::string& name) const;
	std::string GetSidesCachePath(const std::string& gamename) const;
	std::string GetSideImageCachePath(const std::string& gamename, const std::string sidename) const;
	std::string GetUnitsCacheFilePath(const std::string& gamename) const;

private:
	std::string GetMapHash(const std::string& name);
	std::string GetGameHash(const std::string& name);
	bool GetImageFromCache(const std::string& cachefile, UnitsyncImage& img, ImageType imgtype);
	UnitsyncImage GetImageFromUS(const std::string& mapname, const MapInfo& info, ImageType imgtype);

	void ClearCache();
	void GetSpringDataPaths();

	/// get minimap with native width x height
	UnitsyncImage GetMinimap(const std::string& mapname);
	/// get metalmap with native width x height
	UnitsyncImage GetMetalmap(const std::string& mapname);
	/// get heightmap with native width x height
	UnitsyncImage GetHeightmap(const std::string& mapname);

	bool FileExists(const std::string& name) const;
	std::string GetTextfileAsString(const std::string& gamename, const std::string& file_path);

	StringVector GetMapDeps(const std::string& name);

	UnitsyncImage GetImage(const std::string& image_path, bool useWhiteAsTransparent = true) const;

	LocalArchivesVector m_maps_list;	 /// mapname -> hash
	LocalArchivesVector m_mods_list;	 /// gamename -> hash
	LocalArchivesVector m_mods_archive_name; /// gamename -> archive name
	LocalArchivesVector m_maps_archive_name; /// mapname -> archive name
	StringVector m_map_array;		 // this vector is CUSTOM SORTED ALPHABETICALLY, DON'T USE TO ACCESS UNITSYNC DIRECTLY
	StringVector m_mod_array;		 // this vector is CUSTOM SORTED ALPHABETICALLY, DON'T USE TO ACCESS UNITSYNC DIRECTLY
	StringVector m_unsorted_map_array;       // this is because unitsync doesn't have a search map index by name ..
	StringVector m_unsorted_mod_array;       // this isn't necessary but makes things more symmetrical :P
	StringVector m_datapaths;

	/// caches sett().GetCachePath(), because that method calls back into
	/// susynclib(), there's a good chance main thread blocks on some
	/// WorkerThread operation... cache is invalidated on reload.
	std::string m_cache_path;
	std::map<std::string, GameOptions> m_map_gameoptions;
	std::map<std::string, GameOptions> m_game_gameoptions;

	mutable boost::mutex m_lock;
	WorkerThread* m_cache_thread;
	StringSignalType m_async_ops_complete_sig;

	//! this function returns only the cache path without the file extension,
	//! the extension itself would be added in the function as needed
	std::string GetFileCachePath(const std::string& archivename, bool IsGame, bool usehash = true) const;

	bool _LoadUnitSyncLib(const std::string& unitsyncloc);
	void _FreeUnitSyncLib();

	MapInfo _GetMapInfoEx(const std::string& mapname);

	void PopulateArchiveList();

	friend Unitsync& usync();

	bool supportsManualUnLoad;
};

Unitsync& usync();

struct GameOptions
{
	OptionMapBool bool_map;
	OptionMapFloat float_map;
	OptionMapString string_map;
	OptionMapList list_map;
	OptionMapSection section_map;
};

/// Helper class for managing async operations safely
class UnitSyncAsyncOps : public boost::noncopyable
{
public:
	UnitSyncAsyncOps(const Unitsync::StringSignalSlotType& evtHandler)
	    : m_evtHandler_connection()
	{
		m_evtHandler_connection = usync().RegisterEvtHandler(evtHandler);
	}

	~UnitSyncAsyncOps()
	{
		Disconnect();
	}

	void Disconnect()
	{
		usync().UnregisterEvtHandler(m_evtHandler_connection);
	}
	bool Connected()
	{
		return m_evtHandler_connection.connected();
	}

	void GetMapImageAsync(const std::string& mapname, ImageType imgtype, int width, int height)
	{
		usync().GetMapImageAsync(mapname, imgtype, width, height);
	}

private:
	boost::signals2::connection m_evtHandler_connection;
};

} // namespace LSL

#endif // LIBSPRINGLOBBY_HEADERGUARD_SPRINGUNITSYNC_H
