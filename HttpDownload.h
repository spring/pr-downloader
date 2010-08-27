#ifndef HTTP_DOWNLOAD_H
#define HTTP_DOWNLOAD_H

#include <string>

class CHttpDownload{
	static CHttpDownload* singleton;

public:
	static inline CHttpDownload* GetInstance() {
		return singleton;
	}
	static void Initialize();
	CHttpDownload();
	bool download(const std::string& Url,const std::string& filename);
private:

};


#define httpDownload CHttpDownload::GetInstance()

#endif
