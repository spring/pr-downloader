#include <list>
class CSdp;

class CRepo:public IRapidRepo{
	std::string repourl;
public:

	CRepo(const std::string& repourl):
		IRapidRepo(repourl){
		this->repourl=repourl;
	}
	void download(const std::string& url);
	void parse();
private:
	std::list<CSdp*> sdps;
	std::string tmpFile;
};
