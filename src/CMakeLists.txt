

if   (UNIX AND NOT MINGW)
else (UNIX AND NOT MINGW)
        set(MINGWLIBS ${CMAKE_SOURCE_DIR}/mingwlibs CACHE PATH "Location of the mingwlibs package")
      if    (NOT EXISTS "${MINGWLIBS}" OR NOT IS_DIRECTORY "${MINGWLIBS}")
               MESSAGE(FATAL_ERROR "MINGWLIBS is not a valid directory")
       endif (NOT EXISTS "${MINGWLIBS}" OR NOT IS_DIRECTORY "${MINGWLIBS}")

	include_directories(${MINGWLIBS}/include)
	set(CMAKE_LIBRARY_PATH ${MINGWLIBS}/dll)
	set(CMAKE_PREFIX_PATH ${MINGWLIBS})
	set(CMAKE_FIND_ROOT_PATH ${MINGWLIBS} ${CMAKE_FIND_ROOT_PATH})
	set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
	set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
	set(CMAKE_SHARED_LIBRARY_PREFIX "")
	set(CMAKE_SHARED_MODULE_PREFIX "")
	set (MINGW 1)
endif (UNIX AND NOT MINGW)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -O0 -g")

find_package(CURL)
find_package(ZLIB)
include(FindPkgConfig)

include_directories(${CURL_INCLUDE_DIR})
include_directories(${ZLIB_INCLUDE_DIR})
include_directories("src")
include_directories(".")

#----------------------------------------------------------------------------------------------------
# libtorrent-rasterbar dependency check
#----------------------------------------------------------------------------------------------------

IF( NOT WIN32)
	SET ( LIBTORRENT_MIN_VERSION 0.13 )
	pkg_check_modules( LIBTORRENT libtorrent-rasterbar>=${LIBTORRENT_MIN_VERSION} )
	IF( LIBTORRENT_FOUND )
		INCLUDE_DIRECTORIES( ${LIBTORRENT_INCLUDE_DIRS} )
		LINK_LIBRARIES( ${LIBTORRENT_LIBRARIES} )
	ELSE()
		MESSAGE( FATAL_ERROR "libtorrent-rasterbar library not found! Please install the package or toggle OPTION_TORRENT_SYSTEM to OFF")
	ENDIF()
ELSE( NOT WIN32)
	link_directories( ${boost_LIB_DIR} )
	INCLUDE_DIRECTORIES( ${boost_INCLUDE_DIR} )
	ADD_DEFINITIONS( -mthreads  )
	LINK_LIBRARIES( -mthreads torrent-rasterbar ${Boost_LIBRARIES} z ws2_32 mswsock  )
ENDIF( NOT WIN32)



set (RAPID "rapid")
add_executable(${RAPID}
	FileSystem.cpp
	main.cpp
	Util.cpp
	md5.cpp
Downloader/IDownloader.cpp
Downloader/Torrent/TorrentDownloader.h
Downloader/Torrent/Torrent.cpp
Downloader/Torrent/TorrentDownloader.cpp
Downloader/Plasma/PlasmaDownloader.cpp
Downloader/Plasma/soap/soapC.cpp
Downloader/Plasma/soap/soapPlasmaServiceSoap12Proxy.cpp
Downloader/Plasma/soap/soapPlasmaServiceSoapProxy.cpp
Downloader/Plasma/soap/stdsoap2.cpp
Downloader/Rapid/RapidDownloader.cpp
Downloader/Rapid/RepoMaster.cpp
Downloader/Rapid/Repo.cpp
Downloader/Rapid/Sdp.cpp
Downloader/Http/HttpDownloader.cpp
)

target_link_libraries(${RAPID} ${ZLIB_LIBRARIES})

if   (WIN32)
	target_link_libraries(${RAPID} "-Wl,-subsystem,console -lwsock32 ${MINGWLIBS}/dll/libcurl.dll")
else (WIN32)
	target_link_libraries(${RAPID} ${CURL_LIBRARIES})
endif(WIN32)

