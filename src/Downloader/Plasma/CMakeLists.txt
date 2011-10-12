add_library(Plasma
	PlasmaDownloader.cpp
)

target_link_libraries(Plasma
	${WIN32LIBS}
	soap
)

