/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LIBSPRINGLOBBY_HEADERGUARD_SPRINGUNITSYNCLIB_H
#define LIBSPRINGLOBBY_HEADERGUARD_SPRINGUNITSYNCLIB_H

#include <string>
#include <stdexcept>

#include "data.h"
#include "signatures.h"
#include <lslutils/type_forwards.h>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>

namespace LSL
{

class UnitsyncImage;
struct UnitsyncFunctionLoader;

static const unsigned int MapInfoMaxStartPositions = 16;

/** \todo needs to be replaced in favor of the ones in out excpetion header **/
/*  FIXME: doesn't catch LSL::unitsync
class unitsync_assert : public std::runtime_error
{
public:
	unitsync_assert(std::string msg) : std::runtime_error(msg) {}
};
*/

//! Everything spring makes available about a map
struct SpringMapInfo {
	char* description;
	int tidalStrength;
	int gravity;
	float maxMetal;
	int extractorRadius;
	int minWind;
	int maxWind;

	int width;
	int height;
	int posCount;
	StartPos positions[MapInfoMaxStartPositions];

	char* author;

	SpringMapInfo()
	    : description(NULL)
	    , tidalStrength(0)
	    , gravity(0)
	    , maxMetal(0.0f)
	    , extractorRadius(0)
	    , minWind(0)
	    , maxWind(0)
	    , width(0)
	    , height(0)
	    , posCount(0)
	    , author(NULL)
	{
	}
};

/**
 * \brief Primitive class handling the unitsync library.
 *
 * This class is - in a limited way - thread safe but may block execution
 * in case two threads use it at the same time.  The thread safety ensures
 * there can never be multiple threads executing unitsync functions at the
 * same time.  However, many unitsync functions use (hidden) global state,
 * so often there is a need for running multiple unitsync methods while
 * holding a single lock continuously.
 */
class UnitsyncLib : public boost::noncopyable
{
	//! we use this to offload the mind numblingly boring pointer loading
	friend struct UnitsyncFunctionLoader;

public:
	/**
	 * Constructor.
	 */
	UnitsyncLib();

	/**
	 * Destructor, unloads unitsync if loaded.
	 */
	~UnitsyncLib();

	/**
	 * Loads the unitsync library from path.
	 * @param path path to the unitsync lib.
	 * @see Unload().
	 * @note Throws runtime_error if load failed.
	 */
	bool Load(const std::string& path);

	/**
	 * Unload the unitsync library. Does nothing if not loaded.
	 * @see Load().
	 */
	void Unload();

	/**
	 * Returns true if the library is loaded.
	 */
	bool IsLoaded() const;

	/**
	 * Get list of errors from unitsync library in an array
	 */
	std::vector<std::string> GetUnitsyncErrors() const;

	bool VersionSupports(LSL::GameFeature feature) const;


	int GetModIndex(const std::string& name);

	std::string GetSpringVersion();

	std::string GetSpringDataDir();
	int GetSpringDataDirCount();
	std::string GetSpringDataDirByIndex(const int index);
	std::string GetConfigFilePath();

	int GetMapCount();
	unsigned int GetMapChecksum(int index);
	std::string GetMapName(int index);
	int GetMapArchiveCount(int index);
	std::string GetMapArchiveName(int arnr);

	typedef std::vector<std::string>
	    StringVector;
	StringVector GetMapDeps(int index);

	/**
	 * @brief Get information about a map.
	 * @note Throws assert_exception if unsuccessful.
	 */
	MapInfo GetMapInfoEx(int index);

	/**
	 * @brief Get minimap.
	 * @note Throws assert_exception if unsuccessful.
	 */
	UnitsyncImage GetMinimap(const std::string& mapFileName);

	/**
	 * @brief Get metalmap.
	 * @note Throws assert_exception if unsuccessful.
	 */
	UnitsyncImage GetMetalmap(const std::string& mapFileName);

	/**
	 * @brief Get heightmap.
	 * @note Throws assert_exception if unsuccesful.
	 */
	UnitsyncImage GetHeightmap(const std::string& mapFileName);

