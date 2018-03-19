#include "Lua.h"

#include <stdexcept>

#include <string.h>

namespace Rapid {

LuaT::LuaT()
:
	mState{luaL_newstate()}
{
	luaL_openlibs(mState);
}

LuaT::~LuaT()
{
	lua_close(mState);
}

ModinfoT LuaT::getModinfo(std::string const & Buffer)
{
	auto Error = luaL_dostring(mState, Buffer.c_str());
	if (Error != 0) throw std::runtime_error{"Lua error"};

	std::string Name;
	std::string Version;
	std::vector<std::string> Depends;

	lua_pushnil(mState);
	while (lua_next(mState, -2) != 0)
	{
		char const * Symbol = lua_tostring(mState, -2);

		// Get name string
		if (strcasecmp(Symbol, "name") == 0) Name = lua_tostring(mState, -1);

		// Get version string
		else if (strcasecmp(Symbol, "version") == 0) Version = lua_tostring(mState, -1);

		// Get depends array
		else if (strcasecmp(Symbol, "depend") == 0)
		{
			lua_pushstring(mState, "table");
			lua_gettable(mState, LUA_GLOBALSINDEX);

			lua_pushstring(mState, "getn");
			lua_gettable(mState, -2);

			lua_pushvalue(mState, -3);
			lua_call(mState, 1, 1);
			auto Number = lua_tonumber(mState, -1);
			Depends.reserve(Number);
			lua_pop(mState, 2);

			lua_pushnil(mState);
			while (lua_next(mState, -2) != 0)
			{
				Depends.push_back(lua_tostring(mState, -1));
				lua_pop(mState, 1);
			}
		}

		lua_pop(mState, 1);
	}

	return {Name, Version, Depends};
}

}
