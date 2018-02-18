#ifndef LSL_LOGGING_H
#define LSL_LOGGING_H

extern void lslLogDebug(const char* fileName, int line, const char* funcName,
                        const char* format, ...);
extern void lslLogInfo(const char* fileName, int line, const char* funcName,
                       const char* format, ...);
extern void lslLogWarning(const char* fileName, int line, const char* funcName,
                          const char* format, ...);
extern void lslLogError(const char* fileName, int line, const char* funcName,
                        const char* format, ...);

#define LslDebug(...)                     \
	do {                              \
		lslLogDebug(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
	} while (0)
#define LslInfo(...)                     \
	do {                                \
		lslLogInfo(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
	} while (0)
#define LslWarning(...)                     \
	do {                                \
		lslLogWarning(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
	} while (0)
#define LslError(...)                     \
	do {                              \
		lslLogError(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
	} while (0)


#endif // LSL_LOGGING_H
