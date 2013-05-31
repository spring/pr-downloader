/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef IDOWNLOADSOBSERVER_H
#define IDOWNLOADSOBSERVER_H

class IDownload;

class IDownloadsObserver
{
public:
	virtual void Add(IDownload* dl);
	virtual void Remove(IDownload* dl);
private:
};

#endif // IDOWNLOADSOBSERVER_H
