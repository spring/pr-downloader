/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "config.h"

#include <lslutils/conversion.h>

#include "misc.h"
#include "globalsmanager.h"

#ifdef WIN32
#ifdef _POSIX_
#undef _POSIX_
#endif
#include <stdlib.h>
int setenv(const char *name, const char *value, int overwrite)
{
    return _putenv_s(name, value);
}
#endif

namespace LSL
{
namespace Util
{

// FIXME chose better defaults
Config::Config()
    : Cache("cache")
    , CurrentUsedUnitSync("unitsync")
    , CurrentUsedSpringBinary("spring")
{
}

Config& config()
{
	static LSL::Util::LineInfo<Config> m(AT);
	static LSL::Util::GlobalObjectHolder<Config, LSL::Util::LineInfo<Config> > m_sett(m);
	return m_sett;
}

lslColor Config::GetBattleLastColor() const
{
	//    auto
	return lslColor();
}

std::string Config::GetCachePath() const
{
	return Cache;
}

std::string Config::GetCurrentUsedUnitSync() const
{
	return CurrentUsedUnitSync;
}


std::string Config::GetCurrentUsedSpringBinary() const
{
	return CurrentUsedSpringBinary;
}

std::string Config::GetDataDir() const
{
	return DataDir;
}

void Config::ConfigurePaths(const std::string& Cache, const std::string& CurrentUsedUnitSync, const std::string& CurrentUsedSpringBinary, const std::string& DataDir)
{
	this->Cache = Cache;
	this->CurrentUsedUnitSync = CurrentUsedUnitSync;
	this->CurrentUsedSpringBinary = CurrentUsedSpringBinary;
	this->DataDir = DataDir;
	setenv("SPRING_WRITEDIR", DataDir.c_str(), 1);
}

} // namespace Util
} // namespace LSL {
