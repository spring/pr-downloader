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


class CFileSystem
{
	static CFileSystem* singleton;

public:
	static CFileSystem* GetInstance();
	static void Initialize();
	static void Shutdown();

	//FIXME: maybe not portable?
	struct FileData {
		std::string name;
		unsigned char md5[16];
		unsigned int crc32;
		unsigned int size;
		unsigned int compsize; //compressed size
		bool download;
	};
	CFileSystem();
	~CFileSystem();
	/**
		parses the file for a mod and creates
	*/
	bool parseSdp(const std::string& filename, std::list<CFileSystem::FileData>& files);	//Validates a File, (checks the md5)
	bool fileIsValid(const FileData& mod, const std::string& filename) const;
	/**
		returns a temporary file name, file is deleted in destructor if not moved away
	*/
	std::string createTempFile();

	/**
		returns the spring writeable directory
	*/
	const std::string& getSpringDir() const;

	/**
		checks if a directory exists
	*/
	bool directoryExists(const std::string& path) const;

	/**
		creates directory if it doesn't exist, expects PATH_DELIMETER at the end of the path
		creates a directory with all subdirectorys (doesn't handle c:\ ...)
	*/
	void createSubdirs(const std::string& path) const;

	const std::string getPoolFileName(const CFileSystem::FileData& fdata) const;
	/**
		Validate all files in /pool/ (check md5)
		@return count of valid files found
	*/
	int validatePool(const std::string& path);

	/**
		check if file is older then secs, returns true if file is older or something goes wrong
	*/
	bool isOlder(const std::string& filename, int secs);
	/**
		check if a file is readable
	*/
	bool fileExists(const std::string& filename);
private:
	std::list<std::string> tmpfiles;
	std::list<FileData> mods;
	bool parse_repository_line(char* str, SRepository* repository, int size);
	std::string springdir;
};

#define fileSystem CFileSystem::GetInstance()

#ifdef WIN32
#define PATH_DELIMITER '\\'
#else
#define PATH_DELIMITER '/'
#endif

#endif
