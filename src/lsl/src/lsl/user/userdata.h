/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_USERDATA_H
#define LSL_USERDATA_H

#include <lslutils/type_forwards.h>
#include <lslutils/misc.h>
#include <string>

namespace LSL
{

const unsigned int SYNC_UNKNOWN = 0;
const unsigned int SYNC_SYNCED = 1;
const unsigned int SYNC_UNSYNCED = 2;

//! @brief Struct used to store a client's status.
struct UserStatus {
	enum RankContainer {
		RANK_1,
		RANK_2,
		RANK_3,
		RANK_4,
		RANK_5,
		RANK_6,
		RANK_7,
		RANK_8
	};

	bool in_game;
	bool away;
	RankContainer rank;
	bool moderator;
	bool bot;
	UserStatus()
	    : in_game(false)
	    , away(false)
	    , rank(RANK_1)
	    , moderator(false)
	    , bot(false)
	{
	}
	std::string GetDiffString(const UserStatus& other) const;
};

/** \todo really  not necessary to have a sep class for this **/
struct UserPosition {
	int x;
	int y;
	UserPosition()
	    : x(-1)
	    , y(-1)
	{
	}
};

//! Battle specific user data
struct UserBattleStatus {
	//!!! when adding something to this struct, also modify User::UpdateBattleStatus() !!
	// total 17 members here
	int team;
	int ally;
	lslColor color;
	int color_index;
	int handicap;
	int side;
	unsigned int sync;
	bool spectator;
	bool ready;
	bool isfromdemo;
	UserPosition pos; // for startpos = 4
	// bot-only stuff
	std::string owner;
	std::string aishortname;
	std::string airawname;
	std::string aiversion;
	int aitype;
	// for nat holepunching
	std::string ip;
	unsigned int udpport;
	std::string scriptPassword;
	bool IsBot() const
	{
		return !aishortname.empty();
	}
	UserBattleStatus();

	bool operator==(const UserBattleStatus& s) const;
	bool operator!=(const UserBattleStatus& s) const;
};

} // namespace LSL

#endif // LSL_USERDATA_H
