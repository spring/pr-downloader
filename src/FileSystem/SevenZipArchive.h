/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#ifndef _7ZIP_ARCHIVE_H
#define _7ZIP_ARCHIVE_H

extern "C" {
#include "lib/7z/7zFile.h"
#include "lib/7z/7z.h"
};

#include <vector>
#include <string>
#include "IArchive.h"

/**
 * An LZMA/7zip compressed, single-file archive.
 */
class CSevenZipArchive: public IArchive
{
public:
	CSevenZipArchive(const std::string& name);
	virtual ~CSevenZipArchive();

	virtual unsigned int NumFiles() const;
	virtual bool GetFile(unsigned int fid, std::vector<unsigned char>& buffer);
	virtual void FileInfo(unsigned int fid, std::string& name, int& size) const;
	virtual unsigned GetCrc32(unsigned int fid);

private:
	UInt32 blockIndex;
	Byte* outBuffer;
	size_t outBufferSize;

	struct FileData {
		int fp;
		/**
		 * Real/unpacked size of the file in bytes.
		 * @see #unpackedSize
		 * @see #packedSize
		 */
		int size;
		std::string origName;
		unsigned int crc;
		/**
		 * How many bytes of files have to be unpacked to get to this file.
		 * This either equal to size, or is larger, if there are other files
		 * in the same solid block.
		 * @see #size
		 * @see #packedSize
		 */
		int unpackedSize;
		/**
		 * How many bytes of the archive have to be read
		 * from disc to get to this file.
		 * This may be smaller or larger then size,
		 * and is smaller then or equal to unpackedSize.
		 * @see #size
		 * @see #unpackedSize
		 */
		int packedSize;
		/**
		 * file mode
		 */
		int mode;
	};
	int GetFileName(const CSzArEx* db, int i);
	const char* GetErrorStr(int res);

	std::vector<FileData> fileData;
	UInt16 *tempBuf;
	int tempBufSize;

	CFileInStream archiveStream;
	CSzArEx db;
	CLookToRead lookStream;
	ISzAlloc allocImp;
	ISzAlloc allocTempImp;

	bool isOpen;
};

#endif // _7ZIP_ARCHIVE_H
