#ifndef PLASMA_DOWNLOADER_H
#define PLASMA_DOWNLOADER_H

#include "../IDownloader.h"


class CPlasmaDownloader: public IDownloader {
public:
	CPlasmaDownloader();
	virtual std::list<IDownload>* search(const std::string& name, IDownload::category cat=IDownload::CAT_NONE);
	virtual bool download(IDownload& download);

private:
	std::string torrentPath;
};

#endif
