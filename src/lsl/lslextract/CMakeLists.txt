
add_executable(lslextract
	lslextract.cpp
	server/server.cpp
	server/connection.cpp
	server/connection_manager.cpp
	server/request_handler.cpp
	server/request_parser.cpp
	server/reply.cpp
	server/mime_types.cpp
	${pr-downloader_SOURCE_DIR}/src/Logger.cpp
)
find_package(PNG REQUIRED)
find_package(X11 REQUIRED)

remove_definitions(-DHAVE_WX -D__WXDEBUG__ -D__WXGTK__ -DHAVE_SPRINGLOBBY=1 -DHAVE_CONFIG_H  -DHAVE_LIBNOTIFY)
target_link_libraries(lslextract
	lsl-unitsync
	lsl-utils
	Downloader
	${Boost_LIBRARIES}
	${PNG_LIBRARY}
	${X11_LIBRARIES}
	${CMAKE_DL_LIBS}
	${LSL_JSONCPP_LIBRARIES}
)

target_include_directories(lslextract
	PRIVATE
		${pr-downloader_SOURCE_DIR}/src/lsl
		${PRD_JSONCPP_INCLUDE_DIR}
)

