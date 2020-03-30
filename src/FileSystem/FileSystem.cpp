/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "FileSystem.h"
#include "Util.h"
#include "Downloader/IDownloader.h"
#include "HashMD5.h"
#include "HashSHA1.h"
#include "FileData.h"
#include "Logger.h"
#include "SevenZipArchive.h"
#include "ZipArchive.h"
#include "lib/bencode/bencode.h"

#include <zlib.h>
#include <string.h>
#include <list>
#include <string>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#include <math.h>
#ifndef SHGFP_TYPE_CURRENT
#define SHGFP_TYPE_CURRENT 0
#endif
#else
#include <sys/statvfs.h>
#include <errno.h>
#endif

static CFileSystem* singleton = nullptr;

FILE* CFileSystem::propen(const std::string& filename,
			  const std::string& mode)
{
#ifdef WIN32
	FILE* ret = _wfopen(s2ws(filename).c_str(), s2ws(mode).c_str());
#else
	FILE* ret = fopen(filename.c_str(), mode.c_str());
#endif
	if (ret == nullptr) {
		LOG_ERROR("Couldn't open %s", filename.c_str());
	}
	return ret;
}

bool CFileSystem::fileIsValid(const FileData* mod,
			      const std::string& filename) const
{
	unsigned char data[IO_BUF_SIZE];
	FILE* f = propen(filename, "rb");
	gzFile inFile = gzdopen(fileno(f), "rb");
	if (inFile == nullptr) { // file can't be opened
		fclose(f);
		LOG_ERROR("Could not open file %s", filename.c_str());
		return false;
	}
	HashMD5 md5hash;
	md5hash.Init();
	//	unsigned long filesize=0;
	int bytes;
	while ((bytes = gzread(inFile, data, IO_BUF_SIZE)) > 0) {
		md5hash.Update((char*)data, bytes);
		//		filesize=filesize+bytes;
	}

	md5hash.Final();
	gzclose(inFile);
	fclose(f);
	/*	if (filesize!=mod->size){
                  ERROR("File %s invalid, size wrong: %d but should be %d",
     filename.c_str(),filesize, mod->size);
                  return false;
          }*/
	if (!md5hash.compare(mod->md5, sizeof(mod->md5))) { // file is invalid
		//		ERROR("Damaged file found: %s",filename.c_str());
		//		removeFile(filename.c_str());
		return false;
	}
	return true;
}

std::string getMD5fromFilename(const std::string& path)
{
	const size_t start = path.rfind(PATH_DELIMITER) + 1;
	const size_t end = path.find(".", start);
	return path.substr(start, end-start);
}

bool CFileSystem::parseSdp(const std::string& filename, std::list<FileData>& files)
{
	char c_name[255];
	unsigned char c_md5[16];
	unsigned char c_crc32[4];
	unsigned char c_size[4];
	unsigned char length;

	FILE* f = propen(filename, "rb");
	gzFile in = gzdopen(fileno(f), "rb");
	if (in == Z_NULL) {
		LOG_ERROR("Could not open %s", filename.c_str());
		fclose(f);
		return false;
	}
	files.clear();
	HashMD5 sdpmd5;
	sdpmd5.Init();
	while (true) {

		if (!gzread(in, &length, 1)) {
			if (gzeof(in)) {
				break;
			}
			LOG_ERROR("Unexpected eof in %s", filename.c_str());
			gzclose(in);
			fclose(f);
			return false;
		}
		if (!((gzread(in, &c_name, length)) && (gzread(in, &c_md5, 16)) &&
		      (gzread(in, &c_crc32, 4)) && (gzread(in, &c_size, 4)))) {
			LOG_ERROR("Error reading %s", filename.c_str());
			gzclose(in);
			fclose(f);
			return false;
		}
		FileData fd;
		fd.name = std::string(c_name, length);
		memcpy(fd.md5, &c_md5, 16);
		memcpy(fd.crc32, &c_crc32, 4);
		fd.size = parse_int32(c_size);
		files.push_back(fd);

		HashMD5 nameMd5;
		nameMd5.Init();
		nameMd5.Update(fd.name.data(), fd.name.size());
		nameMd5.Final();
		assert(nameMd5.getSize() == 16);
		assert(sizeof(fd.md5) == 16);
		sdpmd5.Update((const char*)nameMd5.Data(), nameMd5.getSize());
		sdpmd5.Update((const char*)&fd.md5[0], sizeof(fd.md5));
	}
	gzclose(in);
	fclose(f);
	sdpmd5.Final();
	const std::string filehash = getMD5fromFilename(filename);
	if (filehash != sdpmd5.toString()) {
		LOG_ERROR("%s is invalid, deleted (%s vs %s)", filename.c_str(), filehash.c_str(), sdpmd5.toString().c_str());
		removeFile(filename);
		return false;
	}
	LOG_DEBUG("Parsed %s with %d files", filename.c_str(), (int)files.size());
	return true;
}

