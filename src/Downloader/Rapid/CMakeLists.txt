add_library(Rapid
	RapidDownloader.cpp
	Repo.cpp
	RepoMaster.cpp
	Sdp.cpp
)
FILE( GLOB_RECURSE header "*.h" )
INSTALL (FILES ${header} DESTINATION ${INCLUDE_DIR}/Downloader/Rapid )