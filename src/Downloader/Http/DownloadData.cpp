/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "DownloadData.h"
#include "Downloader/CurlWrapper.h"

DownloadData::DownloadData() :
	curlw(new CurlWrapper())
{
}
