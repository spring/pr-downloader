add_library(Plasma STATIC
	PlasmaDownloader.cpp
)

target_link_libraries(Plasma
	${WIN32LIBS}
	soap
)

