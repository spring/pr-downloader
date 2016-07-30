/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "userlist.h"

namespace LSL
{

const ConstUserPtr UserList::FindByNick(const std::string& nick) const
{
	MapType::const_iterator it = find(nick);
	if (it != end())
		return it->second;
	return ConstUserPtr();
}

const UserPtr UserList::FindByNick(const std::string& nick)
{
	MapType::const_iterator it = find(nick);
	if (it != end())
		return it->second;
	return UserPtr();
}

const ConstCommonUserPtr CommonUserList::FindByNick(const std::string& nick) const
{
	MapType::const_iterator it = find(nick);
	if (it != end())
		return it->second;
	return ConstCommonUserPtr();
}

const CommonUserPtr CommonUserList::FindByNick(const std::string& nick)
{
	MapType::const_iterator it = find(nick);
	if (it != end())
		return it->second;
	return CommonUserPtr();
}
}
