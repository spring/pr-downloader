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
	static void Shutdown();
	CHttpDownload();
	~CHttpDownload();

	/**
		downloads a file from Url to filename
	*/
	bool download(const std::string& Url, const std::string& filename, int pos=1);
	void setCount(unsigned int count);
	unsigned int getCount();
	const std::string& getCacheFile(const std::string &url);
private:
	CURL *curl;
	int count;

};


#define httpDownload CHttpDownload::GetInstance()

#endif
