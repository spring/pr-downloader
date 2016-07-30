#include "userdata.h"

namespace LSL
{

std::string UserStatus::GetDiffString(const UserStatus& old) const
{
	//TODO intl strings
	if (old.away != away)
		return (away ? std::string("away") : std::string("back"));
	if (old.in_game != in_game)
		return (in_game ? std::string("ingame") : std::string("back from game"));
	return std::string();
}

UserBattleStatus::UserBattleStatus()
    : team(0)
    , ally(0)
    , color(lslColor(0, 0, 0))
    , color_index(-1)
    , handicap(0)
    , side(0)
    , sync(SYNC_UNKNOWN)
    , spectator(false)
    , ready(false)
    , isfromdemo(false)
    , aitype(-1)
    , udpport(0)
{
}

bool UserBattleStatus::operator==(const UserBattleStatus& s) const
{
	return ((team == s.team) && (color == s.color) && (handicap == s.handicap) && (side == s.side) && (sync == s.sync) && (spectator == s.spectator) && (ready == s.ready) && (owner == s.owner) && (aishortname == s.aishortname) && (isfromdemo == s.isfromdemo) && (aitype == s.aitype));
}

bool UserBattleStatus::operator!=(const UserBattleStatus& s) const
{
	//		return ( ( team != s.team ) || ( color != s.color ) || ( handicap != s.handicap ) || ( side != s.side )
	//				 || ( sync != s.sync ) || ( spectator != s.spectator ) || ( ready != s.ready )
	//				 || ( owner != s.owner ) || ( aishortname != s.aishortname ) || ( isfromdemo != s.isfromdemo )
	//				 || ( aitype != s.aitype ) );
	return !(this->operator==(s));
}

} //namespace LSL {
