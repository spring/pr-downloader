#ifndef REPO_MASTER_H
#define REPO_MASTER_H
#include <string>
#include <list>
#include <stdio.h>

class CRepo;

class IRapidRepo{
protected:
	std::string url;
public:
	IRapidRepo(std::string url){
		this->url=url;
	}
	//starts the download
	virtual void download(const std::string& name)=0;
	virtual void parse()=0;
	const std::string& getUrl(){
		return url;
	}

};

class CRepoMaster:IRapidRepo{
	static CRepoMaster* singleton;
	std::string tmpFile;
public:
	static void Initialize(std::string& masterurl);
	static CRepoMaster* GetInstance(){
		return singleton;
	}
	CRepoMaster(std::string& masterurl):IRapidRepo(masterurl){}

	void download(const std::string& name);
/** *
	parses a rep master-file
*/
	void parse();
	void updateRepos();
private:
	std::list<CRepo*> repos;

};
#define repoMaster CRepoMaster::GetInstance()

class CMod:IRapidRepo{
	void download(){
	}
};

#endif
