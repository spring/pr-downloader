/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "springbundle.h"
#include "sharedlib.h"
#include "signatures.h"
#include <string>

#include "lslutils/misc.h"
#include "lslutils/logging.h"

namespace LSL
{

bool SpringBundle::GetBundleVersion()
{
	if (!version.empty()) //get version only once
		return true;
	if (!Util::FileExists(unitsync)) {
		return false;
	}
	LslInfo("Trying to load %s", unitsync.c_str());
	void* temphandle = _LoadLibrary(unitsync);
	if (temphandle == nullptr)
		return false;
	std::string functionname = "GetSpringVersion";
	GetSpringVersionPtr getspringversion = (GetSpringVersionPtr)GetLibFuncPtr(temphandle, functionname);
	if (!getspringversion) {
		_FreeLibrary(temphandle);
		LslError("getspringversion: function not found %s", unitsync.c_str());
		return false;
	}
	functionname = "IsSpringReleaseVersion";
	IsSpringReleaseVersionPtr isspringreleaseversion = (IsSpringReleaseVersionPtr)GetLibFuncPtr(temphandle, functionname);

	functionname = "GetSpringVersionPatchset";
	GetSpringVersionPatchsetPtr getspringversionpatcheset = (GetSpringVersionPatchsetPtr)GetLibFuncPtr(temphandle, functionname);

	version = getspringversion();
	if (isspringreleaseversion && getspringversionpatcheset && isspringreleaseversion()) {
		version += ".";
		version += getspringversionpatcheset();
	}
	_FreeLibrary(temphandle);

	if (version == "98.0") {
		LslError("Incompatible Spring version: %s", version.c_str());
		version.clear();
		return false;
	}
	return !version.empty();
}

bool SpringBundle::IsValid()
{
	if (valid)
		return true; //verify only once
	if (!Util::FileExists(path)) {
		return false;
	}
	if (!Util::FileExists(spring)) {
		return false;
	}
	valid = GetBundleVersion();
	return valid;
}

bool SpringBundle::AutoFindUnitsync(const std::string& unitsyncpath)
{
	if (!unitsync.empty() && (Util::FileExists(unitsync)))
		return true;
	std::string tmp = unitsyncpath + SEP + "unitsync" + LIBEXT;
	if (Util::FileExists(tmp)) {
		unitsync = tmp;
		return true;
	}

	tmp = unitsyncpath + SEP + "libunitsync" + LIBEXT;
	if (Util::FileExists(tmp)) {
		unitsync = tmp;
		return true;
	}
	return false;
}

bool SpringBundle::AutoComplete(std::string searchpath)
{
	// try to find unitsync file name from path
	if (unitsync.empty()) {
		if (!searchpath.empty() && (AutoFindUnitsync(searchpath))) {
		} else if (!path.empty())
			AutoFindUnitsync(path);
	}
	//try to find path from unitsync
	if (path.empty() && !unitsync.empty()) {
		const std::string tmp = Util::ParentPath(unitsync);
		if (Util::FileExists(tmp))
			path = tmp;
	}
	//try to find path from spring
	if (path.empty() && !spring.empty()) {
		const std::string tmp = Util::ParentPath(spring);
		if (Util::FileExists(tmp))
			path = tmp;
	}
	if (spring.empty()) {
		std::string tmp = path + SEP + "spring" + EXEEXT;
		if (Util::FileExists(tmp)) {
			spring = tmp;
		} else {
			tmp = searchpath + SEP + "spring" + EXEEXT;
			if (Util::FileExists(tmp)) {
				spring = tmp;
			}
		}
	}
	if (version.empty()) {
		GetBundleVersion();
	}
	//printf("%s %s %s %s %s\n", __FUNCTION__, searchpath.c_str(), unitsync.c_str(), spring.c_str(), version.c_str());
	return IsValid();
}

std::string SpringBundle::Serialize()
{
	std::string ret = "version " + version + "\n";
	ret += "version " + spring + "\n";
	ret += "unitsync " + unitsync + "\n";
	ret += "path " + path + "\n";
	return ret;
}

// adds path to pathlist if it exists
static void AddPath(const std::string& path, LSL::StringVector& pathlist)
{
	if (LSL::Util::FileExists(path)) {
		pathlist.push_back(path);
	}
}

//reads envvar, splits it by : and ; and add it to pathlist, when exists
static void GetEnv(const std::string& name, LSL::StringVector& pathlist)
{
	const char* envvar = getenv(name.c_str());
	if (envvar == NULL)
		return;
	LSL::StringVector res = LSL::Util::StringTokenize(envvar, ";:");
	for (const std::string path : res) {
		AddPath(path, pathlist);
	}
}

// searches in OS standard paths for a system installed spring
bool SpringBundle::LocateSystemInstalledSpring(LSL::SpringBundle& bundle)
{
	LSL::StringVector paths;

	GetEnv("SPRING_BUNDLE_DIR", paths);
	GetEnv("PATH", paths);
	GetEnv("ProgramFiles", paths);
	GetEnv("ProgramFiles(x86)", paths);
	//GetEnv("ProgramFiles(x64)", paths); //32 bit springlobby can't use 64 bit
	GetEnv("LD_LIBRARY_PATH", paths);
	GetEnv("LDPATH", paths);

	AddPath("/usr/local/lib/spring", paths);
	AddPath("/usr/local/lib64", paths);
	AddPath("/usr/local/games", paths);
	AddPath("/usr/local/games/lib", paths);
	AddPath("/usr/local/lib", paths);
	AddPath("/usr/lib64", paths);
	AddPath("/usr/lib", paths);
	AddPath("/usr/lib/spring", paths);
	AddPath("/usr/games", paths);
	AddPath("/usr/games/lib64", paths);
	AddPath("/usr/games/lib", paths);
	AddPath("/lib", paths);
	AddPath("/bin", paths);

	for (const std::string path : paths) {
		if (bundle.AutoComplete(path)) {
			return true;
		}
	}
	return false;
}


std::map<std::string, SpringBundle> SpringBundle::GetSpringVersionList(const std::list<SpringBundle>& unitsync_paths)
{
	std::map<std::string, SpringBundle> ret;
	std::map<std::string, std::string> uniq;

	for (SpringBundle bundle : unitsync_paths) {
		try {
			bundle.AutoComplete();
			if (uniq.find(bundle.unitsync) != uniq.end()) //don't check/add the same unitsync twice
				continue;
			if (bundle.IsValid() && (ret.find(bundle.version) == ret.end())) {
				LslDebug("Found spring version: %s %s %s", bundle.version.c_str(), bundle.spring.c_str(), bundle.unitsync.c_str());
				ret[bundle.version] = bundle;
				uniq[bundle.unitsync] = bundle.version;
			}
		} catch (...) {
		}
	}
	return ret;
}
};
