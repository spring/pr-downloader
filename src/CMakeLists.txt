

subdirs(Downloader)

set(myRapidFiles
	FileSystem.cpp
	Util.cpp
	md5.cpp
	main.cpp
	Downloader/IDownloader.cpp
)

set (RAPID "rapid")
set (RAPID_LIB "unitsync-ext")


add_library(${RAPID_LIB} SHARED
	${myRapidFiles}
)

target_link_libraries(${RAPID_LIB}
	Rapid
	Http
	Plasma
	Torrent
	Widget
	${CURL_LIBRARIES}
	${LIBTORRENT_LIBRARY}
	${ZLIB_LIBRARIES}
	${Boost_LIBRARIES}
	${TORRENT_LIBRARY}
	${CURL_LIBRARY}
	${ADDITIONAL_LIBS}
)

add_executable(rapid ${myRapidFiles})

target_link_libraries(rapid
	${RAPID_LIB}
	${ADDITIONAL_LIBS}
)

