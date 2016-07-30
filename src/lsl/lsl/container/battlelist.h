/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LIBSPRINGLOBBY_HEADERGUARD_BATTLELIST_H
#define LIBSPRINGLOBBY_HEADERGUARD_BATTLELIST_H

#include "base.h"
#include <lsl/battle/battle.h>
#include <lslutils/type_forwards.h>

namespace LSL
{
namespace Battle
{

//! container for battle pointer
class BattleList : public ContainerBase<Battle>
{
public:
	std::string GetChannelName(const ConstIBattlePtr battle);
};

} //namespace Battle
} //namespace LSL

#endif // LIBSPRINGLOBBY_HEADERGUARD_BATTLELIST_H
