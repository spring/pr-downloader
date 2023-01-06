/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "c_api.h"

#include <stdexcept>
#include <cmath>

#include "lslutils/logging.h"
#include "lslutils/debug.h"
#include "lslutils/misc.h"
#include "lslutils/globalsmanager.h"
#include "lslutils/conversion.h"

#include "image.h"
#include "loader.h"
#include "sharedlib.h"

#define UNITSYNC_EXCEPTION(cond, msg)             \
	do {                                      \
		if (!(cond))                      \
			LSL_THROW(unitsync, msg); \
	} while (0)

#define CHECK_FUNCTION(arg)                                 \
	do {                                                \
		if (!(arg))                                 \
			LSL_THROW(function_missing, "arg"); \
	} while (0)

// on linux / debug builds, check the time a unitsync
#if defined(__linux__) && !defined(NDEBUG)
#include <sys/time.h>

/**
	ScopedTime, takes time of a function call / one scope
	usage: just add at the beginning of a function
	ScopedTime tmp(__FUNCTION__, __LINE__);
*/
class ScopedTime
{
public:
	int diff_ms(timespec t1, timespec t2)
	{
		return (((t1.tv_sec - t2.tv_sec) * 1000000000) + (t1.tv_nsec - t2.tv_nsec)) / 1000000;
	}
	ScopedTime(const std::string& func, const int line)
	    : func(func)
	    , line(line)
	{
		clock_gettime(CLOCK_REALTIME, &start);
	}
	~ScopedTime()
	{
		timespec stop;
		clock_gettime(CLOCK_REALTIME, &stop);
		const int diff = diff_ms(stop, start);
		if (diff > 10) {
			LslWarning("Slow Unitsync call (%s():%d) took: %dms", func.c_str(), line, diff);
		}
	}

private:
	timespec start;
	std::string func;
	int line;
};

#define LOCK_UNITSYNC                                  \
	ScopedTime scopedtime(__FUNCTION__, __LINE__); \
	std::scoped_lock lock_criticalsection(m_lock);

#else
#define LOCK_UNITSYNC \
	std::scoped_lock lock_criticalsection(m_lock);
#endif // __linux__


