

subdirs(Downloader)

set(myRapidFiles
	FileSystem.cpp
	Util.cpp
	md5.cpp
)

set (RAPID "rapid")
set (RAPID_LIB "unitsync-ext")
add_library(${RAPID_LIB} ${myRapidFiles} ${downloader_LIBRARY})
add_executable(${RAPID} "main.cpp")
#target_link_libraries(${RAPID_LIB} "downloader")

target_link_libraries(${RAPID}
	${ZLIB_LIBRARIES}
	${RAPID_LIB}
	${CURL_LIBRARIES}
	${LIBTORRENT_LIBRARY}
	${BOOST_LIBRARIES}
	${TORRENT_LIBRARY}
	${CURL_LIBRARY}
)
