cmake_minimum_required(VERSION 2.6)

set("downloaders" "Plasma" "Rapid" "Torrent" "Http")

subdirs( ${downloaders} )

add_library("downloader" IDownloader.cpp)
target_link_libraries("downloader" ${downloaders})
