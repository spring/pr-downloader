#ifndef PLASMA_DOWNLOADER_H
#define PLASMA_DOWNLOADER_H

#include "Downloader/IDownloader.h"


class CPlasmaDownloader: public IDownloader
{
public:
	CPlasmaDownloader();
	virtual bool search(std::list<IDownload*>& result, const std::string& name, IDownload::category cat=IDownload::CAT_NONE);
	virtual bool download(IDownload* download);

private:
	std::string torrentPath;
};

#endif
