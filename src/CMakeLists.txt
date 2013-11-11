subdirs(lib)
subdirs(Downloader)
subdirs(FileSystem)

add_library(Version STATIC Version.cpp)
set_target_properties(Version PROPERTIES COMPILE_DEFINITIONS "PR_DOWNLOADER_VERSION=${PR_DOWNLOADER_VERSION}")


add_library(Util STATIC
	Util.cpp
	Logger.cpp
	Downloader/Download.cpp
	Downloader/IDownloader.cpp
	Downloader/Mirror.cpp
	Downloader/IDownloadsObserver.cpp
)

add_library(CurlWrapper
	Downloader/CurlWrapper.cpp
)
target_link_libraries(CurlWrapper Version)

set (PRDOWNLOADER "pr-downloader")
set (PRDOWNLOADER_SHARED ${PRDOWNLOADER}_shared)
set (PRDOWNLOADER_STATIC ${PRDOWNLOADER}_static)

IF (WIN32)
	SET(WIN32LIBS ws2_32)
	LIST(APPEND WIN32LIBS mswsock)
ENDIF ()



set(PRDOWNLOADER_LIBS
	${WIN32LIBS}
	Util
	Rapid
	Http
	CurlWrapper
	Plasma
	Widget
	FileSystem
)


OPTION(SHAREDLIB
	"Enables compilation of the shared lib" ON)

if (SHAREDLIB)
	add_library(${PRDOWNLOADER_SHARED} SHARED
		pr-downloader.cpp
	)
	if(DO_INSTALL)
		INSTALL (TARGETS ${PRDOWNLOADER_SHARED}
			RUNTIME DESTINATION ${BINDIR}
			LIBRARY DESTINATION ${LIBDIR}
			ARCHIVE DESTINATION ${LIBDIR} )
	endif()
	target_link_libraries( ${PRDOWNLOADER_SHARED} ${PRDOWNLOADER_LIBS} )
endif()

OPTION(STATICLIB
	"Enables compilation of the static lib" ON)
if (STATICLIB)
	add_library(${PRDOWNLOADER_STATIC} STATIC
		pr-downloader.cpp
	)
	if(DO_INSTALL)
		INSTALL (TARGETS ${PRDOWNLOADER_STATIC}
			RUNTIME DESTINATION ${BINDIR}
			LIBRARY DESTINATION ${LIBDIR}
			ARCHIVE DESTINATION ${LIBDIR} )
	endif()
	target_link_libraries( ${PRDOWNLOADER_STATIC} ${PRDOWNLOADER_LIBS} ${CMAKE_DL_LIBS})
endif()

OPTION(CONSOLETOOL
        "Enables compilation and installation of the console tool ${PRDOWNLOADER}" ON)

if (CONSOLETOOL)
	add_executable(${PRDOWNLOADER}
		main.cpp
	)

	if (LINK_STATIC)
		target_link_libraries( ${PRDOWNLOADER} ${PRDOWNLOADER_STATIC} )
	else  ()
		# hack arround "make install" doesn't generate libpr-downloader_shared.so
		add_dependencies(${PRDOWNLOADER} ${PRDOWNLOADER_SHARED})
		target_link_libraries(${PRDOWNLOADER} ${PRDOWNLOADER_SHARED} )
	endif ()
	if (WIN32)
		set_target_properties(${PRDOWNLOADER} PROPERTIES LINK_FLAGS "-Wl,-subsystem,console")
	endif()
	if (DO_INSTALL)
		INSTALL (TARGETS ${PRDOWNLOADER}
			RUNTIME DESTINATION ${BINDIR}
			LIBRARY DESTINATION ${LIBDIR}
			ARCHIVE DESTINATION ${LIBDIR} )
	endif()
endif(CONSOLETOOL)

FILE( GLOB_RECURSE header "*.h" )



if(NOT WIN32 AND DEVELOP_FILES)
	SET(VERSION ${PR_DOWNLOADER_VERSION})
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/libspringdownloader.pc.in"
	               "${CMAKE_CURRENT_BINARY_DIR}/libspringdownloader.pc" @ONLY)
	if (DO_INSTALL)
		install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libspringdownloader.pc" DESTINATION "${LIBDIR}/pkgconfig")
	endif()
endif()

if(DO_INSTALL AND DEVELOP_FILES AND DO_INSTALL)
	INSTALL (FILES
		pr-downloader.h
	        DESTINATION ${INCLUDE_DIR}/Downloader COMPONENT Devel )
endif()

