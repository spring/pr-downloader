#include "../IDownloader.h"
#define WIDGET_RECHECK_TIME 600

class CWidgetDownloader: public IDownloader{
public:
	CWidgetDownloader(){};
	~CWidgetDownloader(){};
	bool download(IDownload& download);
	std::list<IDownload>* search(const std::string& name, IDownload::category=IDownload::CAT_NONE);

};
