

subdirs(Downloader)

set(myRapidFiles
	FileSystem.cpp
	Util.cpp
	md5.cpp
	sha1.cpp
	main.cpp
	Download.cpp
	Downloader/IDownloader.cpp
	bencode/bencode.c
)

set (RAPID "pr-downloader")
set (RAPID_LIB "unitsync-ext")
IF (WIN32)
	SET(WIN32LIBS ws2_32)
	LIST(APPEND WIN32LIBS  mswsock)
ENDIF ()



add_library(${RAPID_LIB} SHARED
	${myRapidFiles}
)

target_link_libraries(${RAPID_LIB}
	Rapid
	Http
	Plasma
	Widget
	${CURL_LIBRARIES}
	${ZLIB_LIBRARIES}
	${XMLRPC_LIBRARIES}
	${ADDITIONAL_LIBS}
)

add_executable(${RAPID} ${myRapidFiles})

target_link_libraries(${RAPID}
	${WIN32LIBS}
	${ZLIB_LIBRARIES}
	${XMLRPC_LIBRARIES}
	${CURL_LIBRARIES}
	Rapid
	Http
	Plasma
	Widget
)

IF(WIN32)
	INSTALL(TARGETS ${RAPID} ${RAPID_LIB} DESTINATION  ${CMAKE_SOURCE_DIR}/dist)
	INSTALL(FILES
		${MINGWLIBS}/bin/libcurl-4.dll
		${CMAKE_SOURCE_DIR}/data/mingwm10.dll
	DESTINATION ${CMAKE_SOURCE_DIR}/dist)
ELSE()
	INSTALL (TARGETS ${RAPID} ${RAPID_LIB}
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib)
ENDIF()