bool CFileSystem::setWritePath(const std::string& path)
{

	if (!path.empty()) {
		springdir = path;
	} else {
#ifndef WIN32
		const char* buf = getenv("HOME");
		if (buf != nullptr) {
			springdir = buf;
			springdir.append("/.spring");
		} else { // no home: use cwd
			LOG_INFO("HOME isn't set, using CWD./spring");
			springdir = ".spring";
		}
#else
		wchar_t my_documents[MAX_PATH];
		HRESULT result = SHGetFolderPathW(nullptr, CSIDL_PERSONAL, nullptr,
						  SHGFP_TYPE_CURRENT, my_documents);
		if (result == S_OK) {
			springdir = ws2s(my_documents);
		}
		springdir.append("\\My Games\\Spring");
#endif
	}
	if (!springdir.empty()) { // dir has to be without slash at the end
		if (springdir[springdir.length() - 1] == PATH_DELIMITER) {
			springdir = springdir.substr(0, springdir.size() - 1);
		}
	}
	LOG_INFO("Using filesystem-writepath: %s", springdir.c_str());
	return createSubdirs(springdir.c_str());
}

CFileSystem* CFileSystem::GetInstance()
{
	if (singleton == nullptr) {
		singleton = new CFileSystem();
	}
	return singleton;
}

void CFileSystem::Shutdown()
{
	delete singleton;
	singleton = nullptr;
}

const std::string CFileSystem::getSpringDir()
{
	assert(!springdir.empty());
	if (springdir.empty())
		(setWritePath(""));
	return springdir;
}

