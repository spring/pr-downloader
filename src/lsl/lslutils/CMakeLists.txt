
set(libSpringLobbyUtilsSrc
	"${CMAKE_CURRENT_SOURCE_DIR}/misc.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/config.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/thread.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/globalsmanager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/conversion.cpp"
)

file( GLOB RECURSE libSpringLobbyUtilsHeader "${CMAKE_CURRENT_SOURCE_DIR}/*.h" )

#grep -lR cc\"$ src/* | sed -e "s/\.h/\.cc/g"
set(templatesources "")
list(APPEND libSpringLobbyUtilsHeader ${templatesources})
set_source_files_properties(${libSpringLobbyUtilsHeader} PROPERTIES HEADER_FILE_ONLY 1)
link_directories(${Boost_LIBRARY_DIRS})
add_library(lsl-utils STATIC ${libSpringLobbyHeader} ${libSpringLobbyUtilsSrc})
target_link_libraries(lsl-utils ${Boost_THREAD_LIBRARY} pr-md5)
target_include_directories(lsl-utils
	PRIVATE
		${Boost_INCLUDE_DIRS}
		${pr-downloader_SOURCE_DIR}/src/lsl
)
if(WIN32)
	#see https://github.com/mxe/mxe/issues/1448
	target_compile_definitions(lsl-utils
		PRIVATE
			_WIN32_WINNT=0x0501
			BOOST_THREAD_USE_LIB
			MINGW_HAS_SECURE_API=1)
endif()


