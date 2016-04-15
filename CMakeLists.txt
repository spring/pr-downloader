cmake_minimum_required(VERSION 2.8.11)
project(pr-downloader)


IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None DEBUG RELEASE."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)


IF(CMAKE_BUILD_TYPE MATCHES "DEBUG")
	add_definitions("-DDEBUG")
endif()

SET(CMAKE_MODULES_DOWNLOADER "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_MODULES_DOWNLOADER}")
INCLUDE(UtilGit)
if (GIT_FOUND)
	Git_Util_Describe(PR_DOWNLOADER_VERSION "${CMAKE_CURRENT_SOURCE_DIR}/src" "*")
endif()

# Version couldn't be detected, we guess its compiled from tarball
if("${PR_DOWNLOADER_VERSION}" STREQUAL "")
	SET(PR_DOWNLOADER_VERSION "tarball")
endif()

OPTION(PRD_LINK_STATIC "link pr-downloader.exe static or dynamic" ON)
OPTION(PRD_DEVELOP_FILES "install development files (header, libspringdownloader.pc)" OFF)
OPTION(PRD_DO_INSTALL "install files" ON)
OPTION(PRD_ARCHIVE_SUPPORT "enable archive support" ON)
OPTION(PRD_CLEAR_COMPILER_FLAGS "clear all compiler flags" OFF)

find_package(Jsoncpp)
if (${Jsoncpp_FOUND})
	OPTION(PRD_JSONCPP_INTERNAL "use bundled JsonCpp" FALSE)
else()
	OPTION(PRD_JSONCPP_INTERNAL "use bundled JsonCpp" TRUE)
endif()

if(PRD_CLEAR_COMPILER_FLAGS)
	SET(CMAKE_CXX_FLAGS "")
	SET(CMAKE_C_FLAGS "")
	SET(CMAKE_EXE_LINKER_FLAGS "")
	SET(CMAKE_MODULE_LINKER_FLAGS "")
	SET(CMAKE_SHARED_LINKER_FLAGS "")
endif()

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

MESSAGE(STATUS "PR-Downloader version: ${PR_DOWNLOADER_VERSION}")
if   (MINGW)
	if(NOT DEFINED ${MINGWLIBS})
		set(MINGWLIBS "${CMAKE_SOURCE_DIR}/mingwlibs" CACHE PATH "Location of the mingwlibs package")
	endif()
	if(NOT EXISTS "${MINGWLIBS}" OR NOT IS_DIRECTORY "${MINGWLIBS}")
		MESSAGE(WARNING "MINGWLIBS is not a valid directory")
	endif ()


	if (PRD_LINK_STATIC)
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
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -g -DWIN32 -D_WINDOWS -DCURL_STATICLIB")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -g -DWIN32 -D_WINDOWS -DCURL_STATICLIB")
	set (PRD_BINDIR  "."                  CACHE PATH "Where to install binaries" )
	set (PRD_LIBDIR  "."                  CACHE PATH "Where to install libraries" )
	set (PRD_INCLUDE_DIR  "include/spring/" CACHE PATH "Where to install headers" )
else (MINGW)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O0 -g -fPIC")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O0 -g -fPIC")
	#set(CMAKE_SKIP_BUILD_RPATH  FALSE)
	#set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
	set (PRD_BINDIR  "bin"                  CACHE PATH "Where to install binaries" )
	set (PRD_LIBDIR  "lib"                  CACHE PATH "Where to install libraries" )
	set (PRD_INCLUDE_DIR  "include/spring/" CACHE PATH "Where to install headers" )
	#set(CMAKE_INSTALL_RPATH "${PRD_LIBDIR}")
endif (MINGW)

if(WIN32)
	add_definitions(-DUNICODE -D_UNICODE)
endif()

if(MSVC)
	add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
endif()

if (PREFER_STATIC_LIBS)
	#curl dependency
	FIND_PACKAGE_STATIC(OpenSSL REQUIRED)
	FIND_PACKAGE_STATIC(CURL REQUIRED)
	FIND_PACKAGE_STATIC(ZLIB REQUIRED)
else()
	find_package(CURL REQUIRED)
	find_package(ZLIB REQUIRED)
endif()

if (PRD_JSONCPP_INTERNAL)
        # use bundled JsonCpp
        set(PRD_JSONCPP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/jsoncpp/include)
        set(PRD_JSONCPP_LIBRARIES "")
else()
        find_package(Jsoncpp REQUIRED)
	set(PRD_JSONCPP_INCLUDE_DIR ${Jsoncpp_INCLUDE_DIR})
	set(PRD_JSONCPP_LIBRARIES ${Jsoncpp_LIBRARY})
endif()


# MINIZIP_FOUND is used in lib & FileSystem
FIND_PACKAGE(MiniZip)
if(NOT MINIZIP_FOUND)
       message(STATUS "libminizip was not found, using integrated")
       SET(MINIZIP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/lib)
endif()


subdirs(src)

ENABLE_TESTING()
Add_Subdirectory(test)