bool CFileSystem::directoryExists(const std::string& path)
{
	if (path.empty())
		return false;
#ifdef WIN32
	const std::wstring wpath = s2ws(path);
	DWORD dwAttrib = GetFileAttributesW(wpath.c_str());
	return ((dwAttrib != INVALID_FILE_ATTRIBUTES) &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
	struct stat fileinfo;
	const int res = stat(path.c_str(), &fileinfo);
	return (res == 0) && ((fileinfo.st_mode & S_IFDIR) != 0);
#endif
}

bool CreateDir(const std::string& path)
{
	assert(!path.empty());
#ifdef WIN32
	return CreateDirectory(s2ws(path).c_str(), nullptr);
#else
	return mkdir(path.c_str(), 0755) == 0;
#endif
}

bool CFileSystem::createSubdirs(const std::string& path)
{
	assert(!path.empty());
	if (directoryExists(path)) {
		return true;
	}
	for (size_t i = 2; i < path.size(); i++) {
		char c = path.at(i);
#ifdef WIN32
		/* skip for example mkdir(C:\) */
		if ((i == 2) && (c == PATH_DELIMITER))
			continue;
#endif
		if (c != PATH_DELIMITER) {
			continue;
		}

		const std::string tocreate = path.substr(0, i);
		LOG_DEBUG("Checking %s", tocreate.c_str());
		if (fileSystem->directoryExists(tocreate)) {
			continue;
		}

		if (!CreateDir(tocreate)) {
			return false;
		}
	}

	if (directoryExists(path)) {
		return true;
	}
	return CreateDir(path);
}

std::string CFileSystem::getPoolFilename(const std::string& md5str) const
{
	return fileSystem->getSpringDir()
	 + PATH_DELIMITER
	 + "pool"
	 + PATH_DELIMITER
	 + md5str.at(0)
	 + md5str.at(1)
	 + PATH_DELIMITER
	 + md5str.substr(2)
	 + ".gz";
}

int CFileSystem::validatePool(const std::string& path, bool deletebroken)
{
	if (!directoryExists(path)) {
		LOG_ERROR("Pool directory doesn't exist: %s", path.c_str());
		return 0;
	}
	int res = 0;
	std::list<std::string> dirs;
	dirs.push_back(path);
	const int maxdirs = 257; // FIXME: unknown dirs in pool will break bar
	int finished = 0;
	IHash* md5 = new HashMD5();
	while (!dirs.empty()) {
		const std::string dir = dirs.front();
		dirs.pop_front();
		DIR* d = opendir(dir.c_str());
		dirent* dentry;
		while ((dentry = readdir(d)) != nullptr) {
			LOG_PROGRESS(finished, maxdirs);
			const std::string absname = dir + PATH_DELIMITER + dentry->d_name;
			// don't check hidden files / . / ..
			if (dentry->d_name[0] == '.') {
				continue;
			}
#ifndef WIN32
			if ((dentry->d_type & DT_DIR) != 0) { // directory
#else
			struct stat sb;
			stat(absname.c_str(), &sb);
			if ((sb.st_mode & S_IFDIR) != 0) {
#endif
				dirs.push_back(absname);
				continue;
			}

			const int len = absname.length();
			if (len < 36) { // file length has at least to be
					// <md5[0]><md5[1]>/<md5[2-30]>.gz
				LOG_ERROR("Invalid file: %s", absname.c_str());
				continue;
			}

			std::string md5str;
			 // get md5 from path + filename
			md5str.push_back(absname.at(len - 36));
			md5str.push_back(absname.at(len - 35));
			md5str.append(absname.substr(len - 33, 30));
			md5->Set(md5str);
			FileData filedata;
			for (unsigned i = 0; i < 16; i++) {
				filedata.md5[i] = md5->get(i);
			}

			if (!fileIsValid(&filedata, absname)) { // check if md5 in filename
								// is the same as in
								// filename
				LOG_ERROR("Invalid File in pool: %s", absname.c_str());
				if (deletebroken) {
					removeFile(absname);
				}
			} else {
				res++;
			}
		}
		finished++;
		closedir(d);
	}
	delete md5;
	LOG_PROGRESS(finished, maxdirs, true);
	LOG("");
	return res;
}

bool CFileSystem::isOlder(const std::string& filename, int secs)
{
	if (secs <= 0)
		return true;
	struct stat sb;
	if (stat(filename.c_str(), &sb) < 0) {
		return true;
	}
#ifdef WIN32
	SYSTEMTIME pTime;
	FILETIME pFTime;
	GetSystemTime(&pTime);
	SystemTimeToFileTime(&pTime, &pFTime);
	const time_t t = FiletimeToTimestamp(pFTime);
	LOG_DEBUG("%s is %d seconds old, redownloading at %d", filename.c_str(), (int)(t - sb.st_ctime), secs);
	return (t < sb.st_ctime + secs);

#else
	time_t t;
	time(&t);
	struct tm lt;
	localtime_r(&sb.st_mtime, &lt);

	const time_t filetime = mktime(&lt);
	const double diff = difftime(t, filetime);

	LOG_DEBUG("checking time: %s  %.0fs >  %ds res: %d", filename.c_str(), diff, secs, (bool)(diff > secs));
	return (diff > secs);
#endif
}

bool CFileSystem::fileExists(const std::string& path)
{
	if (path.empty())
		return false;
#ifdef WIN32
	DWORD dwAttrib = GetFileAttributesW(s2ws(path).c_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES);
#else
	struct stat buffer;
	return stat(path.c_str(), &buffer) == 0;
#endif
}

bool CFileSystem::removeFile(const std::string& path)
{
#ifdef WIN32
	const bool res = _wunlink(s2ws(path).c_str()) == 0;
#else
	const bool res = unlink(path.c_str()) == 0;
#endif
	if (!res) {
		LOG_ERROR("Couldn't delete file %s", path.c_str());
	}
	return res;
}

bool CFileSystem::removeDir(const std::string& path)
{
#ifdef WIN32
	const bool res = _wrmdir(s2ws(path).c_str()) == 0;
#else
	const bool res = rmdir(path.c_str()) == 0;
#endif
	if (!res) {
		LOG_ERROR("Couldn't delete dir %s", path.c_str());
	}
	return res;
}

bool CFileSystem::parseTorrent(const char* data, int size, IDownload* dl)
{
	be_node* node = be_decoden(data, size);
	//#ifdef DEBUG
	//	be_dump(node);
	//#endif
	if (node == nullptr) {
		LOG_ERROR("couldn't parse torrent");
		return false;
	}
	if (node->type != BE_DICT) {
		LOG_ERROR("Error in torrent data");
		be_free(node);
		return false;
	}

	be_node* infonode = nullptr;
	for (int i = 0; node->val.d[i].val; ++i) { // search for a dict with name info
		if ((node->type == BE_DICT) && (strcmp(node->val.d[i].key, "info") == 0)) {
			infonode = node->val.d[i].val;
			break;
		}
	}
	if (infonode == nullptr) {
		LOG_ERROR("couldn't find info node in be dict");
		be_free(node);
		return false;
	}
	for (int i = 0; infonode->val.d[i].val; ++i) {
		// fetch needed data from dict and fill into dl
		be_node* datanode = infonode->val.d[i].val;
		switch (datanode->type) {
			case BE_STR: // current value is a string
				if ((strcmp("name", infonode->val.d[i].key) == 0) &&
				    (dl->name.empty())) { // set filename if not already set
					dl->name = datanode->val.s;
				} else if (!strcmp("pieces",
						   infonode->val.d[i].key)) { // hash sum of a piece
					const int count =
					    be_str_len(datanode) / 20; // one sha1 sum is 5 * 4 bytes long
					for (int i = 0; i < count; i++) {
						struct IDownload::piece piece;
						const unsigned char* data = (unsigned char*)&datanode->val.s[i * 20];
						piece.sha = new HashSHA1();
						if (!piece.sha->Set(data, 20)) {
							LOG_ERROR("Error setting sha1");
						}
						piece.state = IDownload::STATE_NONE;
						dl->pieces.push_back(piece);
					}
				}
				break;
			case BE_INT:						     // current value is a int
				if (strcmp("length", infonode->val.d[i].key) == 0) { // filesize
					dl->size = datanode->val.i;
				} else if (!strcmp("piece length",
						   infonode->val.d[i].key)) { // length of a piece
					dl->piecesize = datanode->val.i;
					LOG_DEBUG("dl->piecesize: %d", dl->piecesize);
				}
				break;
			default:
				break;
		}
	}
	LOG_DEBUG("Parsed torrent data: %s %d", dl->name.c_str(), dl->piecesize);
	be_free(node);
	return true;
}

bool CFileSystem::dumpSDP(const std::string& filename)
{
	std::list<FileData> files;
	if (!parseSdp(filename, files))
		return false;
	LOG_INFO("md5 (filename in pool)           crc32        size filename");
	HashMD5 md5;
	for (const FileData& fd: files) {
		md5.Set(fd.md5, sizeof(fd.md5));
		LOG_INFO("%s %.8X %8d %s", md5.toString().c_str(), fd.crc32,
		         fd.size, fd.name.c_str());
	}
	return true;
}

bool CFileSystem::validateSDP(const std::string& sdpPath)
{
	LOG_DEBUG("CFileSystem::validateSDP() ...");
	if (!fileExists(sdpPath)){
		LOG_ERROR("SDP file doesn't exist: %s", sdpPath.c_str());
		return false;
	}

	std::list<FileData> files;
	if (!parseSdp(sdpPath, files)) {// parse downloaded file
		LOG_ERROR("Removing invalid SDP file: %s", sdpPath.c_str());
		if (!removeFile(sdpPath)) {
			LOG_ERROR("Failed removing %s, aborting", sdpPath.c_str());
			return false;
		}
		return false;
	}

	bool valid = true;
	for (FileData& fd : files) {
		HashMD5 fileMd5;
		fileMd5.Set(fd.md5, sizeof(fd.md5));
		const std::string filePath = getPoolFilename(fileMd5.toString());
		if(!fileExists(filePath)) {
			valid = false;
			LOG_INFO("Missing file: %s", filePath.c_str());
		} else if (!fileIsValid(&fd, filePath)) {
			valid = false;
			LOG_INFO("Removing invalid file: %s", filePath.c_str());
			if (!removeFile(filePath)) {
				LOG_ERROR("Failed removing %s, aborting", filePath.c_str());
				return false;
			}
		}
	}
	LOG_DEBUG("CFileSystem::validateSDP() done");
	return valid;
}

bool CFileSystem::extractEngine(const std::string& filename,
				const std::string& version)
{
#ifdef ARCHIVE_SUPPORT
	const std::string output = getSpringDir() + PATH_DELIMITER + "engine" +
				   PATH_DELIMITER +
				   CFileSystem::EscapeFilename(version);
	if (!extract(filename, output)) {
		LOG_DEBUG("Failed to extract %s %s", filename.c_str(), output.c_str());
		return false;
	}
	if (portableDownload)
		return true;
	const std::string cfg = output + PATH_DELIMITER + "springsettings.cfg";
	if (fileExists(cfg)) {
		return removeFile(cfg);
	}
	return true;
#else
	LOG_ERROR("no archive support!");
	return false;
#endif
}

bool CFileSystem::extract(const std::string& filename,
			  const std::string& dstdir, bool overwrite)
{
#ifdef ARCHIVE_SUPPORT
	LOG_INFO("Extracting %s to %s", filename.c_str(), dstdir.c_str());
	const int len = filename.length();
	IArchive* archive;
	if ((len > 4) && (filename.compare(len - 3, 3, ".7z") == 0)) {
		archive = new CSevenZipArchive(filename);
	} else {
		archive = new CZipArchive(filename);
	}

	const unsigned int num = archive->NumFiles();
	if (num <= 0) {
		return false;
	}
	for (unsigned int i = 0; i < num; i++) {
		std::vector<unsigned char> buf;
		std::string name;
		int size, mode;
		archive->FileInfo(i, name, size, mode);
		if (!archive->GetFile(i, buf)) {
			LOG_ERROR("Error extracting %s from %s", name.c_str(), filename.c_str());
			delete archive;
			return false;
		}
#ifdef WIN32
		for (unsigned int i = 0; i < name.length();
		     i++) { // replace / with \ on win32
			if (name[i] == '/')
				name[i] = PATH_DELIMITER;
		}
#endif
		std::string tmp = dstdir;

		if (!tmp.empty() && tmp[tmp.length() - 1] != PATH_DELIMITER) {
			tmp += PATH_DELIMITER;
		}

		tmp += name.c_str(); // FIXME: concating UTF-16
		createSubdirs(DirName(tmp));
		if (fileSystem->fileExists(tmp)) {
			LOG_WARN("File already exists: %s", tmp.c_str());
			if (!overwrite)
				continue;
		}
		LOG_INFO("extracting (%s)", tmp.c_str());
		FILE* f = propen(tmp, "wb+");
		if (f == nullptr) {
			LOG_ERROR("Error creating %s", tmp.c_str());
			delete archive;
			return false;
		}
		int res = 1;
		if (!buf.empty())
			res = fwrite(&buf[0], buf.size(), 1, f);
#ifndef WIN32
		fchmod(fileno(f), mode);
#endif
		if (res <= 0) {
			const int err = ferror(f);
			LOG_ERROR("fwrite(%s): %d %s", name.c_str(), err, strerror(err));
			fclose(f);
			delete archive;
			return false;
		}
		fclose(f);
	}
	delete archive;
	LOG_INFO("done");
	return true;
#else
	LOG_ERROR("no archive support!");
	return false;
#endif
}

bool CFileSystem::Rename(const std::string& source,
			 const std::string& destination)
{
#ifdef WIN32
	return MoveFileW(s2ws(source).c_str(), s2ws(destination).c_str());
#else
	int res = rename(source.c_str(), destination.c_str());
	return (res == 0);
#endif
}

std::string CFileSystem::DirName(const std::string& path)
{
	const std::string::size_type pos = path.rfind(PATH_DELIMITER);
	if (pos != std::string::npos) {
		return path.substr(0, pos);
	} else {
		return path;
	}
}

#ifdef WIN32
long CFileSystem::FiletimeToTimestamp(const _FILETIME& time)
{
	LARGE_INTEGER date, adjust;
	date.HighPart = time.dwHighDateTime;
	date.LowPart = time.dwLowDateTime;
	adjust.QuadPart = 11644473600000 * 10000;
	date.QuadPart -= adjust.QuadPart;
	return (date.QuadPart / 10000000);
}

void CFileSystem::TimestampToFiletime(const time_t t, _FILETIME& pft)
{
	LONGLONG ll;
	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft.dwLowDateTime = (DWORD)ll;
	pft.dwHighDateTime = ll >> 32;
}
#endif

std::string CFileSystem::EscapeFilename(const std::string& str)
{
	std::string s = str;
	const static std::string illegalChars = "\\/:?\"<>|";

	for (auto it = s.begin(); it < s.end(); ++it) {
		const bool found = illegalChars.find(*it) != std::string::npos;
		if (found) {
			*it = '_';
		}
	}
	return s;
}

unsigned long CFileSystem::getMBsFree(const std::string& path)
{
#ifdef WIN32
	ULARGE_INTEGER freespace;
	BOOL res = GetDiskFreeSpaceEx(s2ws(path).c_str(), &freespace, nullptr, nullptr);
	if (!res) {
		LOG_ERROR("Error getting free disk space on %s: %d", path.c_str(), GetLastError());
		return 0;
	}
	return freespace.QuadPart / (1024 * 1024);
#else
	struct statvfs st;
	const int ret = statvfs(path.c_str(), &st);
	if (ret != 0) {
		const char *errstr = strerror(errno);
		LOG_ERROR("Error getting free disk space on %s: %s", path.c_str(), errstr);
		return 0;
	}
	if (st.f_frsize) {
		return ((uint64_t)st.f_frsize * st.f_bavail) / (1024 * 1024);
	}
	return ((uint64_t)st.f_bsize * st.f_bavail) / (1024 * 1024);
#endif
}


long CFileSystem::getFileSize(const std::string& path)
{
	FILE* f = propen(path.c_str(), "rb");
	if (f == nullptr)
		return -1;
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fclose(f);
	return size;
}
