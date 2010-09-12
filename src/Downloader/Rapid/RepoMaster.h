#ifndef REPO_MASTER_H
#define REPO_MASTER_H


#include <string>
#include <list>
#include <stdio.h>

class CRepo;

class CRepoMaster{
	std::string path;
	std::string url;
public:
	CRepoMaster(std::string& masterurl);
	~CRepoMaster();
	void download(const std::string& name);
/**
	parses a rep master-file
*/
	void parse();
	void updateRepos();
private:
	std::list<CRepo*> repos;

};

#endif