	int GetPrimaryModIndex(const std::string& modName);
	int GetPrimaryModCount();
	std::string GetPrimaryModArchive(int index);
	int GetPrimaryModArchiveCount(int index);
	std::string GetPrimaryModArchiveList(int arnr);
	unsigned int GetPrimaryModChecksumFromName(const std::string& name);
	StringVector GetModDeps(int index);

	StringVector GetSides(const std::string& modName);

	/**
	 * Add all achives.
	 * @note Not sure what this does, but adding the mod archive path to this when setting new mod seems to work :)
	 */
	void AddAllArchives(const std::string& root);
	void AddArchive(const std::string& name);

	void SetCurrentMod(const std::string& gamename);
	void UnSetCurrentMod();

	std::string GetFullUnitName(int index);
	std::string GetUnitName(int index);
	int GetUnitCount();
	int ProcessUnits();

	/**
	 * Search for a file pattern.
	 * @param the search patterns
	 * @return wxarraystring of results
	 */
	StringVector FindFilesVFS(const std::string& name);
	int OpenFileVFS(const std::string& name);
	int FileSizeVFS(int handle);
	int ReadFileVFS(int handle, void* buffer, int bufferLength);
	void CloseFileVFS(int handle);

	unsigned int GetValidMapCount(const std::string& gamename);
	std::string GetValidMapName(unsigned int MapIndex);

	int GetMapOptionCount(const std::string& name);
	int GetCustomOptionCount(const std::string& gamename, const std::string& filename);
	int GetModOptionCount(const std::string& name);
	int GetAIOptionCount(const std::string& gamename, int index);
	std::string GetOptionKey(int optIndex);
	std::string GetOptionName(int optIndex);
	std::string GetOptionDesc(int optIndex);
	std::string GetOptionSection(int optIndex);
	int GetOptionType(int optIndex);
	int GetOptionBoolDef(int optIndex);
	float GetOptionNumberDef(int optIndex);
	float GetOptionNumberMin(int optIndex);
	float GetOptionNumberMax(int optIndex);
	float GetOptionNumberStep(int optIndex);
	std::string GetOptionStringDef(int optIndex);
	int GetOptionStringMaxLen(int optIndex);
	int GetOptionListCount(int optIndex);
	std::string GetOptionListDef(int optIndex);
	std::string GetOptionListItemKey(int optIndex, int itemIndex);
	std::string GetOptionListItemName(int optIndex, int itemIndex);
	std::string GetOptionListItemDesc(int optIndex, int itemIndex);

	int OpenArchive(const std::string& name);
	void CloseArchive(int archive);
	int FindFilesArchive(int archive, int cur, std::string& nameBuf);
	int OpenArchiveFile(int archive, const std::string& name);
	int ReadArchiveFile(int archive, int handle, void* buffer, int numBytes);
	void CloseArchiveFile(int archive, int handle);
	int SizeArchiveFile(int archive, int handle);
	std::string GetArchivePath(const std::string& name);

	int GetSpringConfigInt(const std::string& key, int defValue);
	std::string GetSpringConfigString(const std::string& key, const std::string& defValue);
	float GetSpringConfigFloat(const std::string& key, const float defValue);
	void SetSpringConfigString(const std::string& key, const std::string& value);
	void SetSpringConfigInt(const std::string& key, int value);
	void SetSpringConfigFloat(const std::string& key, const float value);
	void DeleteSpringConfigKey(const std::string& key);

	/// AI info
	int GetSkirmishAICount(const std::string& gamename);
	/**
	 * Get next search result.
	 * @param the AI index within range of GetSkirmishAIInfoCount
	 * @return an array made of blocks with this layout { key, value, description }
	 */
	StringVector GetAIInfo(int index);

	unsigned int GetArchiveChecksum(const std::string& VFSPath);

	/// lua parser

	void CloseParser();
	bool OpenParserFile(const std::string& filename, const std::string& filemodes, const std::string& accessModes);
	bool OpenParserSource(const std::string& source, const std::string& accessModes);
	bool ParserExecute();
	std::string ParserErrorLog();

