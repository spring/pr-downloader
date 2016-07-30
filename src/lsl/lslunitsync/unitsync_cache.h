/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LIBSPRINGLOBBY_HEADERGUARD_SPRINGUNITSYNC_CACHE_H
#define LIBSPRINGLOBBY_HEADERGUARD_SPRINGUNITSYNC_CACHE_H

#include <string>
#include <lslutils/type_forwards.h>
#include "mru_cache.h"

namespace LSL
{

struct MapInfo;
struct GameOptions;

class Cache
{
public:
	Cache();
	~Cache();
	static Cache& GetInstance();
	static void FreeInstance();
	//! returns an array where each element is a line of the file
	bool Get(const std::string& path, MapInfo& ret);
	bool Get(const std::string& path, GameOptions& opt);
	bool Get(const std::string& path, StringVector& opt);
	bool Get(const std::string& path, UnitsyncImage& img);

	//! write a file where each element of the array is a line
	void Set(const std::string& path, const MapInfo& data);
	void Set(const std::string& path, const GameOptions& opt);
	void Set(const std::string& path, const StringVector& opt);
	void Set(const std::string& path, const UnitsyncImage& img);

	void clear();
	/// this cache facilitates async image fetching (image is stored in cache
	/// in background thread, then main thread gets it from cache)
	MostRecentlyUsedImageCache m_map_image_cache;
	/// this caches MapInfo to facilitate GetMapExAsync
	MostRecentlyUsedMapInfoCache m_mapinfo_cache;
	MostRecentlyUsedArrayStringCache m_sides_cache;
};
} // namespace LSL

#define lslcache Cache::GetInstance()

#endif // LIBSPRINGLOBBY_HEADERGUARD_SPRINGUNITSYNC_CACHE_H
