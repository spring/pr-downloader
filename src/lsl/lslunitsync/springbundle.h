/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LIBSPRINGLOBBY_HEADERGUARD_SPRINGBUNDLE_H
#define LIBSPRINGLOBBY_HEADERGUARD_SPRINGBUNDLE_H

#include <string>
#include <map>
#include <list>

namespace LSL
{

class SpringBundle
{
public:
	SpringBundle()
	    : valid(false){};
	bool GetBundleVersion();
	// try to fill missing information by guessing
	bool AutoComplete(std::string searchpath = "");
	bool IsValid();
	std::string unitsync;
	std::string spring;
	std::string version;
	std::string path;
	/**
	 * Loads unitsync from any number of paths in succession,
	 * queries the Spring versions supported by these unitsyncs,
	 * and returns those.
	 *
	 * This is done by a single function because this "transaction"
	 * needs to hold the unitsync lock the entire time.
	 */
	static std::map<std::string, SpringBundle> GetSpringVersionList(const std::list<SpringBundle>& unitsync_paths);
	static bool LocateSystemInstalledSpring(LSL::SpringBundle& bundle);

private:
	bool AutoFindUnitsync(const std::string& unitsyncpath);
	std::string Serialize();
	bool valid;
};
};

#endif