	void ParserAddTable(int key, bool override);
	void ParserAddTable(const std::string& key, bool override);
	void ParserEndTable();
	void ParserAddTableValue(int key, int val);
	void ParserAddTableValue(const std::string& key, int val);
	void ParserAddTableValue(int key, bool val);
	void ParserAddTableValue(const std::string& key, bool val);
	void ParserAddTableValue(int key, const std::string& val);
	void ParserAddTableValue(const std::string& key, const std::string& val);
	void ParserAddTableValue(int key, float val);
	void ParserAddTableValue(const std::string& key, float val);

	bool ParserGetRootTable();
	bool ParserGetRootTableExpression(const std::string& exp);
	bool ParserGetSubTableInt(int key);
	bool ParserGetSubTableString(const std::string& key);
	bool ParserGetSubTableInt(const std::string& exp);
	void ParserPopTable();

	bool ParserKeyExists(int key);
	bool ParserKeyExists(const std::string& key);

	int ParserGetKeyType(int key);
	int ParserGetKeyType(const std::string& key);

	int ParserGetIntKeyListCount();
	int ParserGetIntKeyListEntry(int index);
	int ParserGetStringKeyListCount();
	int ParserGetStringKeyListEntry(int index);

	int GetKeyValue(int key, int defval);
	bool GetKeyValue(int key, bool defval);
	std::string GetKeyValue(int key, const std::string& defval);
	float GetKeyValue(int key, float defval);
	int GetKeyValue(const std::string& key, int defval);
	bool GetKeyValue(const std::string& key, bool defval);
	std::string GetKeyValue(const std::string& key, const std::string& defval);
	float GetKeyValue(const std::string& key, float defval);

	int GetPrimaryModInfoCount(int index);

	const char* GetInfoKey(int index);
	const char* GetInfoType(int index);

	const char* GetInfoValueString(int index);
	int GetInfoValueInteger(int index);
	float GetInfoValueFloat(int index);
	bool GetInfoValueBool(int index);

	void RemoveAllArchives();


private:
	UnitsyncLib(const UnitsyncLib&);
	//! Keeps track if unitsync is loaded or not.
	bool m_loaded;

	//! Handle to the unitsync library.
	void* m_libhandle;

	//! Critical section controlling access to unitsync functions.
	mutable boost::mutex m_lock;

	//! Path to unitsync.
	std::string m_path;

	//! the current loaded mod.
	std::string m_current_mod;

	/**
	 * Loads the unitsync library from path.
	 * @note this function is not threadsafe if called from code not locked.
	 * @see Load()
	 */
	void _Load(const std::string& path);

	/**
	 * Initializes unitsync.
	 */
	void _Init();

	/**
	 * Calls RemoveAllArchives if available, _Init() otherwise.
	 */
	void _RemoveAllArchives();

	/**
	 * Internal Unload() function.
	 * @note this function is not threadsafe if called from code not locked.
	 * @see Unload()
	 */
	void _Unload();

	void _ConvertSpringMapInfo(const SpringMapInfo& in, MapInfo& out);

	void _SetCurrentMod(const std::string& gamename);

	/**
     * \name function objects
     * Pointers to the functions in unitsync.
	 */
	///@{

	InitPtr m_init;
	UnInitPtr m_uninit;
	GetNextErrorPtr m_get_next_error;
	GetWritableDataDirectoryPtr m_get_writeable_data_dir;
	GetDataDirectoryPtr m_get_data_dir_by_index;
	GetDataDirectoryCountPtr m_get_data_dir_count;

	GetMapInfoCountPtr m_get_map_info_count;
	GetMapCountPtr m_get_map_count;
	GetMapChecksumPtr m_get_map_checksum;
	GetMapNamePtr m_get_map_name;
	GetMapDescriptionPtr m_get_map_description;
	GetMapAuthorPtr m_get_map_author;
	GetMapWidthPtr m_get_map_width;
	GetMapHeightPtr m_get_map_height;
	GetMapTidalStrengthPtr m_get_map_tidalStrength;
	GetMapWindMinPtr m_get_map_windMin;
	GetMapWindMaxPtr m_get_map_windMax;
	GetMapGravityPtr m_get_map_gravity;
	GetMapResourceCountPtr m_get_map_resource_count;
	GetMapResourceNamePtr m_get_map_resource_name;
	GetMapResourceMaxPtr m_get_map_resource_max;
	GetMapResourceExtractorRadiusPtr m_get_map_resource_extractorRadius;
	GetMapPosCountPtr m_get_map_pos_count;
	GetMapPosXPtr m_get_map_pos_x;
	GetMapPosZPtr m_get_map_pos_z;
	GetMinimapPtr m_get_minimap;
	GetInfoMapSizePtr m_get_infomap_size;
	GetInfoMapPtr m_get_infomap;

