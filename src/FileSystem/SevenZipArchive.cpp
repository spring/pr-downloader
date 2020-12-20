/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#include "SevenZipArchive.h"

#include <algorithm>
#include <stdexcept>
#include <string.h> //memcpy

extern "C" {
#include "lib/7z/7zTypes.h"
#include "lib/7z/7zAlloc.h"
#include "lib/7z/7zCrc.h"
}

#include "Logger.h"
#include "Util.h"

static Byte kUtf8Limits[5] = {0xC0, 0xE0, 0xF0, 0xF8, 0xFC};
static bool Utf16_To_Utf8(char* dest, size_t* destLen, const UInt16* src,
			  size_t srcLen)
{
	size_t destPos = 0, srcPos = 0;
	for (;;) {
		unsigned numAdds;
		if (srcPos == srcLen) {
			*destLen = destPos;
			return True;
		}
		UInt32 value = src[srcPos++];
		if (value < 0x80) {
			if (dest)
				dest[destPos] = (char)value;
			destPos++;
			continue;
		}
		if (value >= 0xD800 && value < 0xE000) {
			if (value >= 0xDC00 || srcPos == srcLen)
				break;
			const UInt32 c2 = src[srcPos++];
			if (c2 < 0xDC00 || c2 >= 0xE000)
				break;
			value = (((value - 0xD800) << 10) | (c2 - 0xDC00)) + 0x10000;
		}
		for (numAdds = 1; numAdds < 5; numAdds++)
			if (value < (((UInt32)1) << (numAdds * 5 + 6)))
				break;
		if (dest)
			dest[destPos] =
			    (char)(kUtf8Limits[numAdds - 1] + (value >> (6 * numAdds)));
		destPos++;
		do {
			numAdds--;
			if (dest)
				dest[destPos] = (char)(0x80 + ((value >> (6 * numAdds)) & 0x3F));
			destPos++;
		} while (numAdds != 0);
	}
	*destLen = destPos;
	return false;
}

int CSevenZipArchive::GetFileName(const CSzArEx* db, int i)
{
	const size_t len = SzArEx_GetFileNameUtf16(db, i, nullptr);

	if (len > tempBufSize) {
		SzFree(nullptr, tempBuf);
		tempBufSize = len;
		tempBuf = (UInt16*)SzAlloc(nullptr, tempBufSize * sizeof(tempBuf[0]));
		if (tempBuf == 0) {
			return SZ_ERROR_MEM;
		}
	}
	tempBuf[len - 1] = 0;
	return SzArEx_GetFileNameUtf16(db, i, tempBuf);
}

const char* CSevenZipArchive::GetErrorStr(int res)
{
	switch (res) {
		case SZ_OK:
			return "OK";
		case SZ_ERROR_FAIL:
			return "Extracting failed";
		case SZ_ERROR_CRC:
			return "CRC error (archive corrupted?)";
		case SZ_ERROR_INPUT_EOF:
			return "Unexpected end of file (truncated?)";
		case SZ_ERROR_MEM:
			return "Out of memory";
		case SZ_ERROR_UNSUPPORTED:
			return "Unsupported archive";
		case SZ_ERROR_NO_ARCHIVE:
			return "Archive not found";
	}
	return "Unknown error";
}

CSevenZipArchive::CSevenZipArchive(const std::string& name)
	: IArchive(name)
{
	allocImp.Alloc = SzAlloc;
	allocImp.Free = SzFree;
	allocTempImp.Alloc = SzAllocTemp;
	allocTempImp.Free = SzFreeTemp;
	constexpr const size_t kInputBufSize ((size_t)1 << 18);

	SzArEx_Init(&db);

#ifdef _WIN32
	WRes wres = InFile_OpenW(&archiveStream.file, s2ws(name).c_str());
#else
	WRes wres = InFile_Open(&archiveStream.file, name.c_str());
#endif
	if (wres != 0) {
		LOG_ERROR("Error opening %s %s", name.c_str(), strerror(wres));
		return;
	}

	FileInStream_CreateVTable(&archiveStream);
	LookToRead2_CreateVTable(&lookStream, False);

	lookStream.realStream = &archiveStream.vt;
	LookToRead2_Init(&lookStream);
	lookStream.buf = NULL;
	lookStream.buf = (Byte *)ISzAlloc_Alloc(&allocImp, kInputBufSize);

	CrcGenerateTable();
	SzArEx_Init(&db);

	SRes res = SzArEx_Open(&db, &lookStream.vt, &allocImp, &allocTempImp);
	if (res == SZ_OK) {
		isOpen = true;
	} else {
		isOpen = false;
		LOG_ERROR("Error opening %s: %s", name.c_str(), GetErrorStr(res));
		return;
	}

	// Get contents of archive and store name->int mapping
	for (unsigned int i = 0; i < db.NumFiles; ++i) {
		const bool isDir = SzArEx_IsDir(&db, i);

		if (isDir) {
			continue;
		}

		const int written = GetFileName(&db, i);
		if (written <= 0) {
			LOG_ERROR(
				"Error getting filename in Archive: %s %d, file skipped in %s",
				GetErrorStr(res), res, name.c_str());
			continue;
		}

		char buf[1024];
		size_t dstlen = sizeof(buf);

		Utf16_To_Utf8(buf, &dstlen, tempBuf, written);

		FileData fd;
		fd.origName = buf;
		fd.fp = i;
		fd.size = SzArEx_GetFileSize(&db, i);
		fd.crc = 0; //; (f->Size > 0) ? f->Crc : 0;
		if (SzBitWithVals_Check(&db.Attribs, i)) {
			// LOG_DEBUG("%s %d", fd.origName.c_str(), db.Attribs.Vals[i])
			if (db.Attribs.Vals[i] & 1 << 16)
				fd.mode = 0755;
			else
				fd.mode = 0644;
		}
		fileData.push_back(fd);
	}

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
	SzFree(nullptr, tempBuf);
	ISzAlloc_Free(&allocImp, lookStream.buf);
}

unsigned int CSevenZipArchive::NumFiles() const
{
	return fileData.size();
}

bool CSevenZipArchive::GetFile(unsigned int fid,
			       std::vector<unsigned char>& buffer)
{
	// Get 7zip to decompress it
	size_t offset;
	size_t outSizeProcessed;
	const SRes res = SzArEx_Extract(&db, &lookStream.vt, fileData[fid].fp, &blockIndex,
			     &outBuffer, &outBufferSize, &offset, &outSizeProcessed,
			     &allocImp, &allocTempImp);
	if (res == SZ_OK) {
		buffer.resize(outSizeProcessed);
		memcpy(&buffer[0], (char*)outBuffer + offset, outSizeProcessed);
		return true;
	} else {
		LOG_ERROR("Error extracting %s: %s", fileData[fid].origName.c_str(), GetErrorStr(res));
		return false;
	}
}

void CSevenZipArchive::FileInfo(unsigned int fid, std::string& name, int& size,
				int& mode) const
{
	name = fileData[fid].origName;
	size = fileData[fid].size;
	mode = fileData[fid].mode;
}

unsigned int CSevenZipArchive::GetCrc32(unsigned int fid)
{
	return fileData[fid].crc;
}
