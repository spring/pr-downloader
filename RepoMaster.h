#ifndef REPO_MASTER_H
#define REPO_MASTER_H
#include <string>
#include <list>
#include <stdio.h>

class CRepo;

class CRepoMaster{
	static CRepoMaster* singleton;
	std::string tmpFile;
	std::string url;
public:
	static void Initialize(std::string& masterurl);
	static void Shutdown();
	static CRepoMaster* GetInstance(){
		return singleton;
	}
	void download(const std::string& name);
/** *
	parses a rep master-file
*/
	void parse();
	void updateRepos();
	CRepoMaster(std::string url);
private:
	std::list<CRepo*> repos;

};
#define repoMaster CRepoMaster::GetInstance()

#endif
