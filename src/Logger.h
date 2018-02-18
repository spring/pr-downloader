/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef LOGGER_H
#define LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

void LOG_DISABLE(bool disableLogging);

enum L_LEVEL {
	L_ERROR = 1,
	L_RAW = 2,
	L_INFO = 3,
	L_WARN = 4,
	L_DEBUG = 5,
};

/**
	*	plain log output
	*/
extern void L_LOG(const char* fileName, int line, const char* funcName,
           L_LEVEL level, const char* format, ...);

#define LOG(...) \
	L_LOG(__FILE__, __LINE__, __FUNCTION__, L_RAW, __VA_ARGS__)

#define LOG_ERROR(...) \
	L_LOG(__FILE__, __LINE__, __FUNCTION__, L_ERROR, __VA_ARGS__)

#define LOG_INFO(...) \
	L_LOG(__FILE__, __LINE__, __FUNCTION__, L_INFO, __VA_ARGS__)

#define LOG_WARN(...) \
	L_LOG(__FILE__, __LINE__, __FUNCTION__, L_WARN, __VA_ARGS__)

#ifndef NDEBUG
#define LOG_DEBUG(...) \
	L_LOG(__FILE__, __LINE__, __FUNCTION__, L_DEBUG, __VA_ARGS__);
#else
#define LOG_DEBUG(fmt, ...)
#endif

/**
	*	output progress bar
	*	@param done bytes already downloaded
	*	@param total total bytes to download
	*	@param forceOutput force output
	*/
extern void LOG_PROGRESS(long done, long total, bool forceOutput = false);

#ifdef __cplusplus
}
#endif


#endif
