#ifndef REPO_H
#define REPO_H


#include <list>
class CSdp;
class CRapidDownloader;

class CRepo{
	std::string repourl;
	 CRapidDownloader* rapid;
public:
	CRepo(const std::string& repourl, CRapidDownloader* rapid){
		this->repourl=repourl;
		this->rapid=rapid;
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
private:
	bool parse();
	std::list<CSdp*> sdps;
	std::string tmpFile;
};

#endif
