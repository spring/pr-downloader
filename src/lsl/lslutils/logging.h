#ifndef LSL_LOGGING_H
#define LSL_LOGGING_H

#include "../../Logger.h"

#define LslDebug(...) \
	L_LOG(__FILE__, __LINE__, __FUNCTION__, L_DEBUG, __VA_ARGS__);

#define LslInfo(...) \
	L_LOG(__FILE__, __LINE__, __FUNCTION__, L_INFO, __VA_ARGS__);

#define LslWarning(...) \
	L_LOG(__FILE__, __LINE__, __FUNCTION__, L_WARN, __VA_ARGS__);

#define LslError(...) \
	L_LOG(__FILE__, __LINE__, __FUNCTION__, L_ERROR, __VA_ARGS__);

#endif // LSL_LOGGING_H
