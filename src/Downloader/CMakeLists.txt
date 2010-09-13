cmake_minimum_required(VERSION 2.6)

set("downloaders"
	"Http"
	"Plasma"
	"Rapid"
	"Torrent"
	"Widget"
)

subdirs( ${downloaders} )

add_library("downloader" IDownloader.cpp)
target_link_libraries("downloader" ${downloaders})
