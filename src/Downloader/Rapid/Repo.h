#ifndef REPO_H
#define REPO_H


#include <list>
class CSdp;

class CRepo{
	std::string repourl;
public:
	CRepo(const std::string& repourl){
		this->repourl=repourl;
	}

	/**
		downloads a repo
	*/
	void download();

	/**
	parse a repo file (versions.gz)
	a line looks like
	nota:revision:1,52a86b5de454a39db2546017c2e6948d,,NOTA test-1

	<tag>,<md5>,<depends on (descriptive name)>,<descriptive name>
	*/
	void parse();
private:
	std::list<CSdp*> sdps;
	std::string tmpFile;
};

#endif
