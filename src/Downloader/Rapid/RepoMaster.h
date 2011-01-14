#ifndef REPO_MASTER_H
#define REPO_MASTER_H


#include <string>
#include <list>
#include <stdio.h>

class CRepo;
class CRapidDownloader;

class CRepoMaster{
	std::string path;
	std::string url;
	CRapidDownloader* rapid;
public:
	CRepoMaster(std::string& masterurl, CRapidDownloader* rapid);
	~CRepoMaster();
	void download(const std::string& name);
	/**
		parses a rep master-file
	*/
	bool parse();
	void updateRepos();
private:
	std::list<CRepo*> repos;

};

#endif
