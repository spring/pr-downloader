
subdirs(lib)
subdirs(Downloader)
subdirs(FileSystem)

set(PRDOWNLOADER_FILES
	Util.cpp
	Logger.cpp
	Downloader/Download.cpp
	Downloader/IDownloader.cpp
	Downloader/Mirror.cpp
)

set (PRDOWNLOADER "pr-downloader")
set (PRDOWNLOADER_SHARED ${PRDOWNLOADER}_shared)
set (PRDOWNLOADER_STATIC ${PRDOWNLOADER}_static)

IF (WIN32)
	SET(WIN32LIBS ws2_32)
	LIST(APPEND WIN32LIBS mswsock)
ENDIF ()



add_library(${PRDOWNLOADER_SHARED} SHARED
	${PRDOWNLOADER_FILES}
	pr-downloader.cpp
)

add_library(${PRDOWNLOADER_STATIC} STATIC
	${PRDOWNLOADER_FILES}
	pr-downloader.cpp
)

add_executable(${PRDOWNLOADER}
	${PRDOWNLOADER_FILES}
	main.cpp
)

set(PRDOWNLOADER_LIBS
	${WIN32LIBS}
	Rapid
	Http
	Plasma
	Widget
	FileSystem
)

target_link_libraries( ${PRDOWNLOADER_SHARED} ${PRDOWNLOADER_LIBS} )
target_link_libraries( ${PRDOWNLOADER_STATIC} ${PRDOWNLOADER_LIBS} )
target_link_libraries( ${PRDOWNLOADER} ${PRDOWNLOADER_SHARED} )

FILE( GLOB_RECURSE header "*.h" )

INSTALL (TARGETS ${PRDOWNLOADER} ${PRDOWNLOADER_SHARED} ${PRDOWNLOADER_STATIC}
	RUNTIME DESTINATION ${BINDIR}
	LIBRARY DESTINATION ${LIBDIR}
	ARCHIVE DESTINATION ${LIBDIR} )
IF(NOT WIN32)
	SET(VERSION ${PR_DOWNLOADER_VERSION})
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/libspringdownloader.pc.in"
	               "${CMAKE_CURRENT_BINARY_DIR}/libspringdownloader.pc" @ONLY)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libspringdownloader.pc" DESTINATION "${LIBDIR}/pkgconfig")
ENDIF(NOT WIN32)

INSTALL (FILES
        pr-downloader.h
        DESTINATION ${INCLUDE_DIR}/Downloader COMPONENT Devel )

