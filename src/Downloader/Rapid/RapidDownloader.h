/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef RAPID_DOWNLOADER_H
#define RAPID_DOWNLOADER_H

#include "Downloader/IDownloader.h"

#include <string>
#include <list>
#include <stdio.h>
#define REPO_MASTER_RECHECK_TIME \
	86400 // how long to cache the repo-master file in secs without rechecking
#define REPO_RECHECK_TIME 0
#define REPO_MASTER "http://repos.springrts.com/repos.gz"

class CSdp;
class CHttpDownload;
class CFileSystem;
class CRepo;

class CRapidDownloader : public IDownloader
{
public:
	CRapidDownloader();
	~CRapidDownloader();

	/**
          search for a mod, searches for the short + long name
  */
	bool search(std::list<IDownload*>& result, const std::string& name,
		    DownloadEnum::Category = DownloadEnum::CAT_NONE) override;
	/**
          start a download
  */
	bool download(IDownload* download, int max_parallel = 10) override;

	bool setOption(const std::string& key, const std::string& value) override;

	void addRemoteSdp(CSdp& dsp);
	/**
          parses a rep master-file
  */
private:
	/**
          lists all tags on all servers
  */
	void list_tag();
	/**
          remove a dsp from the list of remote dsps
  */
	void downloadRepo(const std::string& url);
	bool updateRepos(const std::string& searchstr);
	bool parse();
	void downloadbyname(const std::string& name);
	std::string path;
	std::string url;
	std::list<CRepo> repos;

	/**
          download by name, for example "Complete Annihilation revision 1234"
  */
	bool download_name(IDownload* download, int reccounter = 0,
			   std::string name = "");
	/**
          update all repos from the web
  */
	bool reloadRepos(const std::string& searchstr);
	bool reposLoaded;
	/**
          helper function for sort
  */
	/**
  *	compare str1 with str2
  *	if str2==* or "" it matches
  *	used for search in downloaders
  */
	static bool match_download_name(const std::string& str1,
					const std::string& str2);

	static bool list_compare(CSdp& first, CSdp& second);
	std::list<CSdp> sdps;
};

#endif
