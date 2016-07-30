#don't write when git errored out resulting in unset version (ie when compiling from tarball)
IF ( NOT GIT_ERROR )
    CONFIGURE_FILE( ${libSpringLobby_SOURCE_DIR}/cmake/config.h ${libSpringLobby_BINARY_DIR}/libSpringLobby_config.h )
ENDIF ( NOT GIT_ERROR )
#if nothing went wrong we have the file and can define HAVE_CONFIG_H
IF ( EXISTS ${libSpringLobby_BINARY_DIR}/libSpringLobby_config.h  OR  EXISTS ${libSpringLobby_SOURCE_DIR}/libSpringLobby_config.h  )
    ADD_DEFINITIONS( -DHAVE_CONFIG_H )
ENDIF ( EXISTS ${libSpringLobby_BINARY_DIR}/libSpringLobby_config.h  OR  EXISTS ${libSpringLobby_SOURCE_DIR}/libSpringLobby_config.h  )


INCLUDE (CheckFunctionExists)

add_subdirectory( lslutils )
if (LSLSERVER)
	add_subdirectory( lsl )
endif()
add_subdirectory( lslunitsync )
option(LSL_EXTRACT "Compile lslextract, a tool to extract metadata from spring's archive files" OFF)
if (LSL_EXTRACT)
	add_subdirectory( lslextract )
endif()
