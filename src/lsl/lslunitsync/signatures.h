#ifndef LSL_SIGNATURES_H
#define LSL_SIGNATURES_H

namespace LSL
{

struct SpringMapInfo;

/** @{
, \defgroup DllPointerTypes Pointer types used with the unitsync library.
, \TODO move from global namespace
,/
*/


#ifdef _WIN32
#define USYNC_CALLCONV __stdcall
#else
#define USYNC_CALLCONV
#endif

#define FUNC(ret, name, ...) \
	typedef ret(USYNC_CALLCONV* name)(__VA_ARGS__)

FUNC(const char*, GetSpringVersionPtr);
FUNC(const char*, GetSpringVersionPatchsetPtr);
FUNC(bool, IsSpringReleaseVersionPtr);

FUNC(int, InitPtr, bool, int);
FUNC(void, UnInitPtr);
FUNC(const char*, GetNextErrorPtr);
FUNC(const char*, GetWritableDataDirectoryPtr);
FUNC(const char*, GetDataDirectoryPtr, int);
FUNC(int, GetDataDirectoryCountPtr);

FUNC(int, GetMapCountPtr);
FUNC(unsigned int, GetMapChecksumPtr, int);
FUNC(const char*, GetMapNamePtr, int);
FUNC(const char*, GetMapDescriptionPtr, int);
FUNC(const char*, GetMapAuthorPtr, int);
FUNC(int, GetMapWidthPtr, int);
FUNC(int, GetMapHeightPtr, int);
FUNC(int, GetMapTidalStrengthPtr, int);
FUNC(int, GetMapWindMinPtr, int);
FUNC(int, GetMapWindMaxPtr, int);
FUNC(int, GetMapGravityPtr, int);
FUNC(int, GetMapResourceCountPtr, int);
FUNC(const char*, GetMapResourceNamePtr, int, int);
FUNC(float, GetMapResourceMaxPtr, int, int);
FUNC(int, GetMapResourceExtractorRadiusPtr, int, int);
FUNC(int, GetMapPosCountPtr, int);
FUNC(float, GetMapPosXPtr, int, int);
FUNC(float, GetMapPosZPtr, int, int);

FUNC(int, GetMapInfoExPtr, const char*, SpringMapInfo*, int);
FUNC(void*, GetMinimapPtr, const char*, int);
FUNC(int, GetInfoMapSizePtr, const char*, const char*, int*, int*);
FUNC(int, GetInfoMapPtr, const char*, const char*, void*, int);

FUNC(unsigned int, GetPrimaryModChecksumPtr, int);
FUNC(int, GetPrimaryModIndexPtr, const char*);
FUNC(const char*, GetPrimaryModNamePtr, int);
FUNC(int, GetPrimaryModCountPtr);
FUNC(const char*, GetPrimaryModArchivePtr, int);

FUNC(int, GetSideCountPtr);
FUNC(const char*, GetSideNamePtr, int);

FUNC(void, AddAllArchivesPtr, const char*);
FUNC(void, RemoveAllArchivesPtr);

FUNC(const char*, GetFullUnitNamePtr, int);
FUNC(const char*, GetUnitNamePtr, int);
FUNC(int, GetUnitCountPtr);
FUNC(int, ProcessUnitsNoChecksumPtr);

FUNC(int, InitFindVFSPtr, const char*);
FUNC(int, FindFilesVFSPtr, int, char*, int);
FUNC(int, OpenFileVFSPtr, const char*);
FUNC(int, FileSizeVFSPtr, int);
FUNC(int, ReadFileVFSPtr, int, void*, int);
FUNC(void, CloseFileVFSPtr, int);

FUNC(void, SetSpringConfigFilePtr, const char*);
FUNC(const char*, GetSpringConfigFilePtr);

FUNC(int, GetSpringConfigIntPtr, const char*, int);
FUNC(const char*, GetSpringConfigStringPtr, const char*, const char*);
FUNC(float, GetSpringConfigFloatPtr, const char*, float);

FUNC(void, SetSpringConfigStringPtr, const char*, const char*);
FUNC(void, SetSpringConfigIntPtr, const char*, int);
FUNC(void, SetSpringConfigFloatPtr, const char*, float);
FUNC(void, DeleteSpringConfigKeyPtr, const char*);

FUNC(int, ProcessUnitsPtr, void);
FUNC(void, AddArchivePtr, const char*);
FUNC(unsigned int, GetArchiveChecksumPtr, const char*);
FUNC(const char*, GetArchivePathPtr, const char*);
FUNC(int, GetMapArchiveCountPtr, const char*);
FUNC(const char*, GetMapArchiveNamePtr, int);
FUNC(unsigned int, GetMapChecksumPtr, int);
FUNC(int, GetMapChecksumFromNamePtr, const char*);

FUNC(const char*, GetPrimaryModShortNamePtr, int);
FUNC(const char*, GetPrimaryModVersionPtr, int);
FUNC(const char*, GetPrimaryModMutatorPtr, int);
FUNC(const char*, GetPrimaryModGamePtr, int);
FUNC(const char*, GetPrimaryModShortGamePtr, int);
FUNC(const char*, GetPrimaryModDescriptionPtr, int);
FUNC(const char*, GetPrimaryModArchivePtr, int);
FUNC(int, GetPrimaryModArchiveCountPtr, int);
FUNC(const char*, GetPrimaryModArchiveListPtr, int);
FUNC(unsigned int, GetPrimaryModChecksumFromNamePtr, const char*);
FUNC(unsigned int, GetModValidMapCountPtr);
FUNC(const char*, GetModValidMapPtr, int);

FUNC(int, GetLuaAICountPtr);
FUNC(const char*, GetLuaAINamePtr, int);
FUNC(const char*, GetLuaAIDescPtr, int);

FUNC(int, GetMapOptionCountPtr, const char*);
FUNC(int, GetCustomOptionCountPtr, const char*);
FUNC(int, GetModOptionCountPtr);
FUNC(int, GetSkirmishAIOptionCountPtr, int);
FUNC(const char*, GetOptionKeyPtr, int);
FUNC(const char*, GetOptionNamePtr, int);
FUNC(const char*, GetOptionDescPtr, int);
FUNC(const char*, GetOptionSectionPtr, int);
FUNC(const char*, GetOptionStylePtr, int);
FUNC(int, GetOptionTypePtr, int);
FUNC(int, GetOptionBoolDefPtr, int);
FUNC(float, GetOptionNumberDefPtr, int);
FUNC(float, GetOptionNumberMinPtr, int);
FUNC(float, GetOptionNumberMaxPtr, int);
FUNC(float, GetOptionNumberStepPtr, int);
FUNC(const char*, GetOptionStringDefPtr, int);
FUNC(int, GetOptionStringMaxLenPtr, int);
FUNC(int, GetOptionListCountPtr, int);
FUNC(const char*, GetOptionListDefPtr, int);
FUNC(const char*, GetOptionListItemKeyPtr, int, int);
FUNC(const char*, GetOptionListItemNamePtr, int, int);
FUNC(const char*, GetOptionListItemDescPtr, int, int);

FUNC(int, OpenArchivePtr, const char*);
FUNC(void, CloseArchivePtr, int);
FUNC(int, FindFilesArchivePtr, int, int, char*, int*);
FUNC(int, OpenArchiveFilePtr, int, const char*);
FUNC(int, ReadArchiveFilePtr, int, int, void*, int);
FUNC(void, CloseArchiveFilePtr, int, int);
FUNC(int, SizeArchiveFilePtr, int, int);

FUNC(int, GetSkirmishAICountPtr);
FUNC(int, GetSkirmishAIInfoCountPtr, int);
FUNC(const char*, GetInfoKeyPtr, int);
FUNC(const char*, GetInfoValuePtr, int);
FUNC(const char*, GetInfoDescriptionPtr, int);
FUNC(const char*, GetInfoType, int);

FUNC(const char*, GetSysHashPtr);
FUNC(const char*, GetMacHashPtr);

/// Unitsync functions wrapping lua parser
/** @} */
FUNC(void, lpClosePtr);
FUNC(int, lpOpenFilePtr, const char*, const char*, const char*);
FUNC(int, lpOpenSourcePtr, const char*, const char*);
FUNC(int, lpExecutePtr);
FUNC(const char*, lpErrorLogPtr);

FUNC(void, lpAddTableIntPtr, int, int override);
FUNC(void, lpAddTableStrPtr, const char*, int override);
FUNC(void, lpEndTablePtr);
FUNC(void, lpAddIntKeyIntValPtr, int, int);
FUNC(void, lpAddStrKeyIntValPtr, const char*, int);
FUNC(void, lpAddIntKeyBoolValPtr, int, int);
FUNC(void, lpAddStrKeyBoolValPtr, const char*, int);
FUNC(void, lpAddIntKeyFloatValPtr, int, float val);
FUNC(void, lpAddStrKeyFloatValPtr, const char*, float val);
FUNC(void, lpAddIntKeyStrValPtr, int, const char* val);
FUNC(void, lpAddStrKeyStrValPtr, const char*, const char* val);

FUNC(int, lpRootTablePtr);
FUNC(int, lpRootTableExprPtr, const char* expr);
FUNC(int, lpSubTableIntPtr, int);
FUNC(int, lpSubTableStrPtr, const char*);
FUNC(int, lpSubTableExprPtr, const char* expr);
FUNC(void, lpPopTablePtr);

FUNC(int, lpGetKeyExistsIntPtr, int);
FUNC(int, lpGetKeyExistsStrPtr, const char*);

FUNC(int, lpGetIntKeyTypePtr, int);
FUNC(int, lpGetStrKeyTypePtr, const char*);

FUNC(int, lpGetIntKeyListCountPtr);
FUNC(int, lpGetIntKeyListEntryPtr, int);
FUNC(int, lpGetStrKeyListCountPtr);
FUNC(const char*, lpGetStrKeyListEntryPtr, int);

FUNC(int, lpGetIntKeyIntValPtr, int, int);
FUNC(int, lpGetStrKeyIntValPtr, const char*, int);
FUNC(int, lpGetIntKeyBoolValPtr, int, int);
FUNC(int, lpGetStrKeyBoolValPtr, const char*, int);
FUNC(float, lpGetIntKeyFloatValPtr, int, float);
FUNC(float, lpGetStrKeyFloatValPtr, const char*, float);
FUNC(const char*, lpGetIntKeyStrValPtr, int, const char*);
FUNC(const char*, lpGetStrKeyStrValPtr, const char*, const char*);

FUNC(int, GetPrimaryModInfoCountPtr, int);
FUNC(const char*, GetInfoTypePtr, int);
FUNC(const char*, GetInfoValueStringPtr, int);
FUNC(int, GetInfoValueIntegerPtr, int);
FUNC(float, GetInfoValueFloatPtr, int);
FUNC(bool, GetInfoValueBoolPtr, int);
FUNC(int, GetMapInfoCountPtr, int);

} //namespace LSL

#endif // LSL_SIGNATURES_H
