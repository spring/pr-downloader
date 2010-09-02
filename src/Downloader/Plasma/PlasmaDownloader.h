#ifndef PLASMA_DOWNLOADER_H
#define PLASMA_DOWNLOADER_H

#include "Downloader/IDownloader.h"


class CPlasmaDownloader: public IDownloader {
public:
	CPlasmaDownloader();
	void download(const std::string& name);
	void start(IDownload* download = NULL);
	const IDownload* addDownload(const std::string& url, const std::string& filename="");
	bool removeDownload(IDownload& download);
	std::list<IDownload>* search(const std::string& name);
};

#endif
