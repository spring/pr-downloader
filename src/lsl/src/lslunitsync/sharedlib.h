/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_FUNCTION_PTR_H
#define LSL_FUNCTION_PTR_H

#include <string>

namespace LSL
{


//! common loading point for functions from a given library handle
void _FreeLibrary(void* handle);
void* _LoadLibrary(const std::string& libpath);

void* GetLibFuncPtr(void* libhandle, const std::string& name);

} // namespace LSL

#endif // LSL_FUNCTION_PTR_H
