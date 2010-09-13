
set("downloaders"
	"Http"
	"Plasma"
	"Rapid"
	"Torrent"
	"Widget"
)

subdirs( ${downloaders} )

add_library("downloader" SHARED IDownloader.cpp)
target_link_libraries("downloader" ${downloaders})

