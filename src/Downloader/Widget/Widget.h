#include <string>

class CWidget{
public:
	CWidget(const std::string& filename);
private:
	std::string name;
	std::string changelog;
	std::string author;
};
