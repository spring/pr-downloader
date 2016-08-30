/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "RapidDownloader.h"
#include "FileSystem/FileSystem.h"
#include "Util.h"
#include "Logger.h"
#include "Repo.h"
#include "Sdp.h"

#include <stdio.h>
#include <string>
#include <string.h>
#include <list>
#include <zlib.h>
#include <algorithm> //std::min
#include <set>

#ifndef WIN32
#include <regex.h>
#endif
#undef min
#undef max

CRapidDownloader::CRapidDownloader()
    : url(REPO_MASTER)
    , reposLoaded(false)
{
}

CRapidDownloader::~CRapidDownloader()
{
	sdps.clear();
}

void CRapidDownloader::addRemoteSdp(CSdp& sdp)
{
	sdps.push_back(sdp);
}

bool CRapidDownloader::list_compare(CSdp& first, CSdp& second)
{
	std::string name1;
	std::string name2;
	name1.clear();
	name2.clear();
	name1 = (first.getShortName());
	name2 = (second.getShortName());
	unsigned int len;
	len = std::min(name1.size(), name2.size());
	for (unsigned int i = 0; i < len; i++) {
		if (tolower(name1[i]) < tolower(name2[i])) {
			return true;
		}
	}
	return false;
}

bool CRapidDownloader::reloadRepos(const std::string& searchstr)
{
	if (reposLoaded)
		return true;
	reposLoaded = true;
	return updateRepos(searchstr);
}

bool CRapidDownloader::download_name(IDownload* download, int reccounter,
				     std::string name)
{
	LOG_DEBUG("%s %s", name.c_str(), download->name.c_str());
	if (reccounter > 10)
		return false;
	LOG_DEBUG("Using rapid to download %s", download->name.c_str());
	std::set<std::string> downloaded;

	for (CSdp& sdp : sdps) {
		if (!match_download_name(sdp.getName(),
					 name.empty() ? download->name : name))
			continue;

		if (downloaded.find(sdp.getMD5()) !=
		    downloaded.end()) // already downloaded, skip (i.e. stable entries are
				      // twice in versions.gz)
			continue;
		downloaded.insert(sdp.getMD5());

		LOG_DOWNLOAD(sdp.getName().c_str());
		if (!sdp.download(download)) {
			return false;
		}
		if (sdp.getDepends().empty())
			continue;
		if (!download_name(download, reccounter + 1, sdp.getDepends())) {
			return false;
		}
	}
	return true;
}

bool CRapidDownloader::search(std::list<IDownload*>& result,
			      const std::string& name,
			      DownloadEnum::Category cat)
{
	LOG_DEBUG("%s", name.c_str());
	reloadRepos(name);
	sdps.sort(list_compare);
	for (CSdp& sdp : sdps) {
		if (match_download_name(sdp.getShortName(), name) ||
		    (match_download_name(sdp.getName(), name))) {
			IDownload* dl =
			    new IDownload(sdp.getName().c_str(), name, cat, IDownload::TYP_RAPID);
			dl->addMirror(sdp.getShortName().c_str());
			result.push_back(dl);
		}
	}
	return true;
}

bool CRapidDownloader::download(IDownload* download, int /*max_parallel*/)
{
	LOG_DEBUG("%s", download->name.c_str());
	if (download->dltype != IDownload::TYP_RAPID) { // skip non-rapid downloads
		LOG_DEBUG("skipping non rapid-dl");
		return true;
	}
	reloadRepos(download->origin_name);
	return download_name(download, 0);
}

bool CRapidDownloader::match_download_name(const std::string& str1,
					   const std::string& str2)
{
	if (str2 == "")
		return true;
	if (str1 == str2)
		return true;
	if (str2 == "*")
		return true;
	// FIXME: add regex support for win32
	/*
  #ifndef WIN32
          regex_t regex;
          if (regcomp(&regex, str2.c_str(), 0)==0) {
                  int res=regexec(&regex, str1.c_str(),0, NULL, 0 );
                  regfree(&regex);
                  if (res==0) {
                          return true;
                  }
          }
  #endif
  */
	return false;
}

bool CRapidDownloader::setOption(const std::string& key,
				 const std::string& value)
{
	if (key == "masterurl") {
		url = value;
		reposLoaded = false;
		return true;
	}
	if (key == "forceupdate") {
		reposLoaded = false;
		return true;
	}
	return IDownloader::setOption(key, value);
}

void CRapidDownloader::downloadbyname(const std::string& name)
{
	std::string tmp;
	if (!urlToPath(name, tmp)) {
		LOG_ERROR("Invalid path: %s", tmp.c_str());
		return;
	}
	path = fileSystem->getSpringDir() + PATH_DELIMITER + "rapid" +
	       PATH_DELIMITER + tmp;
	fileSystem->createSubdirs(CFileSystem::DirName(path));
	LOG_DEBUG("%s", name.c_str());
	// first try already downloaded file, as repo master file rarely changes
	if ((fileSystem->fileExists(path)) &&
	    (fileSystem->isOlder(path, REPO_MASTER_RECHECK_TIME)) && parse())
		return;
	IDownload dl(path);
	dl.addMirror(name);
	httpDownload->download(&dl);
	parse();
}

bool CRapidDownloader::parse()
{
	FILE* f = fileSystem->propen(path, "rb");
	if (f == NULL) {
		return false;
	}
	gzFile fp = gzdopen(fileno(f), "rb");
	if (fp == Z_NULL) {
		LOG_ERROR("Could not open %s", path.c_str());
		return false;
	}
	char buf[IO_BUF_SIZE];
	repos.clear();
	int i = 0;
	while (gzgets(fp, buf, sizeof(buf)) != Z_NULL) {
		const std::string line = buf;
		const std::vector<std::string> items = tokenizeString(line, ',');
		if (items.size() <= 2) { // create new repo from url
			gzclose(fp);
			fclose(f);
			LOG_ERROR("Parse Error %s, Line %d: %s", path.c_str(), i, buf);
			return false;
		}
		i++;
		CRepo repotmp = CRepo(items[1], items[0], this);
		repos.push_back(repotmp);
	}
	gzclose(fp);
	fclose(f);
	LOG_INFO("Found %d repos in %s", repos.size(), path.c_str());
	return true;
}

bool CRapidDownloader::updateRepos(const std::string& searchstr)
{

	std::string::size_type pos = searchstr.find(':');
	std::string tag;
	if (pos != std::string::npos) { // a tag is found, set it
		tag = searchstr.substr(0, pos);
	}

	LOG_DEBUG("%s", "Updating repos...");
	downloadbyname(url);

	std::list<IDownload*> dls;
	std::list<CRepo*> usedrepos;
	for (CRepo& repo : repos) {
		IDownload* dl = new IDownload();
		if (repo.getDownload(*dl)) {
			if (repo.getShortName() == tag) { // matching repo exists, update this
							  // only
				IDownloader::freeResult(dls);
				usedrepos.clear();
				usedrepos.push_back(&repo);
				dls.push_back(dl);
				break;
			}
			usedrepos.push_back(&repo);
			dls.push_back(dl);
		} else {
			delete dl;
		}
	}

	httpDownload->download(dls);
	for (CRepo* repo : usedrepos) {
		repo->parse();
	}
	IDownloader::freeResult(dls);
	return true;
}
