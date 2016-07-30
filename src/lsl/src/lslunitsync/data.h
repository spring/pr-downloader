/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_HEADERGUARD_SPRINGUNITSYNC_DATA_H
#define LSL_HEADERGUARD_SPRINGUNITSYNC_DATA_H

#include <vector>
#include <map>
#include <string>

namespace LSL
{

struct UnitsyncGame {
	UnitsyncGame()
	    : name(std::string())
	    , hash(std::string())
	{
	}
	UnitsyncGame(const std::string& name, const std::string& hash)
	    : name(name)
	    , hash(hash)
	{
	}
	std::string name;
	std::string hash;
};

struct StartPos {
	int x;
	int y;
};

struct MapInfo {
	std::string description;
	int tidalStrength;
	int gravity;
	float maxMetal;
	int extractorRadius;
	int minWind;
	int maxWind;

	int width;
	int height;
	std::vector<StartPos> positions;

	std::string author;
	MapInfo()
	    : description("")
	    , tidalStrength(0)
	    , gravity(0)
	    , maxMetal(0.0f)
	    , extractorRadius(0)
	    , minWind(0)
	    , maxWind(0)
	    , width(0)
	    , height(0)
	    , author("")
	{
	}
};

struct UnitsyncMap {
	UnitsyncMap()
	    : name(std::string())
	    , hash(std::string())
	{
	}
	UnitsyncMap(const std::string& name, const std::string& hash)
	    : name(name)
	    , hash(hash)
	{
	}
	std::string name;
	std::string hash;
	MapInfo info;
};

enum GameFeature {
	USYNC_Sett_Handler,
	USYNC_GetInfoMap,
	USYNC_GetDataDir,
	USYNC_GetSkirmishAI
};

enum MediaType {
	map,
	game
};


typedef std::map<std::string, std::string> LocalArchivesVector;

} // namespace LSL

#endif // SPRINGLOBBY_HEADERGUARD_SPRINGUNITSYNC_DATA_H
