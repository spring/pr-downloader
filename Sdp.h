#include "RepoMaster.h"

class CSdp{
public:
	CSdp(const std::string& shortname, const std::string& md5, const std::string& name, const std::string& url);
	void download();
	void parse();
	//returns md5 of a repo
	const std::string& getMD5();
	//returns the descriptional name
	const std::string& getName();
	//returns the shortname, for example ba:stable
	const std::string& getShortName();
private:
	std::string name;
	std::string md5;
	std::string shortname;
	std::string url;
	std::string filename;
	bool downloaded;
};
