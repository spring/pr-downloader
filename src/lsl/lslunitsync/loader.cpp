/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "loader.h"

#include <string>

#include "c_api.h"
#include "sharedlib.h"
#include "lslutils/logging.h"

namespace LSL
{

#define BIND(type, name, var)                               \
	s->var = (type)GetLibFuncPtr(s->m_libhandle, name); \
	if (s->var == nullptr)                              \
		return false;

#define BIND_OPTIONAL(type, name, var) \
	s->var = (type)GetLibFuncPtr(s->m_libhandle, name);

bool UnitsyncFunctionLoader::BindFunctions(UnitsyncLib* s)
{

	// LUA
	BIND(lpClosePtr, "lpClose", m_parser_close);
	BIND(lpOpenFilePtr, "lpOpenFile", m_parser_open_file);
	BIND(lpOpenSourcePtr, "lpOpenSource", m_parser_open_source);
	BIND(lpExecutePtr, "lpExecute", m_parser_execute);
	BIND(lpErrorLogPtr, "lpErrorLog", m_parser_error_log);

	BIND(lpAddTableIntPtr, "lpAddTableInt", m_parser_add_table_int);
	BIND(lpAddTableStrPtr, "lpAddTableStr", m_parser_add_table_string);
	BIND(lpEndTablePtr, "lpEndTable", m_parser_end_table);
	BIND(lpAddIntKeyIntValPtr, "lpAddIntKeyIntVal", m_parser_add_int_key_int_value);
	BIND(lpAddStrKeyIntValPtr, "lpAddStrKeyIntVal", m_parser_add_string_key_int_value);
	BIND(lpAddIntKeyBoolValPtr, "lpAddIntKeyBoolVal", m_parser_add_int_key_bool_value);
	BIND(lpAddStrKeyBoolValPtr, "lpAddStrKeyBoolVal", m_parser_add_string_key_bool_value);
	BIND(lpAddIntKeyFloatValPtr, "lpAddIntKeyFloatVal", m_parser_add_int_key_float_value);
	BIND(lpAddStrKeyFloatValPtr, "lpAddStrKeyFloatVal", m_parser_add_string_key_float_value);
	BIND(lpAddIntKeyStrValPtr, "lpAddIntKeyStrVal", m_parser_add_int_key_string_value);
	BIND(lpAddStrKeyStrValPtr, "lpAddStrKeyStrVal", m_parser_add_string_key_string_value);

	BIND(lpRootTablePtr, "lpRootTable", m_parser_root_table);
	BIND(lpRootTableExprPtr, "lpRootTableExpr", m_parser_root_table_expression);
	BIND(lpSubTableIntPtr, "lpSubTableInt", m_parser_sub_table_int);
	BIND(lpSubTableStrPtr, "lpSubTableStr", m_parser_sub_table_string);
	BIND(lpSubTableExprPtr, "lpSubTableExpr", m_parser_sub_table_expression);
	BIND(lpPopTablePtr, "lpPopTable", m_parser_pop_table);

	BIND(lpGetKeyExistsIntPtr, "lpGetKeyExistsInt", m_parser_key_int_exists);
	BIND(lpGetKeyExistsStrPtr, "lpGetKeyExistsStr", m_parser_key_string_exists);

	BIND(lpGetIntKeyTypePtr, "lpGetIntKeyType", m_parser_int_key_get_type);
	BIND(lpGetStrKeyTypePtr, "lpGetStrKeyType", m_parser_string_key_get_type);

	BIND(lpGetIntKeyListCountPtr, "lpGetIntKeyListCount", m_parser_int_key_get_list_count);
	BIND(lpGetIntKeyListEntryPtr, "lpGetIntKeyListEntry", m_parser_int_key_get_list_entry);
	BIND(lpGetStrKeyListCountPtr, "lpGetStrKeyListCount", m_parser_string_key_get_list_count);
	BIND(lpGetStrKeyListEntryPtr, "lpGetStrKeyListEntry", m_parser_string_key_get_list_entry);

	BIND(lpGetIntKeyIntValPtr, "lpGetIntKeyIntVal", m_parser_int_key_get_int_value);
	BIND(lpGetStrKeyIntValPtr, "lpGetStrKeyIntVal", m_parser_string_key_get_int_value);
	BIND(lpGetIntKeyBoolValPtr, "lpGetIntKeyBoolVal", m_parser_int_key_get_bool_value);
	BIND(lpGetStrKeyBoolValPtr, "lpGetStrKeyBoolVal", m_parser_string_key_get_bool_value);
	BIND(lpGetIntKeyFloatValPtr, "lpGetIntKeyFloatVal", m_parser_int_key_get_float_value);
	BIND(lpGetStrKeyFloatValPtr, "lpGetStrKeyFloatVal", m_parser_string_key_get_float_value);
	BIND(lpGetIntKeyStrValPtr, "lpGetIntKeyStrVal", m_parser_int_key_get_string_value);
	BIND(lpGetStrKeyStrValPtr, "lpGetStrKeyStrVal", m_parser_string_key_get_string_value);

	// MMOptions
	BIND(GetMapOptionCountPtr, "GetMapOptionCount", m_get_map_option_count);
	BIND(GetCustomOptionCountPtr, "GetCustomOptionCount", m_get_custom_option_count);
	BIND(GetModOptionCountPtr, "GetModOptionCount", m_get_mod_option_count);
	BIND(GetSkirmishAIOptionCountPtr, "GetSkirmishAIOptionCount", m_get_skirmish_ai_option_count);
	BIND(GetOptionKeyPtr, "GetOptionKey", m_get_option_key);
	BIND(GetOptionNamePtr, "GetOptionName", m_get_option_name);
	BIND(GetOptionDescPtr, "GetOptionDesc", m_get_option_desc);
	BIND(GetOptionTypePtr, "GetOptionType", m_get_option_type);
	BIND(GetOptionSectionPtr, "GetOptionSection", m_get_option_section);
	BIND(GetOptionBoolDefPtr, "GetOptionBoolDef", m_get_option_bool_def);
	BIND(GetOptionNumberDefPtr, "GetOptionNumberDef", m_get_option_number_def);
	BIND(GetOptionNumberMinPtr, "GetOptionNumberMin", m_get_option_number_min);
	BIND(GetOptionNumberMaxPtr, "GetOptionNumberMax", m_get_option_number_max);
	BIND(GetOptionNumberStepPtr, "GetOptionNumberStep", m_get_option_number_step);
	BIND(GetOptionStringDefPtr, "GetOptionStringDef", m_get_option_string_def);
	BIND(GetOptionStringMaxLenPtr, "GetOptionStringMaxLen", m_get_option_string_max_len);
	BIND(GetOptionListCountPtr, "GetOptionListCount", m_get_option_list_count);
	BIND(GetOptionListDefPtr, "GetOptionListDef", m_get_option_list_def);
	BIND(GetOptionListItemKeyPtr, "GetOptionListItemKey", m_get_option_list_item_key);
	BIND(GetOptionListItemNamePtr, "GetOptionListItemName", m_get_option_list_item_name);
	BIND(GetOptionListItemDescPtr, "GetOptionListItemDesc", m_get_option_list_item_desc);

	//MAP
	bool oldstyle = false;
	BIND_OPTIONAL(GetMapInfoCountPtr, "GetMapInfoCount", m_get_map_info_count);
	if (s->m_get_map_info_count == nullptr) {
		oldstyle = true;
		LslDebug("Using old style map-info fetching (GetMap*()).");
	}
	BIND(GetMapCountPtr, "GetMapCount", m_get_map_count);
	BIND(GetMapChecksumPtr, "GetMapChecksum", m_get_map_checksum);
	BIND(GetMapNamePtr, "GetMapName", m_get_map_name);

	if (oldstyle) {
		BIND(GetMapDescriptionPtr, "GetMapDescription", m_get_map_description);
		BIND(GetMapAuthorPtr, "GetMapAuthor", m_get_map_author);
		BIND(GetMapWidthPtr, "GetMapWidth", m_get_map_width);
		BIND(GetMapHeightPtr, "GetMapHeight", m_get_map_height);
		BIND(GetMapTidalStrengthPtr, "GetMapTidalStrength", m_get_map_tidalStrength);
		BIND(GetMapWindMinPtr, "GetMapWindMin", m_get_map_windMin);
		BIND(GetMapWindMaxPtr, "GetMapWindMax", m_get_map_windMax);
		BIND(GetMapGravityPtr, "GetMapGravity", m_get_map_gravity);
		BIND(GetMapResourceCountPtr, "GetMapResourceCount", m_get_map_resource_count);
		BIND(GetMapResourceNamePtr, "GetMapResourceName", m_get_map_resource_name);
		BIND(GetMapResourceMaxPtr, "GetMapResourceMax", m_get_map_resource_max);
		BIND(GetMapResourceExtractorRadiusPtr, "GetMapResourceExtractorRadius", m_get_map_resource_extractorRadius);
		BIND(GetMapPosCountPtr, "GetMapPosCount", m_get_map_pos_count);
		BIND(GetMapPosXPtr, "GetMapPosX", m_get_map_pos_x);
		BIND(GetMapPosZPtr, "GetMapPosZ", m_get_map_pos_z);
	}

	BIND(GetMinimapPtr, "GetMinimap", m_get_minimap);
	BIND(GetInfoMapSizePtr, "GetInfoMapSize", m_get_infomap_size);
	BIND(GetInfoMapPtr, "GetInfoMap", m_get_infomap);

	BIND(GetMapArchiveCountPtr, "GetMapArchiveCount", m_get_map_archive_count);
	BIND(GetMapArchiveNamePtr, "GetMapArchiveName", m_get_map_archive_name);
	BIND(GetMapChecksumPtr, "GetMapChecksum", m_get_map_checksum);
	BIND(GetMapChecksumFromNamePtr, "GetMapChecksumFromName", m_get_map_checksum_from_name);

	//Basic stuff
	BIND_OPTIONAL(GetSysHashPtr, "GetSysInfoHash", m_sys_hash);
	BIND_OPTIONAL(GetMacHashPtr, "GetMacAddrHash", m_mac_hash);

	BIND(InitPtr, "Init", m_init);
	BIND(UnInitPtr, "UnInit", m_uninit);
	BIND(GetNextErrorPtr, "GetNextError", m_get_next_error);
	BIND(GetWritableDataDirectoryPtr, "GetWritableDataDirectory", m_get_writeable_data_dir);
	BIND(GetDataDirectoryPtr, "GetDataDirectory", m_get_data_dir_by_index);
	BIND(GetDataDirectoryCountPtr, "GetDataDirectoryCount", m_get_data_dir_count);
	BIND(GetSpringVersionPtr, "GetSpringVersion", m_get_spring_version);
	BIND(GetSpringVersionPatchsetPtr, "GetSpringVersionPatchset", m_get_spring_version_patchset);
	BIND(IsSpringReleaseVersionPtr, "IsSpringReleaseVersion", m_is_spring_release_version);

	BIND(AddAllArchivesPtr, "AddAllArchives", m_add_all_archives);
	BIND(RemoveAllArchivesPtr, "RemoveAllArchives", m_remove_all_archives);

	BIND(InitFindVFSPtr, "InitFindVFS", m_init_find_vfs);
	BIND(FindFilesVFSPtr, "FindFilesVFS", m_find_files_vfs);
	BIND(OpenFileVFSPtr, "OpenFileVFS", m_open_file_vfs);
	BIND(FileSizeVFSPtr, "FileSizeVFS", m_file_size_vfs);
	BIND(ReadFileVFSPtr, "ReadFileVFS", m_read_file_vfs);
	BIND(CloseFileVFSPtr, "CloseFileVFS", m_close_file_vfs);


	BIND(ProcessUnitsPtr, "ProcessUnits", m_process_units);
	BIND(AddArchivePtr, "AddArchive", m_add_archive);
	BIND(GetArchiveChecksumPtr, "GetArchiveChecksum", m_get_archive_checksum);
	BIND(GetArchivePathPtr, "GetArchivePath", m_get_archive_path);

	BIND(OpenArchivePtr, "OpenArchive", m_open_archive);
	BIND(CloseArchivePtr, "CloseArchive", m_close_archive);
	BIND(FindFilesArchivePtr, "FindFilesArchive", m_find_Files_archive);
	BIND(OpenArchiveFilePtr, "OpenArchiveFile", m_open_archive_file);
	BIND(ReadArchiveFilePtr, "ReadArchiveFile", m_read_archive_file);
	BIND(CloseArchiveFilePtr, "CloseArchiveFile", m_close_archive_file);
	BIND(SizeArchiveFilePtr, "SizeArchiveFile", m_size_archive_file);

	BIND(GetSkirmishAICountPtr, "GetSkirmishAICount", m_get_skirmish_ai_count);
	BIND(GetSkirmishAIInfoCountPtr, "GetSkirmishAIInfoCount", m_get_skirmish_ai_info_count);

	BIND(GetInfoKeyPtr, "GetInfoKey", m_get_info_key);
	BIND(GetInfoDescriptionPtr, "GetInfoDescription", m_get_description);

	BIND(GetInfoTypePtr, "GetInfoType", m_get_info_type);
	BIND(GetInfoValueStringPtr, "GetInfoValueString", m_get_info_value_string);
	BIND(GetInfoValueFloatPtr, "GetInfoValueFloat", m_get_info_value_float);
	BIND(GetInfoValueBoolPtr, "GetInfoValueBool", m_get_info_value_bool);
	BIND(GetInfoValueIntegerPtr, "GetInfoValueInteger", m_get_info_value_integer);

	// Config
	BIND(SetSpringConfigFilePtr, "SetSpringConfigFile", m_set_spring_config_file_path);
	BIND(GetSpringConfigFilePtr, "GetSpringConfigFile", m_get_spring_config_file_path);

	BIND(SetSpringConfigFloatPtr, "SetSpringConfigFloat", m_set_spring_config_float);
	BIND(GetSpringConfigFloatPtr, "GetSpringConfigFloat", m_get_spring_config_float);
	BIND(GetSpringConfigIntPtr, "GetSpringConfigInt", m_get_spring_config_int);
	BIND(GetSpringConfigStringPtr, "GetSpringConfigString", m_get_spring_config_string);
	BIND(SetSpringConfigStringPtr, "SetSpringConfigString", m_set_spring_config_string);
	BIND(SetSpringConfigIntPtr, "SetSpringConfigInt", m_set_spring_config_int);
	BIND(DeleteSpringConfigKeyPtr, "DeleteSpringConfigKey", m_delete_spring_config_key);

	// Game
	BIND(GetPrimaryModIndexPtr, "GetPrimaryModIndex", m_get_mod_index);
	BIND(GetPrimaryModCountPtr, "GetPrimaryModCount", m_get_mod_count);
	BIND(GetPrimaryModArchivePtr, "GetPrimaryModArchive", m_get_mod_archive);
	BIND(GetPrimaryModInfoCountPtr, "GetPrimaryModInfoCount", m_get_primary_mod_info_count);

	BIND(GetSideCountPtr, "GetSideCount", m_get_side_count);
	BIND(GetSideNamePtr, "GetSideName", m_get_side_name);

	BIND(GetPrimaryModArchivePtr, "GetPrimaryModArchive", m_get_primary_mod_archive);
	BIND(GetPrimaryModArchiveCountPtr, "GetPrimaryModArchiveCount", m_get_primary_mod_archive_count);
	BIND(GetPrimaryModArchiveListPtr, "GetPrimaryModArchiveList", m_get_primary_mod_archive_list);
	BIND(GetPrimaryModChecksumFromNamePtr, "GetPrimaryModChecksumFromName", m_get_primary_mod_checksum_from_name);

	BIND(GetModValidMapCountPtr, "GetModValidMapCount", m_get_mod_valid_map_count);
	BIND(GetModValidMapPtr, "GetModValidMap", m_get_valid_map);

	BIND(GetUnitCountPtr, "GetUnitCount", m_get_unit_count);
	BIND(GetUnitNamePtr, "GetUnitName", m_get_unit_name);
	BIND(GetFullUnitNamePtr, "GetFullUnitName", m_get_unit_full_name);

	return true;
}

void UnitsyncFunctionLoader::UnbindFunctions(UnitsyncLib* s)
{
	// LUA
	s->m_parser_close = nullptr;
	s->m_parser_open_file = nullptr;
	s->m_parser_open_source = nullptr;
	s->m_parser_execute = nullptr;
	s->m_parser_error_log = nullptr;

	s->m_parser_add_table_int = nullptr;
	s->m_parser_add_table_string = nullptr;
	s->m_parser_end_table = nullptr;
	s->m_parser_add_int_key_int_value = nullptr;
	s->m_parser_add_string_key_int_value = nullptr;
	s->m_parser_add_int_key_bool_value = nullptr;
	s->m_parser_add_string_key_bool_value = nullptr;
	s->m_parser_add_int_key_float_value = nullptr;
	s->m_parser_add_string_key_float_value = nullptr;
	s->m_parser_add_int_key_string_value = nullptr;
	s->m_parser_add_string_key_string_value = nullptr;

	s->m_parser_root_table = nullptr;
	s->m_parser_root_table_expression = nullptr;
	s->m_parser_sub_table_int = nullptr;
	s->m_parser_sub_table_string = nullptr;
	s->m_parser_sub_table_expression = nullptr;
	s->m_parser_pop_table = nullptr;

	s->m_parser_key_int_exists = nullptr;
	s->m_parser_key_string_exists = nullptr;

	s->m_parser_int_key_get_type = nullptr;
	s->m_parser_string_key_get_type = nullptr;

	s->m_parser_int_key_get_list_count = nullptr;
	s->m_parser_int_key_get_list_entry = nullptr;
	s->m_parser_string_key_get_list_count = nullptr;
	s->m_parser_string_key_get_list_entry = nullptr;

	s->m_parser_int_key_get_int_value = nullptr;
	s->m_parser_string_key_get_int_value = nullptr;
	s->m_parser_int_key_get_bool_value = nullptr;
	s->m_parser_string_key_get_bool_value = nullptr;
	s->m_parser_int_key_get_float_value = nullptr;
	s->m_parser_string_key_get_float_value = nullptr;
	s->m_parser_int_key_get_string_value = nullptr;
	s->m_parser_string_key_get_string_value = nullptr;

	// MMOptions
	s->m_get_map_option_count = nullptr;
	s->m_get_custom_option_count = nullptr;
	s->m_get_mod_option_count = nullptr;
	s->m_get_skirmish_ai_option_count = nullptr;
	s->m_get_option_key = nullptr;
	s->m_get_option_name = nullptr;
	s->m_get_option_desc = nullptr;
	s->m_get_option_type = nullptr;
	s->m_get_option_section = nullptr;
	s->m_get_option_bool_def = nullptr;
	s->m_get_option_number_def = nullptr;
	s->m_get_option_number_min = nullptr;
	s->m_get_option_number_max = nullptr;
	s->m_get_option_number_step = nullptr;
	s->m_get_option_string_def = nullptr;
	s->m_get_option_string_max_len = nullptr;
	s->m_get_option_list_count = nullptr;
	s->m_get_option_list_def = nullptr;
	s->m_get_option_list_item_key = nullptr;
	s->m_get_option_list_item_name = nullptr;
	s->m_get_option_list_item_desc = nullptr;

	//MAP
	s->m_get_map_info_count = nullptr;

	s->m_get_map_count = nullptr;
	s->m_get_map_checksum = nullptr;
	s->m_get_map_name = nullptr;

	s->m_get_map_description = nullptr;
	s->m_get_map_author = nullptr;
	s->m_get_map_width = nullptr;
	s->m_get_map_height = nullptr;
	s->m_get_map_tidalStrength = nullptr;
	s->m_get_map_windMin = nullptr;
	s->m_get_map_windMax = nullptr;
	s->m_get_map_gravity = nullptr;
	s->m_get_map_resource_count = nullptr;
	s->m_get_map_resource_name = nullptr;
	s->m_get_map_resource_max = nullptr;
	s->m_get_map_resource_extractorRadius = nullptr;
	s->m_get_map_pos_count = nullptr;
	s->m_get_map_pos_x = nullptr;
	s->m_get_map_pos_z = nullptr;

	s->m_get_minimap = nullptr;
	s->m_get_infomap_size = nullptr;
	s->m_get_infomap = nullptr;

	s->m_get_map_archive_count = nullptr;
	s->m_get_map_archive_name = nullptr;
	s->m_get_map_checksum = nullptr;
	s->m_get_map_checksum_from_name = nullptr;

	//Basic stuff
	s->m_sys_hash = nullptr;
	s->m_mac_hash = nullptr;

	s->m_init = nullptr;
	s->m_uninit = nullptr;
	s->m_get_next_error = nullptr;
	s->m_get_writeable_data_dir = nullptr;
	s->m_get_data_dir_by_index = nullptr;
	s->m_get_data_dir_count = nullptr;
	s->m_get_spring_version = nullptr;
	s->m_get_spring_version_patchset = nullptr;
	s->m_is_spring_release_version = nullptr;

	s->m_add_all_archives = nullptr;
	s->m_remove_all_archives = nullptr;

	s->m_init_find_vfs = nullptr;
	s->m_find_files_vfs = nullptr;
	s->m_open_file_vfs = nullptr;
	s->m_file_size_vfs = nullptr;
	s->m_read_file_vfs = nullptr;
	s->m_close_file_vfs = nullptr;


	s->m_process_units = nullptr;
	s->m_add_archive = nullptr;
	s->m_get_archive_checksum = nullptr;
	s->m_get_archive_path = nullptr;

	s->m_open_archive = nullptr;
	s->m_close_archive = nullptr;
	s->m_find_Files_archive = nullptr;
	s->m_open_archive_file = nullptr;
	s->m_read_archive_file = nullptr;
	s->m_close_archive_file = nullptr;
	s->m_size_archive_file = nullptr;

	s->m_get_skirmish_ai_count = nullptr;
	s->m_get_skirmish_ai_info_count = nullptr;

	s->m_get_info_key = nullptr;
	s->m_get_description = nullptr;

	s->m_get_info_type = nullptr;
	s->m_get_info_value_string = nullptr;
	s->m_get_info_value_float = nullptr;
	s->m_get_info_value_bool = nullptr;
	s->m_get_info_value_integer = nullptr;

	// Config
	s->m_set_spring_config_file_path = nullptr;
	s->m_get_spring_config_file_path = nullptr;

	s->m_set_spring_config_float = nullptr;
	s->m_get_spring_config_float = nullptr;
	s->m_get_spring_config_int = nullptr;
	s->m_get_spring_config_string = nullptr;
	s->m_set_spring_config_string = nullptr;
	s->m_set_spring_config_int = nullptr;
	s->m_delete_spring_config_key = nullptr;

	// Game
	s->m_get_mod_index = nullptr;
	s->m_get_mod_count = nullptr;
	s->m_get_mod_archive = nullptr;
	s->m_get_primary_mod_info_count = nullptr;

	s->m_get_side_count = nullptr;
	s->m_get_side_name = nullptr;

	s->m_get_primary_mod_archive = nullptr;
	s->m_get_primary_mod_archive_count = nullptr;
	s->m_get_primary_mod_archive_list = nullptr;
	s->m_get_primary_mod_checksum_from_name = nullptr;

	s->m_get_mod_valid_map_count = nullptr;
	s->m_get_valid_map = nullptr;

	s->m_get_unit_count = nullptr;
	s->m_get_unit_name = nullptr;
	s->m_get_unit_full_name = nullptr;
}

} // namespace LSL