//! Macro that checks if a function is present/loaded, unitsync is loaded, and locks it on call.
#define InitLib(arg)                                                        \
	LOCK_UNITSYNC;                                                      \
	UNITSYNC_EXCEPTION(m_loaded, "Unitsync function not loaded:" #arg); \
	CHECK_FUNCTION(arg);


namespace LSL
{

UnitsyncLib::UnitsyncLib()
    : m_loaded(false)
    , m_libhandle(NULL)
    , m_path(std::string())
    , m_init(NULL)
    , m_uninit(NULL)
{
}


UnitsyncLib::~UnitsyncLib()
{
	Unload();
}

bool UnitsyncLib::Load(const std::string& path)
{
	LOCK_UNITSYNC;
	_Load(path);
	_Init();
	return m_loaded;
}


void UnitsyncLib::_Load(const std::string& path)
{
	assert(!path.empty());
	if (IsLoaded() && path == m_path)
		return;

	_Unload();

	m_path = path;

	// Load the library.
	LslDebug("Loading from: %s", path.c_str());
	m_libhandle = _LoadLibrary(path);
	if (m_libhandle == nullptr)
		return;

	// Load all function from library.

	m_loaded = UnitsyncFunctionLoader::BindFunctions(this);

	if (!m_loaded) {
		m_uninit = NULL;
		_Unload();
	}
}

void UnitsyncLib::_Init()
{
	if (IsLoaded() && m_init != NULL) {
		m_current_mod.clear();
		const int res = m_init(true, 1);
		if (res == 0) {
			LslError("Unitsync init failed!");
		}
		const std::vector<std::string> errors = GetUnitsyncErrors();
		for (const std::string& error : errors) {
			LslError("%s", error.c_str());
		}
	}
}

void UnitsyncLib::_RemoveAllArchives()
{
	if (m_remove_all_archives)
		m_remove_all_archives();
	else
		_Init();
}

void UnitsyncLib::RemoveAllArchives()
{
	InitLib(m_add_all_archives);
	m_remove_all_archives();
}

void UnitsyncLib::Unload()
{
	if (!IsLoaded())
		return; // dont even lock anything if unloaded.
	LOCK_UNITSYNC;
	_Unload();
}

void UnitsyncLib::_Unload()
{
	// as soon as we enter m_uninit unitsync technically isn't loaded anymore.
	m_loaded = false;

	m_path.clear();

	// can't call UnSetCurrentMod() because it takes the unitsync lock
	m_current_mod.clear();

	if (m_uninit)
		m_uninit();
	UnitsyncFunctionLoader::UnbindFunctions(this);
	_FreeLibrary(m_libhandle);
	m_libhandle = NULL;
	m_init = NULL;
	m_uninit = NULL;
}

bool UnitsyncLib::IsLoaded() const
{
	return m_loaded;
}

std::vector<std::string> UnitsyncLib::GetUnitsyncErrors() const
{
	std::vector<std::string> ret;
	try {
		UNITSYNC_EXCEPTION(m_loaded, "Unitsync not loaded.");
		CHECK_FUNCTION(m_get_next_error);

		const char* msg = m_get_next_error();
		while (msg) {
			ret.push_back(msg);
			msg = m_get_next_error();
		}
		return ret;
	} catch (std::runtime_error& e) {
		ret.push_back(e.what());
		return ret;
	}
}

bool UnitsyncLib::VersionSupports(LSL::GameFeature feature)
{
	LOCK_UNITSYNC;
	switch (feature) {
		case LSL::USYNC_Sett_Handler:
			return m_set_spring_config_string;
		case LSL::USYNC_GetInfoMap:
			return m_get_infomap_size;
		case LSL::USYNC_GetDataDir:
			return m_get_writeable_data_dir;
		case LSL::USYNC_GetSkirmishAI:
			return m_get_skirmish_ai_count;
		default:
			return false;
	}
}

void UnitsyncLib::SetCurrentMod(const std::string& gamename)
{
	InitLib(m_init); // assumes the others are fine
	// (m_add_all_archives, m_get_mod_archive, m_get_mod_index)

	_SetCurrentMod(gamename);
}

void UnitsyncLib::_SetCurrentMod(const std::string& gamename)
{
	if (m_current_mod != gamename) {
		if (!m_current_mod.empty())
			_RemoveAllArchives();
		m_add_all_archives(m_get_mod_archive(m_get_mod_index(gamename.c_str())));
		m_current_mod = gamename;
	}
}

void UnitsyncLib::UnSetCurrentMod()
{
	LOCK_UNITSYNC;
	if (!m_current_mod.empty())
		_RemoveAllArchives();
	m_current_mod.clear();
}

int UnitsyncLib::GetModIndex(const std::string& name)
{
	return GetPrimaryModIndex(name);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  -- The UnitSync functions --
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


std::string UnitsyncLib::GetSpringVersion()
{
	InitLib(m_get_spring_version);
	std::string version = Util::SafeString(m_get_spring_version());
	if (m_is_spring_release_version && m_get_spring_version_patchset && m_is_spring_release_version()) {
		version += ".";
		version += m_get_spring_version_patchset();
	}
	return version;
}

std::string UnitsyncLib::GetSpringDataDir()
{
	InitLib(m_get_writeable_data_dir);
	return Util::SafeString(m_get_writeable_data_dir());
}

int UnitsyncLib::GetSpringDataDirCount()
{
	InitLib(m_get_data_dir_count);
	return m_get_data_dir_count();
}

std::string UnitsyncLib::GetSpringDataDirByIndex(const int index)
{
	InitLib(m_get_data_dir_by_index);
	return Util::SafeString(m_get_data_dir_by_index(index));
}

std::string UnitsyncLib::GetConfigFilePath()
{
	InitLib(m_get_spring_config_file_path);
	return Util::SafeString(m_get_spring_config_file_path());
}

int UnitsyncLib::GetMapCount()
{
	InitLib(m_get_map_count);
	return m_get_map_count();
}

unsigned int UnitsyncLib::GetMapChecksum(int index)
{
	InitLib(m_get_map_checksum);
	return m_get_map_checksum(index);
}

std::string UnitsyncLib::GetMapName(int index)
{
	InitLib(m_get_map_name);
	return Util::SafeString(m_get_map_name(index));
}

int UnitsyncLib::GetMapArchiveCount(int index)
{
	InitLib(m_get_map_archive_count);
	return m_get_map_archive_count(m_get_map_name(index));
}

std::string UnitsyncLib::GetMapArchiveName(int arnr)
{
	InitLib(m_get_map_archive_name);
	return Util::SafeString(m_get_map_archive_name(arnr));
}

UnitsyncLib::StringVector UnitsyncLib::GetMapDeps(int index)
{
	int count = GetMapArchiveCount(index);
	StringVector ret;
	for (int i = 0; i < count; i++) {
		ret.push_back(GetMapArchiveName(i));
	}
	return ret;
}

MapInfo UnitsyncLib::GetMapInfoEx(int index)
{
	MapInfo info;

	if (m_get_map_info_count != nullptr) { //new style fetching (>= spring 101.0)
		InitLib(m_get_map_info_count)
		    CHECK_FUNCTION(m_get_info_key);
		CHECK_FUNCTION(m_get_info_value_string);
		CHECK_FUNCTION(m_get_info_value_integer);
		CHECK_FUNCTION(m_get_info_value_float);

		const int infos = m_get_map_info_count(index);
		int x = 0;
		bool xset = false;
		for (int i = 0; i < infos; i++) {
			auto errors = GetUnitsyncErrors();
			for (const std::string& error : errors) {
				LslError("%s", error.c_str());
			}
			const std::string& key = Util::SafeString(m_get_info_key(i));
			if (key == "description") {
				info.description = Util::SafeString(m_get_info_value_string(i));
				continue;
			}
			if (key == "author") {
				info.author = Util::SafeString(m_get_info_value_string(i));
				continue;
			}
			if (key == "tidalStrength") {
				info.tidalStrength = m_get_info_value_integer(i);
				continue;
			}
			if (key == "gravity") {
				info.gravity = m_get_info_value_integer(i);
				continue;
			}
			if (key == "maxMetal") {
				info.maxMetal = m_get_info_value_float(i);
				continue;
			}
			if (key == "extractorRadius") {
				info.extractorRadius = m_get_info_value_integer(i);
				continue;
			}
			if (key == "minWind") {
				info.minWind = m_get_info_value_integer(i);
				continue;
			}
			if (key == "maxWind") {
				info.maxWind = m_get_info_value_integer(i);
				continue;
			}
			if (key == "width") {
				info.width = m_get_info_value_integer(i);
				continue;
			}
			if (key == "height") {
				info.height = m_get_info_value_integer(i);
				continue;
			}
			if (key == "resource") {
				// FIXME: implement ?!
				//info.resource = m_get_info_value_integer(i);
				continue;
			}
			if (key == "xPos") {
				const std::string type = Util::SafeString(m_get_info_type(i));
				if (type == "integer") {
					x = m_get_info_value_integer(i);
				} else if (type == "float") {
					x = m_get_info_value_float(i);
				} else {
					LslWarning("Unknown datatype for start position: %s", type.c_str());
				}
				xset = true;
				continue;
			}
			if (key == "zPos") {
				assert(xset);
				LSL::StartPos pos;
				pos.x = x;
				const std::string type = Util::SafeString(m_get_info_type(i));
				if (type == "integer") {
					pos.y = m_get_info_value_integer(i);
				} else if (type == "float") {
					pos.y = m_get_info_value_float(i);
				} else {
					LslWarning("Unknown datatype for start position: %s", type.c_str());
				}

				info.positions.push_back(pos);
				xset = false;
				continue;
			}
			LslWarning("Unknown key in GetMapInfoCount(): %s", key.c_str());
		}
		return info;
	}
	//deprecated style of fetching (<= spring 100.0)
	InitLib(m_get_map_description);
	info.description = Util::SafeString(m_get_map_description(index));
	info.tidalStrength = m_get_map_tidalStrength(index);
	info.gravity = m_get_map_gravity(index);

	const int resCount = m_get_map_resource_count(index);
	if (resCount > 0) {
		const int resourceIndex = 0;
		info.maxMetal = m_get_map_resource_max(index, resourceIndex);
		info.extractorRadius = m_get_map_resource_extractorRadius(index, resourceIndex);
	} else {
		info.maxMetal = 0.0f;
		info.extractorRadius = 0.0f;
	}

	info.minWind = m_get_map_windMin(index);
	info.maxWind = m_get_map_windMax(index);

	info.width = m_get_map_width(index);
	info.height = m_get_map_height(index);
	const int posCount = m_get_map_pos_count(index);
	for (int p = 0; p < posCount; ++p) {
		StartPos sp;
		sp.x = m_get_map_pos_x(index, p);
		sp.y = m_get_map_pos_z(index, p);
		info.positions.push_back(sp);
	}
	const char* author = m_get_map_author(index);
	if (author == NULL)
		info.author = "";
	else
		info.author = m_get_map_author(index);
	return info;
}

UnitsyncImage UnitsyncLib::GetMinimap(const std::string& mapFileName)
{
	InitLib(m_get_minimap);
	const int miplevel = 1; // miplevel should not be 10 ffs
	const int width = 1024 >> miplevel;
	const int height = 1024 >> miplevel;
	// this unitsync call returns a pointer to a static buffer
	unsigned short* colors = (unsigned short*)m_get_minimap(mapFileName.c_str(), miplevel);
	if (!colors)
		LSL_THROWF(unitsync, "Get minimap failed %s", mapFileName.c_str());
	UnitsyncImage img = UnitsyncImage::FromMinimapData(colors, width, height);
	return img;
}

UnitsyncImage UnitsyncLib::GetMetalmap(const std::string& mapFileName)
{
	InitLib(m_get_infomap_size); // assume GetInfoMap is available too
	int width = 0, height = 0, retval;
	retval = m_get_infomap_size(mapFileName.c_str(), "metal", &width, &height);
	if (!(retval != 0 && width * height != 0))
		LSL_THROWF(unitsync, "Get metalmap size failed %s", mapFileName.c_str());
	Util::uninitialized_array<unsigned char> grayscale(width * height);
	retval = m_get_infomap(mapFileName.c_str(), "metal", grayscale, 1 /*byte per pixel*/);
	if (retval == 0)
		LSL_THROWF(unitsync, "Get metalmap failed %s", mapFileName.c_str());
	UnitsyncImage img = UnitsyncImage::FromMetalmapData(grayscale, width, height);
	img.RescaleIfBigger();
	return img;
}

UnitsyncImage UnitsyncLib::GetHeightmap(const std::string& mapFileName)
{
	InitLib(m_get_infomap_size); // assume GetInfoMap is available too
	int width = 0, height = 0, retval;
	retval = m_get_infomap_size(mapFileName.c_str(), "height", &width, &height);
	if (!(retval != 0 && width * height != 0))
		LSL_THROWF(unitsync, "Get heightmap size failed %s", mapFileName.c_str());
	Util::uninitialized_array<unsigned short> grayscale(width * height);
	retval = m_get_infomap(mapFileName.c_str(), "height", grayscale, 2 /*byte per pixel*/);
	if (retval == 0)
		LSL_THROW(unitsync, "Get heightmap failed");
	UnitsyncImage img = UnitsyncImage::FromHeightmapData(grayscale, width, height);
	img.RescaleIfBigger();
	return img;
}


int UnitsyncLib::GetPrimaryModIndex(const std::string& modName)
{
	InitLib(m_get_mod_index);
	return m_get_mod_index(modName.c_str());
}

int UnitsyncLib::GetPrimaryModCount()
{
	InitLib(m_get_mod_count);
	return m_get_mod_count();
}

std::string UnitsyncLib::GetPrimaryModArchive(int index)
{
	InitLib(m_get_mod_archive);
	CHECK_FUNCTION(m_get_mod_count);
	int count = m_get_mod_count();
	if (index >= count)
		LSL_THROW(unitsync, "index out of bounds");
	return Util::SafeString(m_get_mod_archive(index));
}

int UnitsyncLib::GetPrimaryModArchiveCount(int index)
{
	InitLib(m_get_primary_mod_archive_count);
	return m_get_primary_mod_archive_count(index);
}

std::string UnitsyncLib::GetPrimaryModArchiveList(int arnr)
{
	InitLib(m_get_primary_mod_archive_list);
	return Util::SafeString(m_get_primary_mod_archive_list(arnr));
}

unsigned int UnitsyncLib::GetPrimaryModChecksumFromName(const std::string& name)
{
	InitLib(m_get_primary_mod_checksum_from_name);
	return m_get_primary_mod_checksum_from_name(name.c_str());
}

unsigned int UnitsyncLib::GetMapChecksumFromName(const std::string& name)
{
	InitLib(m_get_map_checksum_from_name);
	return m_get_map_checksum_from_name(name.c_str());
}

UnitsyncLib::StringVector UnitsyncLib::GetModDeps(int index)
{
	const int count = GetPrimaryModArchiveCount(index);
	StringVector ret;
	for (int i = 0; i < count; i++)
		ret.push_back(GetPrimaryModArchiveList(i));
	return ret;
}

UnitsyncLib::StringVector UnitsyncLib::GetSides(const std::string& modName)
{
	InitLib(m_get_side_count);
	if (!m_get_side_name)
		LSL_THROW(function_missing, "m_get_side_name");
	_SetCurrentMod(modName);
	int count = m_get_side_count();
	StringVector ret;
	for (int i = 0; i < count; i++)
		ret.push_back(m_get_side_name(i));
	return ret;
}

void UnitsyncLib::AddAllArchives(const std::string& root)
{
	InitLib(m_add_all_archives);
	m_add_all_archives(root.c_str());
}

void UnitsyncLib::AddArchive(const std::string& name)
{
	InitLib(m_add_archive);
	m_add_archive(name.c_str());
}

std::string UnitsyncLib::GetFullUnitName(int index)
{
	InitLib(m_get_unit_full_name);
	return Util::SafeString(m_get_unit_full_name(index));
}

std::string UnitsyncLib::GetUnitName(int index)
{
	InitLib(m_get_unit_name);
	return Util::SafeString(m_get_unit_name(index));
}

int UnitsyncLib::GetUnitCount()
{
	InitLib(m_get_unit_count);
	return m_get_unit_count();
}

int UnitsyncLib::ProcessUnits()
{
	InitLib(m_process_units);
	return m_process_units();
}

UnitsyncLib::StringVector UnitsyncLib::FindFilesVFS(const std::string& name)
{
	InitLib(m_find_files_vfs);
	CHECK_FUNCTION(m_init_find_vfs);
	int handle = m_init_find_vfs(name.c_str());
	StringVector ret;
	//thanks to assbars awesome edit we now get different invalid values from init and find
	if (handle != -1) {
		do {
			char buffer[1025];
			handle = m_find_files_vfs(handle, &buffer[0], 1024);
			buffer[1024] = 0;
			ret.push_back(&buffer[0]);
		} while (handle);
	}
	return ret;
}

int UnitsyncLib::OpenFileVFS(const std::string& name)
{
	assert(name[0] != '/');
	InitLib(m_open_file_vfs);
	return m_open_file_vfs(name.c_str());
}

int UnitsyncLib::FileSizeVFS(int handle)
{
	InitLib(m_file_size_vfs);
	return m_file_size_vfs(handle);
}

int UnitsyncLib::ReadFileVFS(int handle, void* buffer, int bufferLength)
{
	InitLib(m_read_file_vfs);
	return m_read_file_vfs(handle, buffer, bufferLength);
}

void UnitsyncLib::CloseFileVFS(int handle)
{
	InitLib(m_close_file_vfs);
	m_close_file_vfs(handle);
}

unsigned int UnitsyncLib::GetValidMapCount(const std::string& gamename)
{
	InitLib(m_get_mod_valid_map_count);
	_SetCurrentMod(gamename);
	return m_get_mod_valid_map_count();
}

std::string UnitsyncLib::GetValidMapName(unsigned int MapIndex)
{
	InitLib(m_get_valid_map);
	return Util::SafeString(m_get_valid_map(MapIndex));
}

int UnitsyncLib::GetMapOptionCount(const std::string& name)
{
	InitLib(m_get_map_option_count);
	if (name.empty())
		LSL_THROW(unitsync, "tried to pass empty mapname to unitsync");
	return m_get_map_option_count(name.c_str());
}

int UnitsyncLib::GetCustomOptionCount(const std::string& archive_name, const std::string& filename)
{
	InitLib(m_get_custom_option_count);
	if (archive_name.empty())
		LSL_THROW(unitsync, "tried to pass empty archive_name to unitsync");
	_RemoveAllArchives();
	m_add_all_archives(archive_name.c_str());
	return m_get_custom_option_count(filename.c_str());
}

int UnitsyncLib::GetModOptionCount(const std::string& name)
{
	InitLib(m_get_mod_option_count);
	if (name.empty())
		LSL_THROW(unitsync, "tried to pass empty gamename to unitsync");
	_SetCurrentMod(name);
	return m_get_mod_option_count();
}

int UnitsyncLib::GetAIOptionCount(const std::string& gamename, int aiIndex)
{
	InitLib(m_get_skirmish_ai_option_count);
	_SetCurrentMod(gamename);
	CHECK_FUNCTION(m_get_skirmish_ai_count);
	if (!((aiIndex >= 0) && (aiIndex < m_get_skirmish_ai_count())))
		LSL_THROW(unitsync, "aiIndex out of bounds");
	return m_get_skirmish_ai_option_count(aiIndex);
}

std::string UnitsyncLib::GetOptionKey(int optIndex)
{
	InitLib(m_get_option_key);
	return Util::SafeString(m_get_option_key(optIndex));
}

std::string UnitsyncLib::GetOptionName(int optIndex)
{
	InitLib(m_get_option_name);
	return Util::SafeString(m_get_option_name(optIndex));
}

std::string UnitsyncLib::GetOptionDesc(int optIndex)
{
	InitLib(m_get_option_desc);
	return Util::SafeString(m_get_option_desc(optIndex));
}

std::string UnitsyncLib::GetOptionSection(int optIndex)
{
	InitLib(m_get_option_section);
	return Util::SafeString(m_get_option_section(optIndex));
}

int UnitsyncLib::GetOptionType(int optIndex)
{
	InitLib(m_get_option_type);
	return m_get_option_type(optIndex);
}

int UnitsyncLib::GetOptionBoolDef(int optIndex)
{
	InitLib(m_get_option_bool_def);
	return m_get_option_bool_def(optIndex);
}

float UnitsyncLib::GetOptionNumberDef(int optIndex)
{
	InitLib(m_get_option_number_def);
	return m_get_option_number_def(optIndex);
}

float UnitsyncLib::GetOptionNumberMin(int optIndex)
{
	InitLib(m_get_option_number_min);
	return m_get_option_number_min(optIndex);
}

float UnitsyncLib::GetOptionNumberMax(int optIndex)
{
	InitLib(m_get_option_number_max);
	return m_get_option_number_max(optIndex);
}

float UnitsyncLib::GetOptionNumberStep(int optIndex)
{
	InitLib(m_get_option_number_step);
	return m_get_option_number_step(optIndex);
}

std::string UnitsyncLib::GetOptionStringDef(int optIndex)
{
	InitLib(m_get_option_string_def);
	return Util::SafeString(m_get_option_string_def(optIndex));
}

int UnitsyncLib::GetOptionStringMaxLen(int optIndex)
{
	InitLib(m_get_option_string_max_len);
	return m_get_option_string_max_len(optIndex);
}

int UnitsyncLib::GetOptionListCount(int optIndex)
{
	InitLib(m_get_option_list_count);
	return m_get_option_list_count(optIndex);
}

std::string UnitsyncLib::GetOptionListDef(int optIndex)
{
	InitLib(m_get_option_list_def);
	return Util::SafeString(m_get_option_list_def(optIndex));
}

std::string UnitsyncLib::GetOptionListItemKey(int optIndex, int itemIndex)
{
	InitLib(m_get_option_list_item_key);
	return Util::SafeString(m_get_option_list_item_key(optIndex, itemIndex));
}

std::string UnitsyncLib::GetOptionListItemName(int optIndex, int itemIndex)
{
	InitLib(m_get_option_list_item_name);
	return Util::SafeString(m_get_option_list_item_name(optIndex, itemIndex));
}

std::string UnitsyncLib::GetOptionListItemDesc(int optIndex, int itemIndex)
{
	InitLib(m_get_option_list_item_desc);
	return Util::SafeString(m_get_option_list_item_desc(optIndex, itemIndex));
}

int UnitsyncLib::OpenArchive(const std::string& name)
{
	InitLib(m_open_archive);
	return m_open_archive(name.c_str());
}

void UnitsyncLib::CloseArchive(int archive)
{
	InitLib(m_close_archive);
	m_close_archive(archive);
}

int UnitsyncLib::FindFilesArchive(int archive, int cur, std::string& nameBuf)
{
	InitLib(m_find_Files_archive);
	char buffer[1025];
	int size = 1024;
	bool ret = m_find_Files_archive(archive, cur, &buffer[0], &size);
	buffer[1024] = 0;
	nameBuf = &buffer[0];
	return ret;
}

int UnitsyncLib::OpenArchiveFile(int archive, const std::string& name)
{
	InitLib(m_open_archive_file);
	return m_open_archive_file(archive, name.c_str());
}

int UnitsyncLib::ReadArchiveFile(int archive, int handle, void* buffer, int numBytes)
{
	InitLib(m_read_archive_file);
	return m_read_archive_file(archive, handle, buffer, numBytes);
}

void UnitsyncLib::CloseArchiveFile(int archive, int handle)
{
	InitLib(m_close_archive_file);
	m_close_archive_file(archive, handle);
}

int UnitsyncLib::SizeArchiveFile(int archive, int handle)
{
	InitLib(m_size_archive_file);
	return m_size_archive_file(archive, handle);
}

std::string UnitsyncLib::GetArchivePath(const std::string& name)
{
	InitLib(m_get_archive_path);
	return Util::SafeString(m_get_archive_path(name.c_str()));
}

int UnitsyncLib::GetSpringConfigInt(const std::string& key, int defValue)
{
	InitLib(m_get_spring_config_int);
	return m_get_spring_config_int(key.c_str(), defValue);
}

std::string UnitsyncLib::GetSpringConfigString(const std::string& key, const std::string& defValue)
{
	InitLib(m_get_spring_config_string);
	return Util::SafeString(m_get_spring_config_string(key.c_str(), defValue.c_str()));
}

float UnitsyncLib::GetSpringConfigFloat(const std::string& key, const float defValue)
{
	InitLib(m_get_spring_config_float);
	return m_get_spring_config_float(key.c_str(), defValue);
}

void UnitsyncLib::SetSpringConfigString(const std::string& key, const std::string& value)
{
	InitLib(m_set_spring_config_string);
	m_set_spring_config_string(key.c_str(), value.c_str());
}

void UnitsyncLib::SetSpringConfigInt(const std::string& key, int value)
{
	InitLib(m_set_spring_config_int);
	m_set_spring_config_int(key.c_str(), value);
}


void UnitsyncLib::SetSpringConfigFloat(const std::string& key, const float value)
{
	InitLib(m_set_spring_config_float);

	m_set_spring_config_float(key.c_str(), value);
}

int UnitsyncLib::GetSkirmishAICount(const std::string& gamename)
{
	InitLib(m_get_skirmish_ai_count);
	_SetCurrentMod(gamename);
	return m_get_skirmish_ai_count();
}

UnitsyncLib::StringVector UnitsyncLib::GetAIInfo(int aiIndex)
{
	InitLib(m_get_skirmish_ai_count);
	CHECK_FUNCTION(m_get_skirmish_ai_info_count);
	CHECK_FUNCTION(m_get_description);
	CHECK_FUNCTION(m_get_info_key);
	CHECK_FUNCTION(m_get_info_value_string);

	StringVector ret;
	if (!((aiIndex >= 0) && (aiIndex < m_get_skirmish_ai_count())))
		LSL_THROW(unitsync, "aiIndex out of bounds");

	int infoCount = m_get_skirmish_ai_info_count(aiIndex);
	for (int i = 0; i < infoCount; i++) {
		ret.push_back(m_get_info_key(i));
		ret.push_back(m_get_info_value_string(i));
		ret.push_back(m_get_description(i));
	}
	return ret;
}

unsigned int UnitsyncLib::GetArchiveChecksum(const std::string& VFSPath)
{
	InitLib(m_get_archive_checksum);
	return m_get_archive_checksum(VFSPath.c_str());
}

/// lua parser

void UnitsyncLib::CloseParser()
{
	InitLib(m_parser_close);
	m_parser_close();
}

bool UnitsyncLib::OpenParserFile(const std::string& filename, const std::string& filemodes, const std::string& accessModes)
{
	InitLib(m_parser_open_file);
	return m_parser_open_file(filename.c_str(), filemodes.c_str(), accessModes.c_str());
}

bool UnitsyncLib::OpenParserSource(const std::string& source, const std::string& accessModes)
{
	InitLib(m_parser_open_source);
	return m_parser_open_source(source.c_str(), accessModes.c_str());
}

bool UnitsyncLib::ParserExecute()
{
	InitLib(m_parser_execute);
	return m_parser_execute();
}

std::string UnitsyncLib::ParserErrorLog()
{
	InitLib(m_parser_error_log);
	return Util::SafeString(m_parser_error_log());
}

void UnitsyncLib::ParserAddTable(int key, bool override)
{
	InitLib(m_parser_add_table_int);
	m_parser_add_table_int(key, override);
}

void UnitsyncLib::ParserAddTable(const std::string& key, bool override)
{
	InitLib(m_parser_add_table_string);
	m_parser_add_table_string(key.c_str(), override);
}

void UnitsyncLib::ParserEndTable()
{
	InitLib(m_parser_end_table);
	m_parser_end_table();
}

void UnitsyncLib::ParserAddTableValue(int key, int val)
{
	InitLib(m_parser_add_int_key_int_value);
	m_parser_add_int_key_int_value(key, val);
}

void UnitsyncLib::ParserAddTableValue(const std::string& key, int val)
{
	InitLib(m_parser_add_string_key_int_value);
	m_parser_add_string_key_int_value(key.c_str(), val);
}

void UnitsyncLib::ParserAddTableValue(int key, bool val)
{
	InitLib(m_parser_add_int_key_int_value);
	m_parser_add_int_key_int_value(key, val);
}

void UnitsyncLib::ParserAddTableValue(const std::string& key, bool val)
{
	InitLib(m_parser_add_string_key_int_value);
	m_parser_add_string_key_int_value(key.c_str(), val);
}

void UnitsyncLib::ParserAddTableValue(int key, const std::string& val)
{
	InitLib(m_parser_add_int_key_string_value);
	m_parser_add_int_key_string_value(key, val.c_str());
}

void UnitsyncLib::ParserAddTableValue(const std::string& key, const std::string& val)
{
	InitLib(m_parser_add_string_key_string_value);
	m_parser_add_string_key_string_value(key.c_str(), val.c_str());
}

void UnitsyncLib::ParserAddTableValue(int key, float val)
{
	InitLib(m_parser_add_int_key_float_value);
	m_parser_add_int_key_float_value(key, val);
}

void UnitsyncLib::ParserAddTableValue(const std::string& key, float val)
{
	InitLib(m_parser_add_string_key_float_value);
	m_parser_add_string_key_float_value(key.c_str(), val);
}

bool UnitsyncLib::ParserGetRootTable()
{
	InitLib(m_parser_root_table);
	return m_parser_root_table();
}

bool UnitsyncLib::ParserGetRootTableExpression(const std::string& exp)
{
	InitLib(m_parser_root_table_expression);
	return m_parser_root_table_expression(exp.c_str());
}

bool UnitsyncLib::ParserGetSubTableInt(int key)
{
	InitLib(m_parser_sub_table_int);
	return m_parser_sub_table_int(key);
}

bool UnitsyncLib::ParserGetSubTableString(const std::string& key)
{
	InitLib(m_parser_sub_table_string);
	return m_parser_sub_table_string(key.c_str());
}

bool UnitsyncLib::ParserGetSubTableInt(const std::string& exp)
{
	InitLib(m_parser_sub_table_expression);
	return m_parser_sub_table_expression(exp.c_str());
}

void UnitsyncLib::ParserPopTable()
{
	InitLib(m_parser_pop_table);
	m_parser_pop_table();
}

bool UnitsyncLib::ParserKeyExists(int key)
{
	InitLib(m_parser_key_int_exists);
	return m_parser_key_int_exists(key);
}

bool UnitsyncLib::ParserKeyExists(const std::string& key)
{
	InitLib(m_parser_key_string_exists);
	return m_parser_key_string_exists(key.c_str());
}

int UnitsyncLib::ParserGetKeyType(int key)
{
	InitLib(m_parser_int_key_get_type);
	return m_parser_int_key_get_type(key);
}

int UnitsyncLib::ParserGetKeyType(const std::string& key)
{
	InitLib(m_parser_string_key_get_type);
	return m_parser_string_key_get_type(key.c_str());
}

int UnitsyncLib::ParserGetIntKeyListCount()
{
	InitLib(m_parser_int_key_get_list_count);
	return m_parser_int_key_get_list_count();
}

int UnitsyncLib::ParserGetIntKeyListEntry(int index)
{
	InitLib(m_parser_int_key_get_list_entry);
	return m_parser_int_key_get_list_entry(index);
}

int UnitsyncLib::ParserGetStringKeyListCount()
{
	InitLib(m_parser_string_key_get_list_count);
	return m_parser_string_key_get_list_count();
}

int UnitsyncLib::ParserGetStringKeyListEntry(int index)
{
	InitLib(m_parser_int_key_get_list_entry);
	return m_parser_int_key_get_list_entry(index);
}

int UnitsyncLib::GetKeyValue(int key, int defval)
{
	InitLib(m_parser_int_key_get_int_value);
	return m_parser_int_key_get_int_value(key, defval);
}

bool UnitsyncLib::GetKeyValue(int key, bool defval)
{
	InitLib(m_parser_int_key_get_bool_value);
	return m_parser_int_key_get_bool_value(key, defval);
}

std::string UnitsyncLib::GetKeyValue(int key, const std::string& defval)
{
	InitLib(m_parser_int_key_get_string_value);
	return Util::SafeString(m_parser_int_key_get_string_value(key, defval.c_str()));
}

float UnitsyncLib::GetKeyValue(int key, float defval)
{
	InitLib(m_parser_int_key_get_float_value);
	return m_parser_int_key_get_float_value(key, defval);
}

int UnitsyncLib::GetKeyValue(const std::string& key, int defval)
{
	InitLib(m_parser_string_key_get_int_value);
	return m_parser_string_key_get_int_value(key.c_str(), defval);
}

bool UnitsyncLib::GetKeyValue(const std::string& key, bool defval)
{
	InitLib(m_parser_string_key_get_bool_value);
	return m_parser_string_key_get_bool_value(key.c_str(), defval);
}

std::string UnitsyncLib::GetKeyValue(const std::string& key, const std::string& defval)
{
	InitLib(m_parser_string_key_get_string_value);
	return Util::SafeString(m_parser_string_key_get_string_value(key.c_str(), defval.c_str()));
}

float UnitsyncLib::GetKeyValue(const std::string& key, float defval)
{
	InitLib(m_parser_string_key_get_float_value);
	return m_parser_string_key_get_float_value(key.c_str(), defval);
}

int UnitsyncLib::GetPrimaryModInfoCount(int index)
{
	InitLib(m_get_primary_mod_info_count);
	return m_get_primary_mod_info_count(index);
}

const char* UnitsyncLib::GetInfoKey(int index)
{
	InitLib(m_get_info_key);
	return m_get_info_key(index);
}

const char* UnitsyncLib::GetInfoType(int index)
{
	InitLib(m_get_info_type);
	return m_get_info_type(index);
}

const char* UnitsyncLib::GetInfoValueString(int index)
{
	InitLib(m_get_info_value_string);
	return m_get_info_value_string(index);
}

int UnitsyncLib::GetInfoValueInteger(int index)
{
	InitLib(m_get_info_value_integer);
	return m_get_info_value_integer(index);
}

float UnitsyncLib::GetInfoValueFloat(int index)
{
	InitLib(m_get_info_value_float);
	return m_get_info_value_float(index);
}

bool UnitsyncLib::GetInfoValueBool(int index)
{
	InitLib(m_get_info_value_bool);
	return m_get_info_value_bool(index);
}

void UnitsyncLib::DeleteSpringConfigKey(const std::string& key)
{
	InitLib(m_delete_spring_config_key);
	m_delete_spring_config_key(key.c_str());
}

std::string UnitsyncLib::GetMacHash()
{
	if (m_mac_hash == nullptr)
		return "0";
	InitLib(m_mac_hash);
	return Util::SafeString(m_mac_hash());
}

std::string UnitsyncLib::GetSysHash()
{
	if (m_sys_hash == nullptr)
		return "0";
	InitLib(m_sys_hash);
	return Util::SafeString(m_sys_hash());
}

UnitsyncLib& susynclib()
{
	static LSL::Util::LineInfo<UnitsyncLib> m(AT);
	static LSL::Util::GlobalObjectHolder<UnitsyncLib, LSL::Util::LineInfo<UnitsyncLib> > ss(m);
	return ss;
}

} //namespace LSL
