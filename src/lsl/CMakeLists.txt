

set(Boost_USE_MULTITHREADED ON)
if(${CMAKE_CROSSCOMPILING})
	set(Boost_USE_STATIC_LIBS ON)
	set(Boost_USE_STATIC_RUNTIME ON)
endif()
find_package(Boost 1.40.0 COMPONENTS system REQUIRED)

#----------------------------------------------------------------------------------------------------
# Options, that can be changed be the User in order to customise libSpringLobby
#----------------------------------------------------------------------------------------------------
option(BUILD_SHARED_LIBS "Chooses whether to link dynamic or static libraries. Recommend keeping this activated unless you know what you're doing." ON)

set(LIBSPRINGLOBBY_REV	"${PR_DOWNLOADER_VERSION}")

# If we build for windows Systems, we also include the Resource-File containing the Manifest, Icon and other Resources.
if(WIN32)
	createresourcecompilecommand(libspringlobby_RC_FILE ${libSpringLobby_SOURCE_DIR}/src/ libSpringLobby.rc lsl_icon.o)
	set(libSpringLobbySrc ${libSpringLobbySrc} ${libspringlobby_RC_FILE})
	add_definitions(-DUNICODE -D_UNICODE)
endif()


#----------------------------------------------------------------------------------------------------
# Build target defintions
#----------------------------------------------------------------------------------------------------

# Here we define the executable lsl-server ( or on Windows libSpringLobby.exe )

option(LSLSERVER "Compile and install lsl-server (broken)" OFF)

add_subdirectory( lslutils )
if(LSLSERVER)
	add_subdirectory( lsl )
endif()
add_subdirectory( lslunitsync )
option(LSL_EXTRACT "Compile lslextract, a tool to extract metadata from spring's archive files" OFF)
if(LSL_EXTRACT)
	add_subdirectory( lslextract )
endif()
