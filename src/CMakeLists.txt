
subdirs(lib)
subdirs(Downloader)
subdirs(FileSystem)

set(myRapidFiles
	Util.cpp
	Logger.cpp
	Downloader/Download.cpp
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
	${XMLRPC_LIBRARIES}
)

add_executable(${RAPID}
	main.cpp
	${myRapidFiles}
)

target_link_libraries(${RAPID}
	${WIN32LIBS}
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

	SET( VERSION "${PR_DOWNLOADER_VERSION}" )
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/libspringdownloader.pc.in"
	               "${CMAKE_CURRENT_BINARY_DIR}/libspringdownloader.pc" @ONLY)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libspringdownloader.pc" DESTINATION "${LIBDIR}/pkgconfig")

ENDIF(WIN32)

