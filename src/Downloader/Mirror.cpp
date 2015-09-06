/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "Mirror.h"

#include <limits.h>

Mirror::Mirror(const std::string& url):
	url(url)
{
	status=STATUS_UNKNOWN;
	maxspeed=-1;
}

void Mirror::UpdateSpeed(int speed)
{
	if (speed>maxspeed)
		maxspeed=speed;
}
