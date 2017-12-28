#ifndef LSL_LOGGING_H
#define LSL_LOGGING_H

extern void lsllogdebug(const char* format, ...);
extern void lslloginfo(const char* format, ...);
extern void lsllogwarning(const char* format, ...);
extern void lsllogerror(const char* format, ...);

#define LslDebug(...)                     \
	do {                              \
		lsllogdebug(__VA_ARGS__); \
	} while (0)
#define LslInfo(...)                     \
	do {                                \
		lslloginfo(__VA_ARGS__); \
	} while (0)
#define LslWarning(...)                     \
	do {                                \
		lsllogwarning(__VA_ARGS__); \
	} while (0)
#define LslError(...)                     \
	do {                              \
		lsllogerror(__VA_ARGS__); \
	} while (0)


#endif // LSL_LOGGING_H
