#ifndef HTTP_DOWNLOAD_H
#define HTTP_DOWNLOAD_H

#include <string>
#include <curl/curl.h>


class CHttpDownload{
	static CHttpDownload* singleton;

public:
	static inline CHttpDownload* GetInstance() {
		return singleton;
	}
	static void Initialize();
	CHttpDownload();
	~CHttpDownload();

	//downloads a file from Url to filename
	bool download(const std::string& Url,const std::string& filename);
private:
	CURL *curl;

};


#define httpDownload CHttpDownload::GetInstance()

#endif
