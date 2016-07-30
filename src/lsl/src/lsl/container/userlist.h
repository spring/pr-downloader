/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LIBSPRINGLOBBY_HEADERGUARD_USERLIST_H
#define LIBSPRINGLOBBY_HEADERGUARD_USERLIST_H

#include "base.h"
#include <lsl/user/user.h>

namespace LSL
{

//! container for user pointers
class UserList : public ContainerBase<User>
{
public:
	const ConstUserPtr FindByNick(const std::string& nick) const;
	const UserPtr FindByNick(const std::string& nick);
};

class CommonUserList : public ContainerBase<CommonUser>
{
public:
	const ConstCommonUserPtr FindByNick(const std::string& nick) const;
	const CommonUserPtr FindByNick(const std::string& nick);
};

} // namespace LSL

#endif // LIBSPRINGLOBBY_HEADERGUARD_USERLIST_H