	GetPrimaryModIndexPtr m_get_mod_index;
	GetPrimaryModCountPtr m_get_mod_count;
	GetPrimaryModArchivePtr m_get_mod_archive;

	GetSideCountPtr m_get_side_count;
	GetSideNamePtr m_get_side_name;

	AddAllArchivesPtr m_add_all_archives;
	RemoveAllArchivesPtr m_remove_all_archives;

	GetUnitCountPtr m_get_unit_count;
	GetUnitNamePtr m_get_unit_name;
	GetFullUnitNamePtr m_get_unit_full_name;

	InitFindVFSPtr m_init_find_vfs;
	FindFilesVFSPtr m_find_files_vfs;
	OpenFileVFSPtr m_open_file_vfs;
	FileSizeVFSPtr m_file_size_vfs;
	ReadFileVFSPtr m_read_file_vfs;
	CloseFileVFSPtr m_close_file_vfs;

	GetSpringVersionPtr m_get_spring_version;
	GetSpringVersionPatchsetPtr m_get_spring_version_patchset;
	IsSpringReleaseVersionPtr m_is_spring_release_version;

	ProcessUnitsPtr m_process_units;
	AddArchivePtr m_add_archive;
	GetArchiveChecksumPtr m_get_archive_checksum;
	GetArchivePathPtr m_get_archive_path;
	GetMapArchiveCountPtr m_get_map_archive_count;
	GetMapArchiveNamePtr m_get_map_archive_name;
	GetMapChecksumFromNamePtr m_get_map_checksum_from_name;

	GetPrimaryModArchivePtr m_get_primary_mod_archive;
	GetPrimaryModArchiveCountPtr m_get_primary_mod_archive_count;
	GetPrimaryModArchiveListPtr m_get_primary_mod_archive_list;
	GetPrimaryModChecksumFromNamePtr m_get_primary_mod_checksum_from_name;
	GetModValidMapCountPtr m_get_mod_valid_map_count;
	GetModValidMapPtr m_get_valid_map;

	GetMapOptionCountPtr m_get_map_option_count;
	GetCustomOptionCountPtr m_get_custom_option_count;
	GetModOptionCountPtr m_get_mod_option_count;
	GetSkirmishAIOptionCountPtr m_get_skirmish_ai_option_count;
	GetOptionKeyPtr m_get_option_key;
	GetOptionNamePtr m_get_option_name;
	GetOptionDescPtr m_get_option_desc;
	GetOptionTypePtr m_get_option_type;
	GetOptionSectionPtr m_get_option_section;
	GetOptionBoolDefPtr m_get_option_bool_def;
	GetOptionNumberDefPtr m_get_option_number_def;
	GetOptionNumberMinPtr m_get_option_number_min;
	GetOptionNumberMaxPtr m_get_option_number_max;
	GetOptionNumberStepPtr m_get_option_number_step;
	GetOptionStringDefPtr m_get_option_string_def;
	GetOptionStringMaxLenPtr m_get_option_string_max_len;
	GetOptionListCountPtr m_get_option_list_count;
	GetOptionListDefPtr m_get_option_list_def;
	GetOptionListItemKeyPtr m_get_option_list_item_key;
	GetOptionListItemNamePtr m_get_option_list_item_name;
	GetOptionListItemDescPtr m_get_option_list_item_desc;

	OpenArchivePtr m_open_archive;
	CloseArchivePtr m_close_archive;
	FindFilesArchivePtr m_find_Files_archive;
	OpenArchiveFilePtr m_open_archive_file;
	ReadArchiveFilePtr m_read_archive_file;
	CloseArchiveFilePtr m_close_archive_file;
	SizeArchiveFilePtr m_size_archive_file;

