/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef RAPID_DOWNLOADER_H
#define RAPID_DOWNLOADER_H

#include "Downloader/IDownloader.h"

#include <string>
#include <list>
#include <stdio.h>
#define REPO_MASTER_RECHECK_TIME 86400 //how long to cache the repo-master file in secs without rechecking
#define REPO_RECHECK_TIME 0
#define REPO_MASTER "http://repos.springrts.com/repos.gz"

class CSdp;
class CHttpDownload;
class CFileSystem;
class CRepo;

class CRapidDownloader: public IDownloader
{
public:
	CRapidDownloader();
	~CRapidDownloader();

	/**
		download a tag, for exampe ba:stable
	*/
	bool download_tag(const std::string& modname);
	/**
		lists all tags on all servers
	*/
	void list_tag();
	/**
		remove a dsp from the list of remote dsps
	*/
	void addRemoteDsp(CSdp& dsp);
	/**
		search for a mod, searches for the short + long name
	*/
	virtual bool search(std::list<IDownload*>& result, const std::string& name, IDownload::category=IDownload::CAT_NONE);
	/**
		start a download
	*/
	virtual bool download(IDownload* download,int max_parallel = 10);

	virtual bool setOption(const std::string& key, const std::string& value);
	/**
		parses a rep master-file
	*/
	void updateRepos();
	void downloadRepo(const std::string& url);
	
	

private:
	bool parse();
	void download(const std::string& name);
	std::string path;
	std::string url;
	std::list<CRepo> repos;

	/**
		download by name, for example "Complete Annihilation revision 1234"
	*/
	bool download_name(IDownload * download, int reccounter=0, std::string name = "");
	/**
		update all repos from the web
	*/
	bool reloadRepos();
	bool reposLoaded;
	/**
		helper function for sort
	*/
	/**
	*	compare str1 with str2
	*	if str2==* or "" it matches
	*	used for search in downloaders
	*/
	static bool match_download_name(const std::string &str1,const std::string& str2);

	static bool list_compare(CSdp& first ,CSdp& second);
	std::list<CSdp> sdps;
};

#endif
