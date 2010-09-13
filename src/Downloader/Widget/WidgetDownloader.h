#include "../IDownloader.h"
#define WIDGET_RECHECK_TIME 600

class CWidgetDownloader: public IDownloader{
public:
	CWidgetDownloader(){};
	~CWidgetDownloader(){};
	void start(IDownload* download = NULL);
	const IDownload* addDownload(const std::string& url, const std::string& filename="");
	bool removeDownload(IDownload& download);
	std::list<IDownload>* search(const std::string& name="");
};
