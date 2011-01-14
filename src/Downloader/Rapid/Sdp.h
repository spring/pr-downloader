#ifndef _SDP_H
#define _SDP_H


#include "RepoMaster.h"
#include "../../FileSystem.h"
#include <string.h>

#define LENGTH_SIZE 4

class CSdp{
public:
	CSdp(const std::string& shortname, const std::string& md5, const std::string& name, const std::string& depends, const std::string& url){
		this->shortname=shortname;
		this->name=name;
		this->md5=md5;
		this->url=url;
		this->downloaded=false;
		this->depends=depends;
		this->file_handle=NULL;
		this->downlooadInitialized=false;
		this->cursize=0;
		memset(this->cursize_buf,LENGTH_SIZE,0);
		this->skipped=false;
		this->file_pos=0;
	}
	/**
		download a mod, we already know the host where to download from + the md5 of the sdp file
		we have to download the sdp + parse it + download associated files
	*/
	bool download();
	/**
		returns md5 of a repo
	*/
	const std::string& getMD5(){
		return md5;
	}
	/**
		returns the descriptional name
	*/
	const std::string& getName(){
		return name;
	}
	/**
		returns the shortname, for example ba:stable
	*/
	const std::string& getShortName(){
		return shortname;
	}
	/**
		returns the shortname, for example ba:stable
	*/
	const std::string& getDepends(){
		return depends;
	}

	bool downlooadInitialized;
	std::list<CFileSystem::FileData*>::iterator list_it;
	std::list<CFileSystem::FileData*>* globalFiles;
	FILE* file_handle;
	std::string file_name;

	unsigned int file_pos;
	unsigned int skipped;
	unsigned char cursize_buf[LENGTH_SIZE];
	unsigned int cursize;

private:
	void parse();
	/**
		download files streamed
		streamer.cgi works as follows:
		* The client does a POST to /streamer.cgi?<hex>
		  Where hex = the name of the .sdp
		* The client then sends a gzipped bitarray representing the files
		  it wishes to download. Bitarray is formated in the obvious way,
		  an array of characters where each file in the sdp is represented
		  by the (index mod 8) bit (shifted left) of the (index div 8) byte
		  of the array.
		* streamer.cgi then responds with <big endian encoded int32 length>
		  <data of gzipped pool file> for all files requested. Files in the
		  pool are also gzipped, so there is no need to decompress unless
		  you wish to verify integrity. Note: The filesize here isn't the same
		  as in the .sdp, the sdp-file contains the uncompressed size
		* streamer.cgi also sets the Content-Length header in the reply so
		  you can implement a proper progress bar.

	T 192.168.1.2:33202 -> 94.23.170.70:80 [AP]
	POST /streamer.cgi?652e5bb5028ff4d2fc7fe43a952668a7 HTTP/1.1..Accept-Encodi
	ng: identity..Content-Length: 29..Host: packages.springrts.com..Content-Typ
	e: application/x-www-form-urlencoded..Connection: close..User-Agent: Python
	-urllib/2.6....
	##
	T 192.168.1.2:33202 -> 94.23.170.70:80 [AP]
	......zL..c`..`d.....K.n/....
	*/
	bool downloadStream(std::string url,std::list<CFileSystem::FileData*>& files);
	std::string name;
	std::string md5;
	std::string shortname;
	std::string url;
	std::string filename;
	std::string depends;
	bool downloaded;
};

#endif
