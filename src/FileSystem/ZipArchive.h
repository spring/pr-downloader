/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#ifndef _ZIP_ARCHIVE_H
#define _ZIP_ARCHIVE_H

#include "minizip/unzip.h"

#include <string>
#include <vector>
#include "IArchive.h"

/**
 * A zip compressed, single-file archive.
 */
class CZipArchive : public IArchive
{
public:
	explicit CZipArchive(const std::string& archiveName);
	virtual ~CZipArchive();

	virtual bool IsOpen() const;

	virtual unsigned int NumFiles() const;
	virtual void FileInfo(unsigned int fid, std::string& name, int& size,
			      int& mode) const;
	virtual unsigned int GetCrc32(unsigned int fid);
	bool GetFile(unsigned int fid, std::vector<unsigned char>& buffer);

protected:
	unzFile zip;

	struct FileData
	{
		unz_file_pos fp;
		int size;
		std::string origName;
		unsigned int crc;
		int mode;
	};
	std::vector<FileData> fileData;

	//	virtual bool GetFileImpl(unsigned int fid, std::vector<boost::uint8_t>&
	// buffer);
};

#endif // _ZIP_ARCHIVE_H
