/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "../IDownloader.h"
#define WIDGET_RECHECK_TIME 600

class CWidgetDownloader: public IDownloader
{
public:
	CWidgetDownloader() {};
	~CWidgetDownloader() {};
	bool download(IDownload* download);
	bool search(std::list<IDownload*>& result, const std::string& name, IDownload::category=IDownload::CAT_NONE);

};
