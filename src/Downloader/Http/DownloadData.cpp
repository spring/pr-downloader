/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "DownloadData.h"

#include <curl/curl.h>
#include "Downloader/CurlWrapper.h"

DownloadData::DownloadData()
{
	start_piece=0;
	mirror=NULL;
	download=NULL;
	curlw = new CurlWrapper();
	got_ranges = false;
}

DownloadData::~DownloadData()
{
	delete curlw;
	curlw = NULL;
}

