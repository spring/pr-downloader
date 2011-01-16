#include "HttpDownloader.h"
#include <stdio.h>
#include <curl/curl.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <zlib.h>
#include <xmlrpc-c/girerr.hpp>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/client.hpp> 
#include <xmlrpc-c/client_transport.hpp> 

#include "../../Util.h"


/** *
	draw a nice download status-bar
*/
int progress_func(CHttpDownloader* ptr, double TotalToDownload, double NowDownloaded,
				  double TotalToUpload, double NowUploaded) {
	// how wide you want the progress meter to be
	int totaldotz=40;
	double fractiondownloaded;
	if (TotalToDownload>0)
		fractiondownloaded = NowDownloaded / TotalToDownload;
	else
		fractiondownloaded=0;
	// part of the progressmeter that's already "full"
	int dotz = fractiondownloaded * totaldotz;

	// create the "meter"
	printf("%5d/%5d ", ptr->getStatsPos(),ptr->getCount());
	printf("%3.0f%% [",fractiondownloaded*100);
	int ii=0;
	// part  that's full already
	for ( ; ii < dotz;ii++) {
		printf("=");
	}
	// remaining part (spaces)
	for ( ; ii < totaldotz;ii++) {
		printf(" ");
	}
	// and back to line begin - do not forget the fflush to avoid output buffering problems!
	printf("] %d/%d\r",(int)NowDownloaded,(int)TotalToDownload );
	fflush(stdout);
	return 0;
}


size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	int written = fwrite(ptr, size, nmemb, stream);
	return written;
}

CHttpDownloader::CHttpDownloader() {
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_func);
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	stats_filepos=1;
	stats_count=1;
}

CHttpDownloader::~CHttpDownloader() {
	curl_easy_cleanup(curl);
	curl=NULL;
}

void CHttpDownloader::setCount(unsigned int count) {
	this->stats_count=count;
}

unsigned int CHttpDownloader::getCount() {
	return this->stats_count;
}

unsigned int CHttpDownloader::getStatsPos() {
	return this->stats_filepos;
}

void CHttpDownloader::setStatsPos(unsigned int pos) {
	this->stats_filepos=pos;
}

std::list<IDownload>* CHttpDownloader::search(const std::string& name, IDownload::category cat) {
	DEBUG_LINE("%s", name.c_str()  );
	std::list<IDownload>* res;
	res=new std::list<IDownload>();

	const std::string serverUrl("http://new.springfiles.com/xmlrpc.php");
	const std::string method("springfiles.search");
	std::string category;
	std::string filename=fileSystem->getSpringDir();
	filename+=PATH_DELIMITER;
	switch(cat){
		case IDownload::CAT_MAPS:
			category="map";
			filename+="maps";
			break;
		case IDownload::CAT_MODS:
			category="game";
			filename+="games";
			break;
		default:{
			category="%";
		}
	}
	filename+=PATH_DELIMITER;

	std::map<std::string, xmlrpc_c::value> array;
	std::pair<std::string, xmlrpc_c::value> member("filename",xmlrpc_c::value_string(name));
	std::pair<std::string, xmlrpc_c::value> member2("category",xmlrpc_c::value_string(category));

	array.insert(member2);
	array.insert(member);

	xmlrpc_c::paramList params;
	params.add(xmlrpc_c::value_struct(array));

	xmlrpc_c::rpcOutcome result;

	xmlrpc_c::rpc rpcClient(method, params);
	xmlrpc_c::carriageParm_http0 myParm(serverUrl);

	xmlrpc_c::clientXmlTransportPtr transportP(xmlrpc_c::clientXmlTransport_http::create());
        xmlrpc_c::carriageParm_http0 carriageParm0(serverUrl);
        xmlrpc_c::client_xml client0(transportP);

	client0.call(&carriageParm0, method, params, &result);
	if(!result.succeeded()){
		printf("result: %d", result.getFault().getCode());
		return res;
	}
	if (result.getResult().type()!=xmlrpc_c::value::TYPE_STRUCT){
		return res;
	}
	const xmlrpc_c::value_struct value(result.getResult());
	std::map<std::string, xmlrpc_c::value> resultMap(static_cast<std::map<std::string, xmlrpc_c::value> >(value));

	std::string resMd5=xmlrpc_c::value_string(static_cast<xmlrpc_c::value>(resultMap["md5"]));
	std::string resFilename=xmlrpc_c::value_string(static_cast<xmlrpc_c::value>(resultMap["filename"]));


	xmlrpc_c::value_array mirrorValues=xmlrpc_c::value_array(static_cast<xmlrpc_c::value>(resultMap["mirrors"]));

	std::vector<xmlrpc_c::value> mirrorMap=mirrorValues.vectorValueValue();
	filename+=resFilename;
	if (mirrorMap.empty()){
		return res;
	}
	std::string mirror = xmlrpc_c::value_string(*mirrorMap.begin());
	IDownload dl(mirror,filename,cat);
	std::vector<xmlrpc_c::value>::iterator it;
	for(it=mirrorMap.begin(); it!=mirrorMap.end(); ++it){
		mirror = xmlrpc_c::value_string((*it));
		printf("mirror: %s\n",mirror.c_str());
		dl.addMirror(mirror);
	}
	res->push_back((IDownload&)dl);
	DEBUG_LINE("md5 %s\n", resMd5.c_str());
	DEBUG_LINE("filename %s\n", resFilename.c_str());
	return res;
}

bool CHttpDownloader::download(IDownload& download) {
	DEBUG_LINE("%s",download.name.c_str());

	CURLcode res=CURLE_OK;
	printf("Downloading %s to %s\n",download.url.c_str(), download.name.c_str());
	//FIXME: use etag/timestamp as remote file could be modified
	/*
		if (fileSystem->fileExists(download.name)){
			//TODO: validate file
			printf("file already downloaded: \"%s\"\n",download.name.c_str());
			return true;
		}
	*/

	if (!curl) {
		printf("Error initializing curl");
		return false;
	}
	FILE* fp = fopen(download.name.c_str() ,"wb+");
	if (fp==NULL) {
		printf("CHttpDownloader:: Could not open %s\n",download.name.c_str());
		return false;
	}
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA ,this);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_URL, download.url.c_str());
	res = curl_easy_perform(curl);
	fclose(fp);
	printf("\n"); //new line because of downloadbar
	if (res!=0) {
		printf("error downloading %s\n",download.url.c_str());
		unlink(download.name.c_str());
		return false;
	}
	return true;
}
