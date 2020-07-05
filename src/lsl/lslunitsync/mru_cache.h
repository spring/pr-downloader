/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_HEADERGUARD_MRU_CACHE_H
#define LSL_HEADERGUARD_MRU_CACHE_H


#include <string>
#include <boost/thread/mutex.hpp>
#include "lslutils/logging.h"
#ifdef _WIN32 //undefine windows header pollution
#ifdef GetUserName
#undef GetUserName
#endif
#endif
#include <list>
#include <map>

namespace LSL
{

/// Thread safe MRU cache (works like a std::map but has maximum size)
template <typename TValue>
class MostRecentlyUsedCache
{
public:
	//! name parameter might be used to identify stats in dgb output
	MostRecentlyUsedCache(size_t max_size, const std::string& name = "")
	    : m_max_size(max_size)
	    , m_cache_hits(0)
	    , m_cache_misses(0)
	    , m_name(name)
	{
	}

	~MostRecentlyUsedCache()
	{
		LslDebug("%s - cache hits: %d misses: %d", m_name.c_str(), m_cache_hits, m_cache_misses);
	}

	void Add(const std::string& name, const TValue& img)
	{
		boost::mutex::scoped_lock lock(m_lock);
		if (m_itemnames.size() > m_max_size) {
			m_items.erase(m_itemnames.back());
			m_itemnames.pop_back();
		}
		m_itemnames.push_front(name);
		m_items[name] = img;
	}

	bool TryGet(const std::string& name, TValue& img)
	{
		boost::mutex::scoped_lock lock(m_lock);

		auto it = m_items.find(name);
		if (it == m_items.end()) {
			++m_cache_misses;
			return false;
		}
		// reinsert at front, so that most recently used items are always at front
		m_itemnames.push_front(m_itemnames.back());
		m_itemnames.pop_back();
		++m_cache_hits;
		img = it->second; //copy!
		return true;
	}

	void Clear()
	{
		boost::mutex::scoped_lock lock(m_lock);
		m_items.clear();
		m_itemnames.clear();
	}

private:
	mutable boost::mutex m_lock;
	std::list<std::string> m_itemnames;
	std::map<std::string, TValue> m_items;
	const size_t m_max_size;
	int m_cache_hits;
	int m_cache_misses;
	const std::string m_name;
};

class UnitsyncImage;
struct MapInfo;
typedef MostRecentlyUsedCache<UnitsyncImage> MostRecentlyUsedImageCache;
typedef MostRecentlyUsedCache<MapInfo> MostRecentlyUsedMapInfoCache;
typedef MostRecentlyUsedCache<std::vector<std::string>> MostRecentlyUsedArrayStringCache;

} // namespace LSL

#endif // MRU_CACHE_H
