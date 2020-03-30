/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "Mirror.h"

Mirror::Mirror(const std::string& url_)
    : url(url_)
{
}

void Mirror::UpdateSpeed(int speed)
{
	if (speed > maxspeed) {
		maxspeed = speed;
	}
}
