add_library(Widget  STATIC
	"WidgetDownloader.cpp"
	"Widget.cpp"
)
target_link_libraries(Widget
	pugixml
)
