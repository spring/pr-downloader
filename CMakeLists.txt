cmake_minimum_required(VERSION 3.1)
project(pr-downloader)


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None DEBUG RELEASE."
      FORCE)
endif()


if(CMAKE_BUILD_TYPE MATCHES "DEBUG")
	add_definitions("-DDEBUG")
endif()

set(CMAKE_MODULES_DOWNLOADER "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_MODULES_DOWNLOADER}")
include(UtilGit)
if (GIT_FOUND)
	git_util_describe(PR_DOWNLOADER_VERSION "${CMAKE_CURRENT_SOURCE_DIR}/src" "*")
endif()

# Version couldn't be detected, we guess its compiled from tarball
if("${PR_DOWNLOADER_VERSION}" STREQUAL "")
	set(PR_DOWNLOADER_VERSION "tarball")
endif()

option(PRD_LINK_STATIC "link pr-downloader.exe static or dynamic" ON)
option(PRD_DEVELOP_FILES "install development files (header, libspringdownloader.pc)" OFF)
option(PRD_DO_INSTALL "install files" ON)
option(PRD_ARCHIVE_SUPPORT "enable archive support" ON)
option(PRD_CLEAR_COMPILER_FLAGS "clear all compiler flags" OFF)
option(PRD_DEFAULT_LOGGER "use default logger" ON)

find_package(Jsoncpp)
if (${Jsoncpp_FOUND})
	option(PRD_JSONCPP_INTERNAL "use bundled JsonCpp" FALSE)
else()
	option(PRD_JSONCPP_INTERNAL "use bundled JsonCpp" TRUE)
endif()

if(PRD_CLEAR_COMPILER_FLAGS)
	set(CMAKE_CXX_FLAGS "")
	set(CMAKE_C_FLAGS "")
	set(CMAKE_EXE_LINKER_FLAGS "")
	set(CMAKE_MODULE_LINKER_FLAGS "")
	set(CMAKE_SHARED_LINKER_FLAGS "")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "PR-Downloader version: ${PR_DOWNLOADER_VERSION}")
if   (MINGW)
	if(NOT DEFINED ${MINGWLIBS})
		set(MINGWLIBS "${CMAKE_SOURCE_DIR}/mingwlibs" CACHE PATH "Location of the mingwlibs package")
	endif()
	if(NOT EXISTS "${MINGWLIBS}" OR NOT IS_DIRECTORY "${MINGWLIBS}")
		message(WARNING "MINGWLIBS is not a valid directory")
	endif ()


	if (PRD_LINK_STATIC)
		set(LD_FLAGS "-static-libstdc++ -static-libgcc")
		set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    ${LD_FLAGS}")
		set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${LD_FLAGS}")
	endif()
	set(CMAKE_LIBRARY_PATH ${MINGWLIBS}/lib ${MINGWLIBS}/dll)
	set(CMAKE_PREFIX_PATH ${MINGWLIBS})
	set(CMAKE_FIND_ROOT_PATH ${MINGWLIBS} ${CMAKE_FIND_ROOT_PATH})
	set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
	set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
	set(CMAKE_SHARED_LIBRARY_PREFIX "")
	set(CMAKE_SHARED_MODULE_PREFIX "")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -g -DWIN32 -D_WINDOWS")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -g -DWIN32 -D_WINDOWS")
	set (PRD_BINDIR  "."                  CACHE PATH "Where to install binaries" )
	set (PRD_LIBDIR  "."                  CACHE PATH "Where to install libraries" )
	set (PRD_INCLUDE_DIR  "include/spring/" CACHE PATH "Where to install headers" )
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O0 -g -fPIC")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O0 -g -fPIC")
	#set(CMAKE_SKIP_BUILD_RPATH  FALSE)
	#set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
	set (PRD_BINDIR  "bin"                  CACHE PATH "Where to install binaries" )
	set (PRD_LIBDIR  "lib"                  CACHE PATH "Where to install libraries" )
	set (PRD_INCLUDE_DIR  "include/spring/" CACHE PATH "Where to install headers" )
	#set(CMAKE_INSTALL_RPATH "${PRD_LIBDIR}")
endif ()

if(WIN32)
	add_definitions(-DUNICODE -D_UNICODE)
endif()

if(MSVC)
	add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
endif()

if (PREFER_STATIC_LIBS)
	#curl dependency
	find_package_static(OpenSSL REQUIRED)
	find_package_static(CURL REQUIRED)
	find_package_static(ZLIB REQUIRED)
else()
	find_package(CURL REQUIRED)
	find_package(ZLIB REQUIRED)
endif()

set(CURL_MIN_VER 7.60)
if (CURL_VERSION_STRING VERSION_LESS ${CURL_MIN_VER})
	message(WARNING "libcurl ${CURL_VERSION_STRING} found but ${CURL_MIN_VER} is required!")
endif()


#find_program(CURL_CONFIG_EXECUTABLE NAMES curl-config ONLY_CMAKE_FIND_ROOT_PATH)
#if (CURL_CONFIG_EXECUTABLE-NOTFOUND)
#        message(FATAL_ERROR "couldn't find curl-config!")
#else()
#        message(STATUS "found curl-config: ${CURL_CONFIG_EXECUTABLE}")
#endif()
#
#	set(CURLCONFIGFLAGS "--static-libs")
#else()
#	set(CURLCONFIGFLAGS "--libs")
#endif()
#
#execute_process(COMMAND ${CURL_CONFIG_EXECUTABLE} ${CURLCONFIGFLAGS}
#        OUTPUT_VARIABLE CURL_LD_FLAGS
#        OUTPUT_STRIP_TRAILING_WHITESPACE )
#message(STATUS "Using CURL_LD_FLAGS: ${CURL_LD_FLAGS}")


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
find_package(MINIZIP)
if(NOT MINIZIP_FOUND)
	message(STATUS "libminizip was not found, using integrated")
	set(MINIZIP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/lib)
endif()


add_subdirectory(src)


find_program(CLANG_FORMAT_BINARY NAMES clang-format-3.7 clang-format-3.6 clang-format-3.5 clang-format-3.4 clang-format)
if (EXISTS ${CLANG_FORMAT_BINARY})
        file(GLOB_RECURSE files {CMAKE_CURRENT_SOURCE_DIR} *.cpp *.hpp *.h)

        foreach(exclude src/lib ) # exclude some files
                foreach(file in ${files})
                        if (file MATCHES "^${CMAKE_CURRENT_SOURCE_DIR}/${exclude}")
                                list(REMOVE_ITEM files ${file})
                        endif()
                endforeach()
        endforeach()
        add_custom_target(format-prd COMMAND ${CLANG_FORMAT_BINARY} -i ${files})
else()
        message(WARNING "clang-format not found")
endif()

option(PRD_TESTS "Enable prd unit tests" ON)

if (${PRD_TESTS})
	enable_testing()
	add_subdirectory(test)
endif()
