cmake_minimum_required(VERSION 2.6.3)
project(pr-downloader)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None DEBUG RELEASE."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)


IF(CMAKE_BUILD_TYPE MATCHES "DEBUG")
	add_definitions("-DDEBUG")
endif()
#set (CMAKE_CXX_FLAGS_DEBUG "-DDEBUG")
#set (CMAKE_CXX_FLAGS_RELEASE "")

SET(CMAKE_MODULES_DOWNLOADER "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_MODULES_DOWNLOADER}")
INCLUDE(UtilGit)
if (GIT_FOUND)
	Git_Util_Describe(PR_DOWNLOADER_VERSION "${CMAKE_CURRENT_SOURCE_DIR}/src" "*")
else()
	SET(PR_DOWNLOAD_VERSION "dev")
endif()

OPTION(LINK_STATIC "link pr-downloader.exe static or dynamic" ON)

MESSAGE(STATUS "Version ${PR_DOWNLOADER_VERSION}")
if   (MINGW)
	if(NOT DEFINED ${MINGWLIBS})
		set(MINGWLIBS "${CMAKE_SOURCE_DIR}/mingwlibs" CACHE PATH "Location of the mingwlibs package")
	endif()
	if(NOT EXISTS "${MINGWLIBS}" OR NOT IS_DIRECTORY "${MINGWLIBS}")
		MESSAGE(FATAL_ERROR "MINGWLIBS is not a valid directory")
	endif ()
	include_directories(${MINGWLIBS}/include)
	include_directories("${CMAKE_FIND_ROOT_PATH}/include")


	if (LINK_STATIC)
		Set(LD_FLAGS "-static-libstdc++ -static-libgcc")
		Set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    ${LD_FLAGS}")
		Set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${LD_FLAGS}")
	endif()
	set(CMAKE_LIBRARY_PATH ${MINGWLIBS}/lib)
	set(CMAKE_PREFIX_PATH ${MINGWLIBS})
	set(CMAKE_FIND_ROOT_PATH ${MINGWLIBS} ${CMAKE_FIND_ROOT_PATH})
	set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
	set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
	set(CMAKE_SHARED_LIBRARY_PREFIX "")
	set(CMAKE_SHARED_MODULE_PREFIX "")
	set(CMAKE_CXX_FLAGS "-Wall -O2 -g -DWIN32 -D_WINDOWS -DCURL_STATICLIB")
	set (BINDIR  "."                  CACHE PATH "Where to install binaries" )
	set (LIBDIR  "."                  CACHE PATH "Where to install libraries" )
	set (INCLUDE_DIR  "include/spring/" CACHE PATH "Where to install headers" )
else (MINGW)
	set(CMAKE_CXX_FLAGS "-Wall -O0 -g -fPIC")
	#set(CMAKE_SKIP_BUILD_RPATH  FALSE)
	#set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
	set (BINDIR  "bin"                  CACHE PATH "Where to install binaries" )
	set (LIBDIR  "lib"                  CACHE PATH "Where to install libraries" )
	set (INCLUDE_DIR  "include/spring/" CACHE PATH "Where to install headers" )
	#set(CMAKE_INSTALL_RPATH "${LIBDIR}")
endif (MINGW)

#FIXME: set this compile flag only for one file
add_definitions(-DPR_DOWNLOADER_VERSION=${PR_DOWNLOADER_VERSION})
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS}")

find_package(CURL REQUIRED)
find_package(ZLIB REQUIRED)

#compile fix for soap (springlobby defines this)
remove_definitions(-DHAVE_CONFIG_H)

include_directories(SYSTEM ${CURL_INCLUDE_DIR})
include_directories(SYSTEM ${ZLIB_INCLUDE_DIR})


include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")

# MINIZIP_FOUND is used in lib & FileSystem
FIND_PACKAGE(MiniZip)
if(NOT MINIZIP_FOUND)
	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/lib")
endif()

subdirs(src)

ENABLE_TESTING()
Add_Subdirectory(test)

