
cmake_minimum_required(VERSION 2.6)
project(rapid)

if    (UNIX AND NOT MINGW)
else (UNIX AND NOT MINGW)
        set(MINGWLIBS ${CMAKE_SOURCE_DIR}/mingwlibs CACHE PATH "Location of the mingwlibs package")
      if    (NOT EXISTS "${MINGWLIBS}" OR NOT IS_DIRECTORY "${MINGWLIBS}")
               MESSAGE(FATAL_ERROR "MINGWLIBS is not a valid directory")
       endif (NOT EXISTS "${MINGWLIBS}" OR NOT IS_DIRECTORY "${MINGWLIBS}")

	include_directories(${MINGWLIBS}/include)
	set(CMAKE_LIBRARY_PATH ${MINGWLIBS}/dll)
	set(CMAKE_PREFIX_PATH ${MINGWLIBS})
	set (CMAKE_FIND_ROOT_PATH ${MINGWLIBS} ${CMAKE_FIND_ROOT_PATH})
	set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
	set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
	set(CMAKE_SHARED_LIBRARY_PREFIX "")
	set(CMAKE_SHARED_MODULE_PREFIX "")
	set (MINGW 1)
endif (UNIX AND NOT MINGW)

include(FindCURL)
include(FindZLIB)

include_directories(${CURL_INCLUDE_DIR})
include_directories(${ZLIB_INCLUDE_DIR})


set (RAPID "rapid")

add_executable(${RAPID}
	FileSystem.cpp
	main.cpp
	RapidDownloader.cpp
	RepoMaster.cpp
	Util.cpp
	HttpDownload.cpp
	md5.cpp
	Repo.cpp
	Sdp.cpp
)
target_link_libraries(${RAPID} ${CURL_LIBRARIES} ${ZLIB_LIBRARIES})
if (MINGW)
SET_TARGET_PROPERTIES(${RAPID} PROPERTIES LINK_FLAGS "${MINGWLIBS}/dll/libcurl.dll -Wl,-subsystem,console")
endif (MINGW)

