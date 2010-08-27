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

	std::string* getStrByIdx(std::string& str, char c, int idx){
		unsigned int i=0;
		std::string* tmp=new std::string;
		if (idx==0){
			for(i=0;i<str.size();i++){
				if (str[i]==c)
					break;
			}
			tmp->assign(str.substr(0,i));
			return tmp;
		}
		int start=0;
		int end=0;
		int count=0;
		for(i=0;i<str.length();i++){
			if (str[i]==c){
				count++;
				if(count>=idx){
					if(start==0)
						start=i+1;
					else{
						end=i;
						break;
					}
				}
			}
		}
		tmp->assign(str.substr(start,end-start));
		return tmp;
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
