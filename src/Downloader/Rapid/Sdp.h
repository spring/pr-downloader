#ifndef _SDP_H
#define _SDP_H


#include "RepoMaster.h"
#include "../../FileSystem.h"

class CSdp{
public:
	CSdp(const std::string& shortname, const std::string& md5, const std::string& name, const std::string& depends, const std::string& url){
		this->shortname=shortname;
		this->name=name;
		this->md5=md5;
		this->url=url;
		this->downloaded=false;
		this->depends=depends;
	}
	void download();
	void parse();
	//returns md5 of a repo
	const std::string& getMD5(){
		return md5;
	}
	//returns the descriptional name
	const std::string& getName(){
		return name;
	}
	//returns the shortname, for example ba:stable
	const std::string& getShortName(){
		return shortname;
	}
	//returns the shortname, for example ba:stable
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
	unsigned char cursize_buf[4];
	unsigned int cursize;

private:
	void downloadStream(std::string url,std::list<CFileSystem::FileData*>& files);
	std::string name;
	std::string md5;
	std::string shortname;
	std::string url;
	std::string filename;
	std::string depends;
	bool downloaded;
};

#endif
