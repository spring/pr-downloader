subdirs(lib)
subdirs(Downloader)
subdirs(FileSystem)

add_library(Version STATIC Version.cpp)
set_target_properties(Version PROPERTIES COMPILE_DEFINITIONS "PR_DOWNLOADER_VERSION=${PR_DOWNLOADER_VERSION}")

if (PRD_DISABLE_STDOUT_LOGGING)
	add_definitions(-DDISABLE_STDOUT_LOGGING)
endif()

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
	FileSystem
)


OPTION(PRD_SHAREDLIB
	"Enables compilation of the shared lib" ON)

if (PRD_SHAREDLIB)
	add_library(${PRDOWNLOADER_SHARED} SHARED
		pr-downloader.cpp
	)
	if(PRD_DO_INSTALL)
		INSTALL (TARGETS ${PRDOWNLOADER_SHARED}
			RUNTIME DESTINATION ${PRD_BINDIR}
			LIBRARY DESTINATION ${PRD_LIBDIR}
			ARCHIVE DESTINATION ${PRD_LIBDIR} )
	endif()
	target_link_libraries( ${PRDOWNLOADER_SHARED} ${PRDOWNLOADER_LIBS} )
endif()

OPTION(PRD_STATICLIB
	"Enables compilation of the static lib" ON)
if (PRD_STATICLIB)
	add_library(${PRDOWNLOADER_STATIC} STATIC
		pr-downloader.cpp
	)
	if(PRD_DO_INSTALL)
		INSTALL (TARGETS ${PRDOWNLOADER_STATIC}
			RUNTIME DESTINATION ${PRD_BINDIR}
			LIBRARY DESTINATION ${PRD_LIBDIR}
			ARCHIVE DESTINATION ${PRD_LIBDIR} )
	endif()
	target_link_libraries( ${PRDOWNLOADER_STATIC} ${PRDOWNLOADER_LIBS} ${CMAKE_DL_LIBS})
endif()

OPTION(PRD_CONSOLETOOL
        "Enables compilation and installation of the console tool ${PRDOWNLOADER}" ON)

if (PRD_CONSOLETOOL)
	add_executable(${PRDOWNLOADER}
		main.cpp
	)

	if (PRD_LINK_STATIC)
		target_link_libraries( ${PRDOWNLOADER} ${PRDOWNLOADER_STATIC} )
	else  ()
		# hack arround "make install" doesn't generate libpr-downloader_shared.so
		add_dependencies(${PRDOWNLOADER} ${PRDOWNLOADER_SHARED})
		target_link_libraries(${PRDOWNLOADER} ${PRDOWNLOADER_SHARED} )
	endif ()
	if (WIN32)
		set_target_properties(${PRDOWNLOADER} PROPERTIES LINK_FLAGS "-Wl,-subsystem,console")
	endif()
	if (PRD_DO_INSTALL)
		INSTALL (TARGETS ${PRDOWNLOADER}
			RUNTIME DESTINATION ${PRD_BINDIR}
			LIBRARY DESTINATION ${PRD_LIBDIR}
			ARCHIVE DESTINATION ${PRD_LIBDIR} )
	endif()
endif(PRD_CONSOLETOOL)

FILE( GLOB_RECURSE header "*.h" )



if(NOT WIN32 AND PRD_DEVELOP_FILES)
	SET(VERSION ${PR_DOWNLOADER_VERSION})
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/libspringdownloader.pc.in"
	               "${CMAKE_CURRENT_BINARY_DIR}/libspringdownloader.pc" @ONLY)
	if (PRD_DO_INSTALL)
		install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libspringdownloader.pc" DESTINATION "${PRD_LIBDIR}/pkgconfig")
	endif()
endif()

if(PRD_DO_INSTALL AND PRD_DEVELOP_FILES AND PRD_DO_INSTALL)
	INSTALL (FILES
		pr-downloader.h
	        DESTINATION ${PRD_INCLUDE_DIR}/Downloader COMPONENT Devel )
endif()

