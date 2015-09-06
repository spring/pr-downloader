/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef _MIRROR_H
#define _MIRROR_H

#include <string>

class Mirror
{
public:
	Mirror(const std::string& url);
	void UpdateSpeed(int speed);

	enum MIRROR_STATUS {
		STATUS_BROKEN,
		STATUS_OK,
		STATUS_UNKNOWN
	} status;
	std::string url;
	int maxspeed;
};

#endif
