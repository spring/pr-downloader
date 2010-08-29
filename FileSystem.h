
#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#ifndef WIN32
#include <stdlib.h>
#else
#include <windows.h>
#endif

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
	//parses a sdp
	std::list<CFileSystem::FileData*>* parseSdp(std::string& filename);
	//Validates a File, (checks the md5)
	bool fileIsValid(FileData* mod, std::string& filename);
	//returns a temporary file name, file is deleted in destructor if not moved away
	std::string createTempFile();
	//returns the spring writeable directory
	const std::string& getSpringDir() const;
	bool directory_exist(const std::string& path);
	void create_subdirs (const std::string& path);

private:
	std::list<std::string> tmpfiles;
	std::list<FileData> mods;
	bool parse_repository_line(char* str, SRepository* repository, int size);
	std::string springdir;
};

#define fileSystem CFileSystem::GetInstance()

#endif
