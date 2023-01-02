# - Try to find Jsoncpp
# Once done, this will define
# stolen from
# http://stackoverflow.com/questions/18005880/how-to-writing-a-cmake-module-for-jsoncpp
#
#  Jsoncpp_FOUND - system has Jsoncpp
#  Jsoncpp_INCLUDE_DIR - the Jsoncpp include directory
#  Jsoncpp_LIBRARY - link to use Jsoncpp

# Use pkg-config to get hints about paths
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
	pkg_check_modules(Jsoncpp_PKGCONF jsoncpp)
endif()
# Include dir
find_path(Jsoncpp_INCLUDE_DIR
	NAMES json/json.h
	PATH_SUFFIXES jsoncpp
	PATHS ${Jsoncpp_PKGCONF_INCLUDE_DIRS} # /usr/include/jsoncpp/json
)

# Finally the library itself
find_library(Jsoncpp_LIBRARY
	NAMES jsoncpp
	PATHS ${Jsoncpp_PKGCONF_LIBRARY_DIRS}
)
