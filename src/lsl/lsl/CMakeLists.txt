SET(libSpringLobbySrc
	"${CMAKE_CURRENT_SOURCE_DIR}/container/userlist.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/container/channellist.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/container/battlelist.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/channel.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/user/user.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/user/userdata.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/user/common.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/networking/socket.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/networking/commands.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/networking/tasserverdataformats.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/networking/iserver.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/networking/tasserver.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/battle/ibattle.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/battle/battle.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/battle/tdfcontainer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/spring/spring.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/spring/springprocess.cpp"
	)

FILE( GLOB RECURSE libSpringLobbyHeader "${CMAKE_CURRENT_SOURCE_DIR}/*.h" )

#grep -lR cc\"$ src/* | sed -e "s/\.h/\.cc/g"
SET( templatesources
	""
)
LIST( APPEND libSpringLobbyHeader ${templatesources} )
set_source_files_properties(  ${libSpringLobbyHeader} PROPERTIES HEADER_FILE_ONLY 1 )

ADD_LIBRARY(lsl-server STATIC ${libSpringLobbyHeader} ${libSpringLobbySrc} ${libSpringLobby_RC_FILE} )
TARGET_LINK_LIBRARIES(lsl-server lsl-utils ${Boost_SYSTEM} ${Boost_THREAD_LIBRARY} )
target_include_directories(lsl-server PRIVATE
	${pr-downloader_SOURCE_DIR}/src/lsl
	${pr-downloader_SOURCE_DIR}/src/
)

if (MINGW)
	TARGET_LINK_LIBRARIES( lsl-server iphlpapi )
endif ()

if (LSLSERVER)
	if (WIN32)
	    install(TARGETS lsl-server DESTINATION . )
	else ()
	    install(TARGETS lsl-server
			LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
			ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )
	endif ()
endif ()


