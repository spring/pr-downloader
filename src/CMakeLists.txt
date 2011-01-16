

subdirs(Downloader)

set(myRapidFiles
	FileSystem.cpp
	Util.cpp
	md5.cpp
	main.cpp
	Downloader/IDownloader.cpp
)

set (RAPID "pr-downloader")
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
	${XMLRPC_LIBRARIES}
	${ADDITIONAL_LIBS}
)

add_executable(${RAPID} ${myRapidFiles})

target_link_libraries(${RAPID}
	${RAPID_LIB}
	${ADDITIONAL_LIBS}
)

IF(WIN32)
	INSTALL(TARGETS ${RAPID} ${RAPID_LIB} DESTINATION  ${CMAKE_SOURCE_DIR}/dist)
	INSTALL(FILES
		${MINGWLIBS}/lib/libboost_system-mt.dll
		${MINGWLIBS}/lib/libboost_thread_win32-mt.dll
		${MINGWLIBS}/lib/libboost_filesystem-mt.dll
		${MINGWLIBS}/bin/libcurl-4.dll
		${CMAKE_SOURCE_DIR}/data/mingwm10.dll
	DESTINATION ${CMAKE_SOURCE_DIR}/dist)
ELSE()
	INSTALL (TARGETS ${RAPID} ${RAPID_LIB}
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib)
ENDIF()

