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

void Mirror::escapeUrl(std::string& escaped)
{
	for(unsigned int i=0; i<url.size(); i++) {
		if (url[i]==' ')
			escaped.append("%20");
		else
			escaped.append(1,url[i]);
	}
}
