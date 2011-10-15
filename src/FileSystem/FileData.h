class IHash;
#include <string>
//FIXME: maybe not portable?
class FileData {
	public:
		FileData();
		~FileData();

		std::string name;
		IHash* crc32;
		IHash* md5;
		unsigned int size;
		unsigned int compsize; //compressed size
		bool download;
};

