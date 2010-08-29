#include <list>
class CSdp;

class CRepo{
	std::string repourl;
public:
	CRepo(const std::string& repourl){
		this->repourl=repourl;
	}

	//downloads a repo
	void download();
	//parse the downloaded repo
	void parse();
private:
	std::list<CSdp*> sdps;
	std::string tmpFile;
};
