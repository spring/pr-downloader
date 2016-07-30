/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_HEADERGUARD_BATTLE_H
#define LSL_HEADERGUARD_BATTLE_H

#include <set>

#include <lslutils/type_forwards.h>
#include "ibattle.h"
#include "enum.h"

namespace LSL
{

class Server;
class User;
struct UnitsyncMap;
struct UnitsyncGame;

namespace Battle
{

/** \brief model of a multiplayer battle
* \todo DOCME */
class Battle : public IBattle
{
public:
	Battle(IServerPtr serv, int id);
	~Battle();
	int key() const
	{
		return m_id;
	}

	const IServerPtr GetServer()
	{
		return m_serv;
	}
	const ConstIServerPtr GetServer() const
	{
		return m_serv;
	}

	void SendHostInfo(Enum::HostInfo update);
	void SendHostInfo(const std::string& Tag);
	void Update(const std::string& Tag);

	void Join(const std::string& password = "");
	void Leave();

	void KickPlayer(const CommonUserPtr user);

	void RingNotReadyPlayers();
	void RingNotSyncedPlayers();
	void RingNotSyncedAndNotReadyPlayers();
	void RingPlayer(const ConstUserPtr u);

	void Say(const std::string& msg);
	void DoAction(const std::string& msg);

	void SetLocalMap(const UnitsyncMap& map);

	void OnRequestBattleStatus();
	void SendMyBattleStatus();

	bool ExecuteSayCommand(const std::string& cmd);

	void AddBot(const std::string& nick, UserBattleStatus status);

	void ForceSide(const CommonUserPtr user, int side);
	void ForceTeam(const CommonUserPtr user, int team);
	void ForceAlly(const CommonUserPtr user, int ally);
	void ForceColor(const CommonUserPtr user, const lslColor& col);
	void ForceSpectator(const CommonUserPtr user, bool spectator);
	void BattleKickPlayer(const CommonUserPtr user);
	void SetHandicap(const CommonUserPtr user, int handicap);

	void OnUserAdded(const CommonUserPtr user);
	void OnUserBattleStatusUpdated(const CommonUserPtr user, UserBattleStatus status);
	void OnUserRemoved(const CommonUserPtr user);

	void ForceUnsyncedToSpectate();
	void ForceUnReadyToSpectate();
	void ForceUnsyncedAndUnreadyToSpectate();

	void SetAutoLockOnStart(bool value);
	bool GetAutoLockOnStart();

	void SetLockExternalBalanceChanges(bool value);
	bool GetLockExternalBalanceChanges();

	void FixColors();
	void Autobalance(Enum::BalanceType balance_type = Enum::balance_divide, bool clans = true, bool strong_clans = true, int allyteamsize = 0);
	void FixTeamIDs(Enum::BalanceType balance_type = Enum::balance_divide, bool clans = true, bool strong_clans = true, int controlteamsize = 0);

	void SendScriptToClients();

	///< quick hotfix for bans
	bool IsBanned(const CommonUserPtr user);
	///>

	void SetImReady(bool ready);

	const CommonUserPtr GetMe();
	const ConstCommonUserPtr GetMe() const;

	void UserPositionChanged(const CommonUserPtr user);

	void SaveMapDefaults();
	void LoadMapDefaults(const std::string& mapname);

	void StartHostedBattle();
	void StartSpring();

	void SetInGame(bool ingame);

	void OnUnitsyncReloaded();

	void SetAutoUnspec(bool value);
	bool GetAutoUnspec()
	{
		return m_auto_unspec;
	}

	void ShouldAutoUnspec();
	void SetChannel(const ChannelPtr channel);
	const ChannelPtr GetChannel();

private:
	void OnTimer(const boost::system::error_code& error);
	// Battle variables

	///< quick hotfix for bans
	std::set<std::string> m_banned_users;
	std::set<std::string> m_banned_ips;
	///>

	IServerPtr m_serv;
	bool m_autolock_on_start;
	bool m_auto_unspec;
	const int m_id;
	ChannelPtr m_channel;
};

} // namespace Battle {
} // namespace LSL {

#endif // LSL_HEADERGUARD_BATTLE_H
