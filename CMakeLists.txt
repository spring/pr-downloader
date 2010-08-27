
cmake_minimum_required(VERSION 2.6)


if    (MINGW)
	include_directories(${MINGWLIBS}/include)
        set(CMAKE_LIBRARY_PATH ${MINGWLIBS}/dll)
        set(CMAKE_PREFIX_PATH ${MINGWLIBS})
        set (CMAKE_FIND_ROOT_PATH ${MINGWLIBS} ${CMAKE_FIND_ROOT_PATH})

endif (MINGW)

project(rapid)
Include(FindCURL)
Include(FindZLIB)

add_executable(rapid
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
target_link_libraries(rapid ${CURL_LIBRARIES} ${ZLIB_LIBRARIES})

