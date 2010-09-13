

subdirs(Downloader)

set(myRapidFiles
	FileSystem.cpp
	Util.cpp
	md5.cpp
)

set (RAPID "rapid")
add_library(unitsync-ext ${myRapidFiles} ${downloader_LIBRARY})
add_executable(rapid "main.cpp")

target_link_libraries(unitsync-ext downloader)
target_link_libraries(rapid
	unitsync-ext
	${ZLIB_LIBRARIES}
	${CURL_LIBRARIES}
	${LIBTORRENT_LIBRARY}
	${BOOST_LIBRARIES}
	${TORRENT_LIBRARY}
	${CURL_LIBRARY}
)
