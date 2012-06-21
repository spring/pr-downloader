/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


	enum L_LEVEL {
		L_ERROR = 1,
		L_RAW = 2,
		L_INFO = 3,
		L_DEBUG = 4
	};

	/**
	*	plain log output
	*/
	void L_LOG(L_LEVEL level, const char* format, ...);

#define LOG(...) \
	L_LOG(L_RAW, __VA_ARGS__)

#define LOG_ERROR(...) \
	L_LOG(L_ERROR, __VA_ARGS__)

#define LOG_INFO(...) \
	L_LOG(L_INFO, __VA_ARGS__)

#ifdef DEBUG
#define LOG_DEBUG(fmt, ...) \
	L_LOG(L_DEBUG, "%s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);
#else
#define	LOG_DEBUG(fmt, ...)
#endif

	void LOG_DOWNLOAD(const char* filename);

	/**
	*	output progress bar
	*	@param done bytes already downloaded
	*	@param total total bytes to download
	*	@param forceOutput force output
	*/
	void LOG_PROGRESS(long done, long total, bool forceOutput=false);

#ifdef __cplusplus
}
#endif


#endif
