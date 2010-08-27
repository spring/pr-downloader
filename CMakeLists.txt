
cmake_minimum_required(VERSION 2.6)
project(rapid)
Include(FindCURL)
add_executable(rapid
FileSystem.cpp
main.cpp
RapidDownloader.cpp
RepoMaster.cpp
Util.cpp
HttpDownload.cpp
md5.cpp
Repo.cpp
Sdp.cpp
)
target_link_libraries(rapid ${CURL_LIBRARIES})
