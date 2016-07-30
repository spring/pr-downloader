
SET(libSpringLobbyUtilsSrc
	"${CMAKE_CURRENT_SOURCE_DIR}/misc.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/config.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/crc.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/thread.cpp" 
	"${CMAKE_CURRENT_SOURCE_DIR}/net.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/globalsmanager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/md5.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/conversion.cpp"
	)
	
FILE( GLOB RECURSE libSpringLobbyUtilsHeader "${CMAKE_CURRENT_SOURCE_DIR}/*.h" )

#grep -lR cc\"$ src/* | sed -e "s/\.h/\.cc/g"
SET( templatesources
	""
)
LIST( APPEND libSpringLobbyUtilsHeader ${templatesources} )
set_source_files_properties(  ${libSpringLobbyUtilsHeader} PROPERTIES HEADER_FILE_ONLY 1 )
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
if(WIN32)
	#see https://github.com/mxe/mxe/issues/1448
	add_definitions(
			-D_WIN32_WINNT=0x0501
			-DBOOST_THREAD_USE_LIB
			-DMINGW_HAS_SECURE_API=1
		)
endif()
ADD_LIBRARY(lsl-utils STATIC ${libSpringLobbyHeader} ${libSpringLobbyUtilsSrc} )
TARGET_LINK_LIBRARIES(lsl-utils ${Boost_THREAD_LIBRARY})
target_include_directories(lsl-utils
		PRIVATE ${Boost_INCLUDE_DIRS}
		PRIVATE ${libSpringLobby_SOURCE_DIR}/src
	)


