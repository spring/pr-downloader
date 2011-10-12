add_library(Widget
	"WidgetDownloader.cpp"
	"Widget.cpp"
)
target_link_libraries(Widget
	pugixml
)
