/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_CONFIG_H
#define LSL_CONFIG_H

#define STR_DUMMY(name)               \
	std::string name() const      \
	{                             \
		return std::string(); \
	}
#define INT_DUMMY(name)   \
	int name() const  \
	{                 \
		return 0; \
	}

#include <string>
#include <lslutils/type_forwards.h>

namespace LSL
{
namespace Util
{

template <class PB, class I>
class GlobalObjectHolder;

struct SettStartBox {
	int ally;
	int topx;
	int topy;
	int bottomx;
	int bottomy;
};

class Config
{
	Config();

private:
	std::string Cache;
	std::string CurrentUsedUnitSync;
	std::string CurrentUsedSpringBinary;
	std::string DataDir;

public:
	std::string GetCachePath() const;
	std::string GetCurrentUsedUnitSync() const;
	std::string GetCurrentUsedSpringBinary() const;
	std::string GetDataDir() const;
	void ConfigurePaths(const std::string& Cache, const std::string& CurrentUsedUnitSync, const std::string& CurrentUsedSpringBinary, const std::string& DataDir);
	STR_DUMMY(GetMyInternalUdpSourcePort)
	INT_DUMMY(GetClientPort)

	StringVector GetPresetList()
	{
		StringVector tmp;
		return tmp;
	}
	StringMap GetHostingPreset(const std::string&, size_t)
	{
		StringMap tmp;
		return tmp;
	}
	void SetHostingPreset(const std::string&, size_t, const StringMap&)
	{
	}
	lslColor GetBattleLastColor() const;
	int GetBattleLastSideSel(const std::string& /*gamename*/) const
	{
		return 0;
	}
	void SaveSettings()
	{
	}
	void DeletePreset(const std::string& /*gamename*/)
	{
	}

	void SetMapLastStartPosType(const std::string&, const std::string&){};
	std::string GetMapLastStartPosType(const std::string&) const
	{
		return "";
	}

	template <class T>
	void SetMapLastRectPreset(const std::string&, const T&)
	{
	}
	template <class T>
	T GetMapLastRectPreset(const std::string&)
	{
		return T();
	}

	bool GetBattleLastAutoAnnounceDescription() const
	{
		return false;
	}
	int GetBattleLastAutoSpectTime() const
	{
		return 0;
	}

	template <class PB, class I>
	friend class GlobalObjectHolder;
};

Config& config();

} // namespace Util
} // namespace LSL

#endif // LSL_CONFIG_H
