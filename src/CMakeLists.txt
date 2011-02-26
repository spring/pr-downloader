

subdirs(Downloader)

set(myRapidFiles
	FileSystem.cpp
	Util.cpp
	md5.cpp
	sha1.cpp
	main.cpp
	Download.cpp
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
	Widget
	${CURL_LIBRARIES}
	${ZLIB_LIBRARIES}
	${XMLRPC_LIBRARIES}
	${ADDITIONAL_LIBS}
)

add_executable(${RAPID} ${myRapidFiles})

target_link_libraries(${RAPID}
	${RAPID_LIB}
	${ADDITIONAL_LIBS}
)

FILE( GLOB_RECURSE header "*.h" )

IF(WIN32)
	INSTALL(TARGETS ${RAPID} ${RAPID_LIB} DESTINATION  ${CMAKE_SOURCE_DIR}/dist)
	INSTALL(FILES
		${MINGWLIBS}/bin/libcurl-4.dll
		${CMAKE_SOURCE_DIR}/data/mingwm10.dll
	DESTINATION ${CMAKE_SOURCE_DIR}/dist)
ELSE()
	INSTALL (FILES Util.h md5.h FileSystem.h DESTINATION ${INCLUDE_DIR} )
	INSTALL (TARGETS ${RAPID} ${RAPID_LIB}
		RUNTIME DESTINATION ${BINDIR}
		LIBRARY DESTINATION ${LIBDIR}
		ARCHIVE DESTINATION ${LIBDIR} )
ENDIF()

