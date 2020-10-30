/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_FUNCTION_PTR_H
#define LSL_FUNCTION_PTR_H

#include <string>
#include "lslutils/logging.h"
#ifdef _WIN32
#include <windows.h>
#include <lslutils/conversion.h>
#include <lslutils/misc.h>
#else
#include <dlfcn.h>
#endif

namespace LSL
{

void _FreeLibrary(void* handle)
{
	if (handle == nullptr)
		return;
#ifdef _WIN32
	FreeLibrary((HMODULE)handle);
#else
	dlclose(handle);
#endif
}

void* _LoadLibrary(const std::string& libpath)
{
	void* res = nullptr;
#ifdef _WIN32
	const std::wstring wparentpath = Util::s2ws(LSL::Util::ParentPath(libpath));
	const std::wstring wlibpath = Util::s2ws(libpath);
	SetDllDirectory(nullptr);
	SetDllDirectory(wparentpath.c_str());
	res = LoadLibrary(wlibpath.c_str());
	if (res == nullptr) {
		LslError("Couldn't load the library %s: %s", libpath.c_str(), Util::geterrormsg().c_str());
		return res;
	}
#else
	res = dlopen(libpath.c_str(), RTLD_NOW | RTLD_LOCAL);
	if (res == nullptr) {
		const char* errmsg = dlerror();
		LslError("Couldn't load the library %s: %s", libpath.c_str(), errmsg);
		return nullptr;
	}
#endif
	return res;
}

void* GetLibFuncPtr(void* libhandle, const std::string& name)
{
	if (libhandle == nullptr)
		return nullptr;

#if defined _WIN32
	void* p = (void*)GetProcAddress((HMODULE)libhandle, name.c_str());
#else  // defined _WIN32
	void* p = dlsym(libhandle, name.c_str());
#endif // else defined _WIN32

	if (p == nullptr) {
		LslWarning("Couldn't load %s from unitsync library", name.c_str());
	}
	return p;
}


} // namespace LSL

#endif // LSL_FUNCTION_PTR_H
