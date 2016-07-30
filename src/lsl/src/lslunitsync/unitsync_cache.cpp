/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "unitsync_cache.h"
#include <lslunitsync/unitsync.h>
#include "optionswrapper.h"
#include "data.h"
#include "mmoptionmodel.h"
#include <json/json.h>
#include <lslutils/misc.h>
#include <lslutils/logging.h>
#include <sstream>
#include <boost/format.hpp>


namespace LSL
{

const static int CACHE_VERSION = 2;

static bool ParseJsonFile(const std::string& path, Json::Value& root)
{
	std::FILE* fp = Util::lslopen(path, "rb");
	if (fp == nullptr) {
		return false;
	}
	std::fseek(fp, 0L, SEEK_END);
	unsigned int fsize = std::ftell(fp);
	std::rewind(fp);

	std::string s(fsize, 0);
	if (fsize != std::fread(static_cast<void*>(&s[0]), 1, fsize, fp)) {
		return false;
	}
	std::fclose(fp);
	Json::Reader reader;
	Json::Value tmp;
	if (!reader.parse(s, tmp, false)) {
		return false;
	}
	if (tmp["CacheVersion"] != CACHE_VERSION) {
		return false;
	}
	root = tmp["data"];
	return true;
}

static bool writeJsonFile(const std::string& path, const Json::Value& root)
{
	Json::Value tmp;
	tmp["CacheVersion"] = CACHE_VERSION;
	tmp["data"] = root;

	FILE* f = Util::lslopen(path, "w");
	if (f == nullptr) {
		LslWarning("Couldn't open %s for writing!", path.c_str());
		return false;
	}
	std::stringstream ss;
	ss << tmp; //FIXME: make this efficient
	const std::string& str = ss.str();
	fwrite(str.c_str(), str.size(), 1, f);
	fclose(f);
	return true;
}

Cache::Cache()
    : m_map_image_cache(30, "m_map_image_cache")
    , m_mapinfo_cache(1000000, "m_mapinfo_cache") // takes at most 30k per image (   100x100 24 bpp minimap )
    , m_sides_cache(200, "m_sides_cache")
{

}

bool Cache::Get(const std::string& path, MapInfo& info)
{
	Json::Value root;
	try {
		if (!ParseJsonFile(path, root)) {
			return false;
		}
		info.author = root["author"].asString();
		info.tidalStrength = root["tidalStrength"].asFloat();
		info.gravity = root["gravity"].asInt();
		info.maxMetal = root["maxMetal"].asFloat();
		info.extractorRadius = root["extractorRadius"].asFloat();
		info.minWind = root["minWind"].asInt();
		info.maxWind = root["maxWind"].asInt();
		info.width = root["width"].asInt();
		info.height = root["height"].asInt();
		info.description = root["description"].asString();
		Json::Value items = root["startpositions"];
		for (Json::ArrayIndex i = 0; i < items.size(); i++) {
			StartPos position;
			position.x = items[i]["x"].asInt();
			position.y = items[i]["y"].asInt();
			info.positions.push_back(position);
		}
	} catch (std::exception& e) {
		LslWarning("Exception when parsing %s %s", path.c_str(), e.what());
		return false;
	}
	return true;
}

void Cache::Set(const std::string& path, const MapInfo& info)
{
	Json::Value root;
	root["author"] = info.author;
	root["tidalStrength"] = info.tidalStrength;
	root["gravity"] = info.gravity;
	root["maxMetal"] = info.maxMetal;
	root["extractorRadius"] = info.extractorRadius;
	root["minWind"] = info.minWind;
	root["maxWind"] = info.maxWind;
	root["width"] = info.width;
	root["height"] = info.height;
	root["description"] = info.description;
	for (StartPos pos : info.positions) {
		Json::Value item;
		item["x"] = pos.x;
		item["y"] = pos.y;
		root["startpositions"].append(item);
	}
	writeJsonFile(path, root);
}

void Cache::Set(const std::string& path, const GameOptions& opt)
{
	Json::Value root;

	for (auto const& ent : opt.bool_map) {
		Json::Value entry;
		entry["key"] = ent.second.key;
		entry["name"] = ent.second.name;
		entry["description"] = ent.second.description;
		entry["type"] = ent.second.type;
		entry["section"] = ent.second.section;

		entry["def"] = ent.second.def;
		root.append(entry);
	}
	for (auto const& ent : opt.float_map) {
		Json::Value entry;
		entry["key"] = ent.second.key;
		entry["name"] = ent.second.name;
		entry["description"] = ent.second.description;
		entry["type"] = ent.second.type;
		entry["section"] = ent.second.section;

		entry["def"] = ent.second.def;
		entry["min"] = ent.second.min;
		entry["max"] = ent.second.max;
		entry["stepping"] = ent.second.stepping;
		root.append(entry);
	}

	for (auto const& ent : opt.string_map) {
		Json::Value entry;
		entry["key"] = ent.second.key;
		entry["name"] = ent.second.name;
		entry["description"] = ent.second.description;
		entry["type"] = ent.second.type;
		entry["section"] = ent.second.section;

		entry["def"] = ent.second.def;
		entry["max_len"] = ent.second.max_len;

		root.append(entry);
	}
	for (auto const& ent : opt.list_map) {
		Json::Value entry;
		entry["key"] = ent.second.key;
		entry["name"] = ent.second.name;
		entry["description"] = ent.second.description;
		entry["type"] = ent.second.type;
		entry["section"] = ent.second.section;

		entry["cur_choice_index"] = ent.second.cur_choice_index;
		entry["def"] = ent.second.def;

		for (const listItem& item : ent.second.listitems) {
			Json::Value dict;
			dict["key"] = item.key;
			dict["name"] = item.name;
			dict["desc"] = item.desc;
			entry["items"].append(dict);
		}
		root.append(entry);
	}
	for (auto const& ent : opt.section_map) {
		Json::Value entry;
		entry["key"] = ent.second.key;
		entry["name"] = ent.second.name;
		entry["description"] = ent.second.description;
		entry["type"] = ent.second.type;
		entry["section"] = ent.second.section;

		root.append(entry);
	}
	writeJsonFile(path, root);
}

bool Cache::Get(const std::string& path, GameOptions& opt)
{
	Json::Value root;
	if (!ParseJsonFile(path, root)) {
		return false;
	}

	try {
		for (Json::ArrayIndex i = 0; i < root.size(); i++) {
			const std::string key = root[i]["key"].asString();
			const std::string name = root[i]["name"].asString();
			const std::string section_str = root[i]["section"].asString();
			const std::string optiondesc = root[i]["description"].asString();
			const int opttype = root[i]["type"].asInt();
			switch (opttype) {
				case Enum::opt_float: {
					opt.float_map[key] = mmOptionFloat(name, key, optiondesc, root[i]["def"].asFloat(),
									   root[i]["stepping"].asFloat(),
									   root[i]["min"].asFloat(), root[i]["max"].asFloat(),
									   section_str);
					break;
				}
				case Enum::opt_bool: {
					opt.bool_map[key] = mmOptionBool(name, key, optiondesc, root[i]["def"].asBool(), section_str);
					break;
				}
				case Enum::opt_string: {
					opt.string_map[key] = mmOptionString(name, key, optiondesc, root[i]["def"].asString(), root[i]["max_len"].asInt(), section_str);
					break;
				}
				case Enum::opt_list: {
					opt.list_map[key] = mmOptionList(name, key, optiondesc, root[i]["def"].asString(), section_str);
					const int listItemCount = root[i]["items"].size();
					for (int j = 0; j < listItemCount; ++j) {
						Json::Value& item = root[i]["items"][j];
						const std::string itemkey = item["key"].asString();
						const std::string name = item["name"].asString();
						const std::string desc = item["desc"].asString();
						opt.list_map[key].addItem(itemkey, name, desc);
					}
					break;
				}
				case Enum::opt_section: {
					opt.section_map[key] = mmOptionSection(name, key, optiondesc, section_str);
				}
			}
		}
	} catch (std::exception& e) {
		LslWarning("Exception when parsing %s %s", path.c_str(), e.what());
		return false;
	}
	return true;
}

bool Cache::Get(const std::string& path, StringVector& opt)
{
	if (m_sides_cache.TryGet(path, opt)) { //first return from mru cache
		return true;
	}
	Json::Value root;
	if (!ParseJsonFile(path, root)) {
		return false;
	}
	try {
		opt.clear();
		for (Json::ArrayIndex i = 0; i < root.size(); i++) {
			opt.push_back(root[i].asString());
		}
	} catch (std::exception& e) {
		LslWarning("Exception when parsing %s %s", path.c_str(), e.what());
		return false;
	}
	m_sides_cache.Add(path, opt); //store into mru
	return true;
}

void Cache::Set(const std::string& path, const StringVector& opt)
{
	Json::Value root;
	for (const std::string& item : opt) {
		root.append(item);
	}
	writeJsonFile(path, root);
}

void Cache::clear()
{
	m_map_image_cache.Clear();
	m_mapinfo_cache.Clear();
	m_sides_cache.Clear();

}

bool Cache::Get(const std::string& path, UnitsyncImage& img)
{
	if (m_map_image_cache.TryGet(path, img) && img.isValid()) {
		LslDebug("Loaded from m_map_image_cache: %s", path.c_str());
		return true;
	}
	if (Util::FileExists(path)) {
		LslDebug("Loading from %s", path.c_str());
		img = UnitsyncImage(path);
		if (img.isValid()) {
			m_map_image_cache.Add(path, img);
			return true;
		}
	}
	return false;
}

void Cache::Set(const std::string& path, const UnitsyncImage& img)
{
	if (img.isValid()) {
		img.Save(path);
		m_map_image_cache.Add(path, img);
	}
}

Cache::~Cache()
{
	clear();
}

static Cache* cach = nullptr;

Cache& Cache::GetInstance()
{
	if (cach == nullptr) {
		cach = new Cache();
	}
	return *cach;
}

void Cache::FreeInstance()
{
	assert(cach != nullptr); //no doublefree!
	delete cach;
	cach = nullptr;
}


} // namespace LSL
