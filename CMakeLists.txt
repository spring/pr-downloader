cmake_minimum_required(VERSION 2.6)
project(pr-downloader)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None DEBUG RELEASE."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

set (CMAKE_CXX_FLAGS_DEBUG "-DDEBUG")
set (CMAKE_CXX_FLAGS_RELEASE "")


if   (MINGW)
	set(MINGWLIBS "${CMAKE_SOURCE_DIR}/mingwlibs" CACHE PATH "Location of the mingwlibs package")
	if(NOT EXISTS "${MINGWLIBS}" OR NOT IS_DIRECTORY "${MINGWLIBS}")
		MESSAGE(FATAL_ERROR "MINGWLIBS is not a valid directory")
	endif ()
	include_directories(${MINGWLIBS}/include)
#FIXME: remove hardcoded path
	include_directories("/usr/i586-mingw32msvc/include/")
	set(CMAKE_LIBRARY_PATH ${MINGWLIBS}/lib)
	set(CMAKE_PREFIX_PATH ${MINGWLIBS})
	set(CMAKE_FIND_ROOT_PATH ${MINGWLIBS} ${CMAKE_FIND_ROOT_PATH})
	set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
	set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
	set(CMAKE_SHARED_LIBRARY_PREFIX "")
	set(CMAKE_SHARED_MODULE_PREFIX "")

	set(CMAKE_CXX_FLAGS "-Wall -O2 -g -D_WIN32_WINNT=0x0501 -DWIN32 -D_WINDOWS -DCURL_STATICLIB")
	set(INCLUDE_DIR  "${CMAKE_SOURCE_DIR}/include/spring/" CACHE PATH "Where to install headers" )
else (MINGW)
	set(CMAKE_CXX_FLAGS "-Wall -O0 -g -fPIC")
	set(CMAKE_SKIP_BUILD_RPATH  FALSE)
	set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
	set (BINDIR  "${CMAKE_INSTALL_PREFIX}/bin"                  CACHE PATH "Where to install binaries" )
	set (LIBDIR  "${CMAKE_INSTALL_PREFIX}/lib"                  CACHE PATH "Where to install libraries" )
	set (INCLUDE_DIR  "${CMAKE_INSTALL_PREFIX}/include/spring/" CACHE PATH "Where to install headers" )
	mark_as_advanced( BINDIR LIBDIR INCLUDE_DIR )
	set(CMAKE_INSTALL_RPATH "${LIBDIR}")
endif (MINGW)
set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})

find_package(CURL REQUIRED)
find_package(ZLIB REQUIRED)

include_directories(SYSTEM ${CURL_INCLUDE_DIR})
include_directories(SYSTEM ${ZLIB_INCLUDE_DIR})


include_directories("${CMAKE_SOURCE_DIR}/src")

subdirs(src)

ENABLE_TESTING()
Add_Subdirectory(test)

