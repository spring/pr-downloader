

subdirs(Downloader)
subdirs(FileSystem)

set(myRapidFiles
	Util.cpp
	Logger.cpp
	Download.cpp
	Downloader/IDownloader.cpp
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
	FileSystem
	${CURL_LIBRARIES}
	${ZLIB_LIBRARIES}
	${XMLRPC_LIBRARIES}
	${ADDITIONAL_LIBS}
)

add_executable(${RAPID}
	main.cpp
	${myRapidFiles}
)

target_link_libraries(${RAPID}
	${WIN32LIBS}
	${ZLIB_LIBRARIES}
	${XMLRPC_LIBRARIES}
	${CURL_LIBRARIES}
	Rapid
	Http
	Plasma
	Widget
	FileSystem
)

FILE( GLOB_RECURSE header "*.h" )

IF(WIN32)
	INSTALL(TARGETS ${RAPID} ${RAPID_LIB} DESTINATION  ${CMAKE_SOURCE_DIR}/dist)
ELSE()
	INSTALL (TARGETS ${RAPID} ${RAPID_LIB}
		RUNTIME DESTINATION ${BINDIR}
		LIBRARY DESTINATION ${LIBDIR}
		ARCHIVE DESTINATION ${LIBDIR} )

	SET( VERSION "0.1.1" )
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/libspringdownloader.pc.in"
	               "${CMAKE_CURRENT_BINARY_DIR}/libspringdownloader.pc" @ONLY)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libspringdownloader.pc" DESTINATION "${LIBDIR}/pkgconfig")

ENDIF(WIN32)

