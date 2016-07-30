/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_LOADER_H
#define LSL_LOADER_H

namespace LSL
{

class UnitsyncLib;

struct UnitsyncFunctionLoader {
	static bool BindFunctions(UnitsyncLib* s);
	static void UnbindFunctions(UnitsyncLib* s);
};

} // namespace LSL

#endif // LSL_LOADER_H
