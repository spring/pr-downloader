/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#include "SevenZipArchive.h"

#include <algorithm>
#include <stdexcept>
#include <string.h> //memcpy

extern "C" {
#include "lib/7z/Types.h"
#include "lib/7z/Archive/7z/7zAlloc.h"
#include "lib/7z/Archive/7z/7zExtract.h"
#include "lib/7z/7zCrc.h"
}

#include "Logger.h"


CSevenZipArchive::CSevenZipArchive(const std::string& name)
	: IArchive(name)
{
	blockIndex = 0xFFFFFFFF;
	outBuffer = NULL;
	outBufferSize = 0;

	allocImp.Alloc = SzAlloc;
	allocImp.Free = SzFree;

	allocTempImp.Alloc = SzAllocTemp;
	allocTempImp.Free = SzFreeTemp;

	SzArEx_Init(&db);

	WRes wres = InFile_Open(&archiveStream.file, name.c_str());
	if (wres) {
		LOG_ERROR("Error opening %s %s", name.c_str(), strerror(wres));
		return;
	}

	FileInStream_CreateVTable(&archiveStream);
	LookToRead_CreateVTable(&lookStream, False);

	lookStream.realStream = &archiveStream.s;
	LookToRead_Init(&lookStream);

	CrcGenerateTable();

	SRes res = SzArEx_Open(&db, &lookStream.s, &allocImp, &allocTempImp);
	if (res == SZ_OK) {
		isOpen = true;
	} else {
		isOpen = false;
		std::string error;
		switch (res) {
		case SZ_ERROR_FAIL:
			error = "Extracting failed";
			break;
		case SZ_ERROR_CRC:
			error = "CRC error (archive corrupted?)";
			break;
		case SZ_ERROR_INPUT_EOF:
			error = "Unexpected end of file (truncated?)";
			break;
		case SZ_ERROR_MEM:
			error = "Out of memory";
			break;
		case SZ_ERROR_UNSUPPORTED:
			error = "Unsupported archive";
			break;
		case SZ_ERROR_NO_ARCHIVE:
			error = "Archive not found";
			break;
		default:
			error = "Unknown error";
			break;
		}
		LOG_ERROR("Error opening %s: %s", name.c_str(), error.c_str());
		return;
	}

	// In 7zip talk, folders are pack-units (solid blocks),
	// not related to file-system folders.
	UInt64* folderUnpackSizes = new UInt64[db.db.NumFolders];
	for (unsigned int fi = 0; fi < db.db.NumFolders; fi++) {
		folderUnpackSizes[fi] = SzFolder_GetUnpackSize(db.db.Folders + fi);
	}

	// Get contents of archive and store name->int mapping
	for (unsigned int i = 0; i < db.db.NumFiles; ++i) {
		CSzFileItem* f = db.db.Files + i;
		if (!f->IsDir) {
			std::string fileName = f->Name;

			FileData fd;
			fd.origName = fileName;
			fd.fp = i;
			fd.size = f->Size;
			fd.crc = (f->Size > 0) ? f->FileCRC : 0;
			const UInt32 folderIndex = db.FileIndexToFolderIndexMap[i];
			if (folderIndex == ((UInt32)-1)) {
				// file has no folder assigned
				fd.unpackedSize = f->Size;
				fd.packedSize   = f->Size;
			} else {
				fd.unpackedSize = folderUnpackSizes[folderIndex];
				fd.packedSize   = db.db.PackSizes[folderIndex];
			}

//			StringToLowerInPlace(fileName);
			fileData.push_back(fd);
//			lcNameIndex[fileName] = fileData.size()-1;
		}
	}

	delete [] folderUnpackSizes;
}


CSevenZipArchive::~CSevenZipArchive()
{
	if (outBuffer) {
		IAlloc_Free(&allocImp, outBuffer);
	}
	if (isOpen) {
		File_Close(&archiveStream.file);
	}
	SzArEx_Free(&db, &allocImp);
}

unsigned int CSevenZipArchive::NumFiles() const
{
	return fileData.size();
}

bool CSevenZipArchive::GetFile(unsigned int fid, std::vector<unsigned char>& buffer)
{

	// Get 7zip to decompress it
	size_t offset;
	size_t outSizeProcessed;
	SRes res;

	res = SzAr_Extract(&db, &lookStream.s, fileData[fid].fp, &blockIndex, &outBuffer, &outBufferSize, &offset, &outSizeProcessed, &allocImp, &allocTempImp);
	if (res == SZ_OK) {
		buffer.resize(outSizeProcessed);
		memcpy(&buffer[0], (char*)outBuffer+offset, outSizeProcessed);
		return true;
	} else {
		return false;
	}
}

void CSevenZipArchive::FileInfo(unsigned int fid, std::string& name, int& size) const
{
	name = fileData[fid].origName;
	size = fileData[fid].size;
}

unsigned int CSevenZipArchive::GetCrc32(unsigned int fid)
{
	return fileData[fid].crc;
}
