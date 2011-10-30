/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef REPO_MASTER_H
#define REPO_MASTER_H

#include <string>
#include <list>
#include <stdio.h>

class CRepo;
class CRapidDownloader;

class CRepoMaster
{
	CRapidDownloader* rapid;
public:
	CRepoMaster(const std::string& masterurl, CRapidDownloader* rapid);
	~CRepoMaster();
	/**
		parses a rep master-file
	*/
	void updateRepos();
private:
	bool parse();
	void download(const std::string& name);
	std::string path;
	std::string url;
	std::list<CRepo> repos;

};

#endif
