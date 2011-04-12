add_library(Widget
	"WidgetDownloader.cpp"
	"Widget.cpp"
	"pugixml/pugixml.cpp"
	"pugixml/pugixpath.cpp"
)
FILE( GLOB_RECURSE header "*.h" )
INSTALL (FILES ${header} DESTINATION ${INCLUDE_DIR}/Downloader/Widget )