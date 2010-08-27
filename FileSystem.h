
#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stdlib.h>
#include <list>
#include <string>

class SRepository;
class CRepo;


class CFileSystem{
	static CFileSystem* singleton;

public:
	static inline CFileSystem* GetInstance() {
		return singleton;
	}
	static void Initialize();
	static void Shutdown();

	//FIXME: maybe not portable?
	struct FileData{
		std::string name;
		unsigned char md5[16];
		unsigned int crc32;
		unsigned int size;
	};
	CFileSystem();
	~CFileSystem();
	std::list<CFileSystem::FileData*>* parseSdp(std::string& filename);
	bool fileIsValid(FileData* mod, std::string& filename);
	const std::string* createTempFile();
	CRepo* parse_master_file(const std::string& filename);
	const std::string& getSpringDir();

private:
	std::list<std::string*> tmpfiles;
	std::list<FileData> mods;
	bool parse_repository_line(char* str, SRepository* repository, int size);
	std::string springdir;
};

#define fileSystem CFileSystem::GetInstance()

#endif
