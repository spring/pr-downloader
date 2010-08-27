#include "RepoMaster.h"

//nota:revision:1,52a86b5de454a39db2546017c2e6948d,,NOTA test-1

class CSdp{
public:
	CSdp(const std::string& shortname, const std::string& md5, const std::string& name, const std::string& url);
	void download();
	void parse();
	const std::string& getMD5();
	const std::string& getName();
	const std::string& getShortName();
private:
	std::string name;
	std::string md5;
	std::string shortname;
	std::string url;
	std::string filename;
};