	SetSpringConfigFilePtr m_set_spring_config_file_path;
	GetSpringConfigFilePtr m_get_spring_config_file_path;
	SetSpringConfigFloatPtr m_set_spring_config_float;
	GetSpringConfigFloatPtr m_get_spring_config_float;
	GetSpringConfigIntPtr m_get_spring_config_int;
	GetSpringConfigStringPtr m_get_spring_config_string;
	SetSpringConfigStringPtr m_set_spring_config_string;
	SetSpringConfigIntPtr m_set_spring_config_int;
	DeleteSpringConfigKeyPtr m_delete_spring_config_key;

	GetSkirmishAICountPtr m_get_skirmish_ai_count;
	GetSkirmishAIInfoCountPtr m_get_skirmish_ai_info_count;
	GetInfoKeyPtr m_get_info_key;
	GetInfoTypePtr m_get_info_type;
	GetInfoDescriptionPtr m_get_description;
	GetInfoValueStringPtr m_get_info_value_string;
	GetInfoValueIntegerPtr m_get_info_value_integer;
	GetInfoValueFloatPtr m_get_info_value_float;
	GetInfoValueBoolPtr m_get_info_value_bool;
	GetPrimaryModInfoCountPtr m_get_primary_mod_info_count;

	// lua parser section

	lpClosePtr m_parser_close;
	lpOpenFilePtr m_parser_open_file;
	lpOpenSourcePtr m_parser_open_source;
	lpExecutePtr m_parser_execute;
	lpErrorLogPtr m_parser_error_log;

	lpAddTableIntPtr m_parser_add_table_int;
	lpAddTableStrPtr m_parser_add_table_string;
	lpEndTablePtr m_parser_end_table;
	lpAddIntKeyIntValPtr m_parser_add_int_key_int_value;
	lpAddStrKeyIntValPtr m_parser_add_string_key_int_value;
	lpAddIntKeyBoolValPtr m_parser_add_int_key_bool_value;
	lpAddStrKeyBoolValPtr m_parser_add_string_key_bool_value;
	lpAddIntKeyFloatValPtr m_parser_add_int_key_float_value;
	lpAddStrKeyFloatValPtr m_parser_add_string_key_float_value;
	lpAddIntKeyStrValPtr m_parser_add_int_key_string_value;
	lpAddStrKeyStrValPtr m_parser_add_string_key_string_value;

	lpRootTablePtr m_parser_root_table;
	lpRootTableExprPtr m_parser_root_table_expression;
	lpSubTableIntPtr m_parser_sub_table_int;
	lpSubTableStrPtr m_parser_sub_table_string;
	lpSubTableExprPtr m_parser_sub_table_expression;
	lpPopTablePtr m_parser_pop_table;

	lpGetKeyExistsIntPtr m_parser_key_int_exists;
	lpGetKeyExistsStrPtr m_parser_key_string_exists;

	lpGetIntKeyTypePtr m_parser_int_key_get_type;
	lpGetStrKeyTypePtr m_parser_string_key_get_type;

	lpGetIntKeyListCountPtr m_parser_int_key_get_list_count;
	lpGetIntKeyListEntryPtr m_parser_int_key_get_list_entry;
	lpGetStrKeyListCountPtr m_parser_string_key_get_list_count;
	lpGetStrKeyListEntryPtr m_parser_string_key_get_list_entry;

	lpGetIntKeyIntValPtr m_parser_int_key_get_int_value;
	lpGetStrKeyIntValPtr m_parser_string_key_get_int_value;
	lpGetIntKeyBoolValPtr m_parser_int_key_get_bool_value;
	lpGetStrKeyBoolValPtr m_parser_string_key_get_bool_value;
	lpGetIntKeyFloatValPtr m_parser_int_key_get_float_value;
	lpGetStrKeyFloatValPtr m_parser_string_key_get_float_value;
	lpGetIntKeyStrValPtr m_parser_int_key_get_string_value;
	lpGetStrKeyStrValPtr m_parser_string_key_get_string_value;

	///@}
};

UnitsyncLib& susynclib();

} // namespace LSL

#endif //LIBSPRINGLOBBY_HEADERGUARD_SPRINGUNITSYNCLIB_H
