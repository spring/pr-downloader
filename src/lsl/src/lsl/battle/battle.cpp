#include "battle.h"

/* Copyright (C) 2007 The SpringLobby Team. All rights reserved. */
//
// Class: Battle
//
#include "battle.h"
#include "signals.h"

#include <boost/make_shared.hpp>
#include <lsl/networking/iserver.h>
#include <lsl/user/user.h>
#include <lslutils/misc.h>
#include <lslutils/debug.h>
#include <lslutils/logging.h>
#include <lslutils/conversion.h>
#include <lslutils/config.h>
#include <lsl/spring/spring.h>
#include <lslunitsync/optionswrapper.h>

//SL includes -- bad
#if HAVE_SPRINGLOBBY
#warning FIXME: SpringLobby includes are used!
#include "settings.h"
#include "utils/conversion.h"
#else
#include <lslutils/config.h>
#endif

#define ASSERT_LOGIC(...) \
	do {              \
	} while (0)

namespace LSL
{
namespace Battle
{

Battle::Battle(IServerPtr serv, int id)
    : m_serv(serv)
    , m_autolock_on_start(false)
    , m_auto_unspec(false)
    , m_id(id)

{
	m_opts.battleid = m_id;
}

Battle::~Battle()
{
}

void Battle::SendHostInfo(Enum::HostInfo update)
{
	m_serv->SendHostInfo(update);
}

void Battle::SendHostInfo(const std::string& Tag)
{
	m_serv->SendHostInfo(Tag);
}

void Battle::Update(const std::string& Tag)
{
	Signals::sig_BattleInfoUpdate(shared_from_this(), Tag);
}

void Battle::Join(const std::string& password)
{
	m_serv->JoinBattle(shared_from_this(), password);
	m_is_self_in = true;
}

void Battle::Leave()
{
	m_serv->LeaveBattle(shared_from_this());
}

void Battle::OnRequestBattleStatus()
{
	UserBattleStatus& bs = m_serv->GetMe()->BattleStatus();
	bs.team = GetFreeTeam(true);
	bs.ally = GetFreeAlly(true);
	bs.spectator = false;
	bs.color = Util::config().GetBattleLastColor();
	// FIXME? bs.color = Util::GetFreeColor( GetMe() );
	bs.side = Util::config().GetBattleLastSideSel(GetHostGameName());
	SendMyBattleStatus();
}

void Battle::SendMyBattleStatus()
{
	UserBattleStatus& bs = m_serv->GetMe()->BattleStatus();
	if (IsSynced())
		bs.sync = SYNC_SYNCED;
	else
		bs.sync = SYNC_UNSYNCED;
	m_serv->SendMyBattleStatus(bs);
}

void Battle::SetImReady(bool ready)
{
	UserBattleStatus& bs = m_serv->GetMe()->BattleStatus();
	bs.ready = ready;
	//m_serv->GetMe()->SetBattleStatus( bs );
	SendMyBattleStatus();
}

/*bool Battle::HasMod()
{
  return usync().GameExists( m_opts.modname );
}*/

void Battle::Say(const std::string& msg)
{
	m_serv->SayBattle(shared_from_this(), msg);
}

void Battle::DoAction(const std::string& msg)
{
	m_serv->DoActionBattle(shared_from_this(), msg);
}

void Battle::SetLocalMap(const UnitsyncMap& map)
{
	IBattle::SetLocalMap(map);
	if (IsFounderMe())
		LoadMapDefaults(map.name);
}

const ConstCommonUserPtr Battle::GetMe() const
{
	return m_serv->GetMe();
}

const CommonUserPtr Battle::GetMe()
{
	return m_serv->GetMe();
}

void Battle::SaveMapDefaults()
{
	// save map preset
	std::string mapname = LoadMap().name;
	std::string startpostype = CustomBattleOptions()->getSingleValue("startpostype", LSL::Enum::EngineOption);
	Util::config().SetMapLastStartPosType(mapname, startpostype);
	std::vector<LSL::Util::SettStartBox> rects;
	for (unsigned int i = 0; i <= GetLastRectIdx(); ++i) {
		BattleStartRect rect = GetStartRect(i);
		if (rect.IsOk()) {
			LSL::Util::SettStartBox box;
			box.ally = rect.ally;
			box.topx = rect.left;
			box.topy = rect.top;
			box.bottomx = rect.right;
			box.bottomy = rect.bottom;
			rects.push_back(box);
		}
	}
	Util::config().SetMapLastRectPreset(mapname, rects);
}

void Battle::LoadMapDefaults(const std::string& mapname)
{
	CustomBattleOptions()->setSingleOption("startpostype", Util::config().GetMapLastStartPosType(mapname), LSL::Enum::EngineOption);
	SendHostInfo((boost::format("%d_startpostype") % LSL::Enum::EngineOption).str());

	for (unsigned int i = 0; i <= GetLastRectIdx(); ++i) {
		if (GetStartRect(i).IsOk())
			RemoveStartRect(i); // remove all rects
	}
	SendHostInfo(Enum::HI_StartRects);

	/*    const std::vector<LSL::Util::SettStartBox> savedrects = Util::config().GetMapLastRectPreset( mapname );
    for ( std::vector<LSL::Util::SettStartBox>::const_iterator itor = savedrects.begin(); itor != savedrects.end(); ++itor )
    {
        AddStartRect( itor->ally, itor->topx, itor->topy, itor->bottomx, itor->bottomy );
    }
*/
	SendHostInfo(Enum::HI_StartRects);
}

void Battle::OnUserAdded(const CommonUserPtr user)
{
	if (!user)
		return;
	m_userlist.Add(user);
	IBattle::OnUserAdded(user);
	if (user == GetMe()) {
		m_timer->async_wait(boost::bind(&Battle::OnTimer, this, _1));
	}
	user->SetBattle(shared_from_this());
	user->BattleStatus().isfromdemo = false;

	if (IsFounderMe()) {
		if (IsBanned(user))
			return;

		if ((user != GetMe()) && !user->BattleStatus().IsBot() &&
		    (m_opts.rankneeded != UserStatus::RANK_1) && !user->BattleStatus().spectator) {
			if (m_opts.rankneeded > UserStatus::RANK_1 && user->Status().rank < m_opts.rankneeded) {
				DoAction("Rank limit autospec: " + user->Nick());
				ForceSpectator(user, true);
			} else if (m_opts.rankneeded < UserStatus::RANK_1 && user->Status().rank > (-m_opts.rankneeded - 1)) {
				DoAction("Rank limit autospec: " + user->Nick());
				ForceSpectator(user, true);
			}
		}

		//        m_ah.OnUserAdded( user );
		if (!user->BattleStatus().IsBot() && Util::config().GetBattleLastAutoAnnounceDescription())
			DoAction(m_opts.description);
	}
	// any code here may be skipped if the user was autokicked
	return;
}

void Battle::OnUserBattleStatusUpdated(const CommonUserPtr user, UserBattleStatus status)
{
	if (IsFounderMe()) {
		if ((user != GetMe()) && !status.IsBot() && (m_opts.rankneeded != UserStatus::RANK_1) && !status.spectator) {
			if (m_opts.rankneeded > UserStatus::RANK_1 && user->Status().rank < m_opts.rankneeded) {
				DoAction("Rank limit autospec: " + user->Nick());
				ForceSpectator(user, true);
			} else if (m_opts.rankneeded < UserStatus::RANK_1 && user->Status().rank > (-m_opts.rankneeded - 1)) {
				DoAction("Rank limit autospec: " + user->Nick());
				ForceSpectator(user, true);
			}
		}
		UserBattleStatus previousstatus = user->BattleStatus();
		if (m_opts.lockexternalbalancechanges) {
			if (previousstatus.team != status.team) {
				ForceTeam(user, previousstatus.team);
				status.team = previousstatus.team;
			}
			if (previousstatus.ally != status.ally) {
				ForceAlly(user, previousstatus.ally);
				status.ally = previousstatus.ally;
			}
		}
	}
	IBattle::OnUserBattleStatusUpdated(user, status);
	if (status.handicap != 0) {
		//        UiEvents::GetUiEventSender( UiEvents::OnBattleActionEvent ).SendEvent(
		//                    UiEvents::OnBattleActionData( std::string(" ")) , ( "Warning: user ") + user->Nick() + " got bonus ") ) << status.handicap )
		//                    );
	}
	if (IsFounderMe()) {
		if (ShouldAutoStart()) {
			Signals::sig_BattleCouldStartHosted(shared_from_this());
		}
	}
	if (!GetMe()->BattleStatus().spectator)
		SetAutoUnspec(false); // we don't need auto unspec anymore
	ShouldAutoUnspec();
	//    ui().OnUserBattleStatus( *this, user );
}

void Battle::OnUserRemoved(CommonUserPtr user)
{
	//    m_ah.OnUserRemoved(user);
	IBattle::OnUserRemoved(user);
	ShouldAutoUnspec();
}

void Battle::RingNotReadyPlayers()
{
	for (size_t i = 0; i < m_userlist.size(); i++) {
		const ConstCommonUserPtr u = m_userlist.At(i);
		const UserBattleStatus& bs = u->BattleStatus();
		if (bs.IsBot())
			continue;
		if (!bs.ready && !bs.spectator)
			m_serv->Ring(u);
	}
}

void Battle::RingNotSyncedPlayers()
{
	for (size_t i = 0; i < m_userlist.size(); i++) {
		const ConstCommonUserPtr u = m_userlist.At(i);
		const UserBattleStatus& bs = u->BattleStatus();
		if (bs.IsBot())
			continue;
		if (!bs.sync && !bs.spectator)
			m_serv->Ring(u);
	}
}

void Battle::RingNotSyncedAndNotReadyPlayers()
{
	for (size_t i = 0; i < m_userlist.size(); i++) {
		const ConstCommonUserPtr u = m_userlist.At(i);
		const UserBattleStatus& bs = u->BattleStatus();
		if (bs.IsBot())
			continue;
		if ((!bs.sync || !bs.ready) && !bs.spectator)
			m_serv->Ring(u);
	}
}

void Battle::RingPlayer(const ConstUserPtr u)
{
	if (u->BattleStatus().IsBot())
		return;
	m_serv->Ring(u);
}

bool Battle::ExecuteSayCommand(const std::string& cmd)
{
	std::string cmd_name = boost::algorithm::to_lower_copy(Util::BeforeFirst(cmd, " "));
	if (cmd_name == "/me") {
		m_serv->DoActionBattle(shared_from_this(), Util::AfterFirst(cmd, " "));
		return true;
	}
	if (cmd_name == "/replacehostip") {
		std::string ip = Util::AfterFirst(cmd, " ");
		if (ip.empty())
			return false;
		m_opts.ip = ip;
		return true;
	}
	//< quick hotfix for bans
	if (IsFounderMe()) {
		if (cmd_name == "/ban") {
			std::string nick = Util::AfterFirst(cmd, " ");
			m_banned_users.insert(nick);
			try {
				const CommonUserPtr user = GetUser(nick);
				const IBattlePtr b = shared_from_this();
				m_serv->BattleKickPlayer(b, user);
			} catch (/*assert_exception*/...) {
			}
			//            UiEvents::GetUiEventSender( UiEvents::OnBattleActionEvent ).SendEvent(
			//						UiEvents::OnBattleActionData( std::string(" ") , nick+" banned" )
			//                        );

			//m_serv->DoActionBattle( m_opts.battleid, cmd.AfterFirst(' ') );
			return true;
		}
		if (cmd_name == "/unban") {
			std::string nick = Util::AfterFirst(cmd, " ");
			m_banned_users.erase(nick);
			//            UiEvents::GetUiEventSender( UiEvents::OnBattleActionEvent ).SendEvent(
			//						UiEvents::OnBattleActionData( std::string(" ") , nick+" unbanned" )
			//                        );
			//m_serv->DoActionBattle( m_opts.battleid, cmd.AfterFirst(' ') );
			return true;
		}
		if (cmd_name == "/banlist") {
			//            UiEvents::GetUiEventSender( UiEvents::OnBattleActionEvent ).SendEvent(
			//						UiEvents::OnBattleActionData( std::string(" ") , "banlist:" )
			//                        );

			for (std::set<std::string>::const_iterator i = m_banned_users.begin(); i != m_banned_users.end(); ++i) {
				//                UiEvents::GetUiEventSender( UiEvents::OnBattleActionEvent ).SendEvent(
				//							UiEvents::OnBattleActionData( std::string(" ") , *i )
				//                            );
			}
			for (std::set<std::string>::iterator i = m_banned_ips.begin(); i != m_banned_ips.end(); ++i) {
				//                UiEvents::GetUiEventSender( UiEvents::OnBattleActionEvent ).SendEvent(
				//							UiEvents::OnBattleActionData( std::string(" ") , *i )
				//                            );
			}
			return true;
		}
		if (cmd_name == "/unban") {
			std::string nick = Util::AfterFirst(cmd, " ");
			m_banned_users.erase(nick);
			m_banned_ips.erase(nick);
			//            UiEvents::GetUiEventSender( UiEvents::OnBattleActionEvent ).SendEvent(
			//						UiEvents::OnBattleActionData( std::string(" ") , nick+" unbanned" )
			//                        );

			//m_serv->DoActionBattle( m_opts.battleid, cmd.AfterFirst(' ') );
			return true;
		}
		if (cmd_name == "/ipban") {
			std::string nick = Util::AfterFirst(cmd, " ");
			m_banned_users.insert(nick);
			//            UiEvents::GetUiEventSender( UiEvents::OnBattleActionEvent ).SendEvent(
			//						UiEvents::OnBattleActionData( std::string(" ") , nick+" banned" )
			//                        );

			CommonUserPtr user = m_userlist.FindByNick(nick);
			if (user) {
				const CommonUserPtr user = GetUser(nick);
				if (!user->BattleStatus().ip.empty()) {
					m_banned_ips.insert(user->BattleStatus().ip);
					//                    UiEvents::GetUiEventSender( UiEvents::OnBattleActionEvent ).SendEvent(
					//								UiEvents::OnBattleActionData( std::string(" ") , user->BattleStatus().ip+" banned" )
					//                                );
				}
				m_serv->BattleKickPlayer(shared_from_this(), user);
			}
			//m_banned_ips.erase(nick);

			//m_serv->DoActionBattle( m_opts.battleid, cmd.AfterFirst(' ') );
			return true;
		}
	}
	//>
	return false;
}

///< quick hotfix for bans
/// returns true if user is banned (and hence has been kicked)
bool Battle::IsBanned(const CommonUserPtr user)
{
	if (IsFounderMe()) {
		if (m_banned_users.count(user->Nick()) > 0)
		//				|| useractions().DoActionOnUser(UserActions::ActAutokick, user->Nick() ) )
		{
			KickPlayer(user);
			//            UiEvents::GetUiEventSender( UiEvents::OnBattleActionEvent ).SendEvent(
			//						UiEvents::OnBattleActionData( std::string(" ") , user->Nick()+" is banned, kicking" )
			//                        );
			return true;
		} else if (m_banned_ips.count(user->BattleStatus().ip) > 0) {
			//            UiEvents::GetUiEventSender( UiEvents::OnBattleActionEvent ).SendEvent(
			//						UiEvents::OnBattleActionData( std::string(" ") , user->BattleStatus().ip+" is banned, kicking" )
			//                        );
			KickPlayer(user);
			return true;
		}
	}
	return false;
}
///>

void Battle::SetAutoLockOnStart(bool value)
{
	m_autolock_on_start = value;
}

bool Battle::GetAutoLockOnStart()
{
	return m_autolock_on_start;
}

void Battle::SetLockExternalBalanceChanges(bool value)
{
	if (value)
		DoAction("has locked player balance changes");
	else
		DoAction("has unlocked player balance changes");
	m_opts.lockexternalbalancechanges = value;
}

bool Battle::GetLockExternalBalanceChanges()
{
	return m_opts.lockexternalbalancechanges;
}


void Battle::AddBot(const std::string& nick, UserBattleStatus status)
{
	m_serv->AddBot(shared_from_this(), nick, status);
}

void Battle::ForceSide(CommonUserPtr user, int side)
{
	m_serv->ForceSide(shared_from_this(), user, side);
}

void Battle::ForceTeam(CommonUserPtr user, int team)
{
	IBattle::ForceTeam(user, team);
	m_serv->ForceTeam(shared_from_this(), user, team);
}

void Battle::ForceAlly(CommonUserPtr user, int ally)
{
	IBattle::ForceAlly(user, ally);
	m_serv->ForceAlly(shared_from_this(), user, ally);
}

void Battle::ForceColor(CommonUserPtr user, const lslColor& col)
{
	IBattle::ForceColor(user, col);
	m_serv->ForceColor(shared_from_this(), user, col);
}

void Battle::ForceSpectator(CommonUserPtr user, bool spectator)
{
	m_serv->ForceSpectator(shared_from_this(), user, spectator);
}

void Battle::KickPlayer(CommonUserPtr user)
{
	m_serv->BattleKickPlayer(shared_from_this(), user);
}

void Battle::SetHandicap(CommonUserPtr user, int handicap)
{
	m_serv->SetHandicap(shared_from_this(), user, handicap);
}

void Battle::ForceUnsyncedToSpectate()
{
	const size_t numusers = m_userlist.size();
	for (size_t i = 0; i < numusers; ++i) {
		const CommonUserPtr user = m_userlist.At(i);
		UserBattleStatus& bs = user->BattleStatus();
		if (bs.IsBot())
			continue;
		if (!bs.spectator && !bs.sync)
			ForceSpectator(user, true);
	}
}

void Battle::ForceUnReadyToSpectate()
{
	const size_t numusers = m_userlist.size();
	for (size_t i = 0; i < numusers; ++i) {
		const CommonUserPtr user = m_userlist.At(i);
		UserBattleStatus& bs = user->BattleStatus();
		if (bs.IsBot())
			continue;
		if (!bs.spectator && !bs.ready)
			ForceSpectator(user, true);
	}
}

void Battle::ForceUnsyncedAndUnreadyToSpectate()
{
	const size_t numusers = m_userlist.size();
	for (size_t i = 0; i < numusers; ++i) {
		const CommonUserPtr user = m_userlist.At(i);
		UserBattleStatus& bs = user->BattleStatus();
		if (bs.IsBot())
			continue;
		if (!bs.spectator && (!bs.sync || !bs.ready))
			ForceSpectator(user, true);
	}
}

void Battle::UserPositionChanged(const CommonUserPtr user)
{
	m_serv->SendUserPosition(user);
}

void Battle::SendScriptToClients()
{
	m_serv->SendScriptToClients(GetScript());
}

void Battle::StartHostedBattle()
{
	if (m_userlist.Exists(GetMe())) {
		assert(false);
		//        if ( IsFounderMe() )
		//        {
		//            if ( sett().GetBattleLastAutoControlState() )
		//            {
		//                FixTeamIDs( (Enum::BalanceType)sett().GetFixIDMethod(), sett().GetFixIDClans(),
		//                            sett().GetFixIDStrongClans(), sett().GetFixIDGrouping() );
		//                Autobalance( (Enum::BalanceType)sett().GetBalanceMethod(), sett().GetBalanceClans(),
		//                             sett().GetBalanceStrongClans(), sett().GetBalanceGrouping() );
		//                FixColors();
		//            }
		//            if ( IsProxy() )
		//            {
		//                if ( UserExists( GetProxy() ) && !GetUser(GetProxy()).Status().in_game )
		//                {
		//                    // DON'T set m_generating_script here, it will trick the script generating code to think we're the host
		//                    std::string hostscript = spring().WriteScriptTxt( *this );
		//                    try
		//                    {
		//						std::string path = sett().GetCurrentUsedDataDir() + wxFileName::GetPathSeparator() + "relayhost_script.txt";
		//                        if ( !wxFile::Access( path, wxFile::write ) ) {
		//                            LslError( "Access denied to script.txt." );
		//                        }

		//                        wxFile f( path, wxFile::write );
		//                        f.Write( hostscript );
		//                        f.Close();

		//                    } catch (...) {}
		//                    m_serv->SendScriptToProxy( hostscript );
		//                }
		//            }
		//            if( GetAutoLockOnStart() )
		//            {
		//                SetIsLocked( true );
		//                SendHostInfo( Enum::HI_Locked );
		//            }
		//            sett().SetLastHostMap( GetServer().GetCurrentBattle()->GetHostMapName() );
		//            sett().SaveSettings();
		//            if ( !IsProxy() ) GetServer().StartHostedBattle();
		//            else if ( UserExists( GetProxy() ) && GetUser(GetProxy()).Status().in_game ) // relayhost is already ingame, let's try to join it
		//            {
		//                StartSpring();
		//            }
		//        }
	}
}

void Battle::StartSpring()
{
	const CommonUserPtr me = GetMe();
	if (me && !me->Status().in_game) {
		me->BattleStatus().ready = false;
		SendMyBattleStatus();
		// set m_generating_script, this will make the script.txt writer realize we're just clients even if using a relayhost
		m_generating_script = true;
		me->Status().in_game = spring().Run(shared_from_this());
		m_generating_script = false;
		me->SendMyUserStatus();
	}
	//    ui().OnBattleStarted( *this );
}

void Battle::OnTimer(const boost::system::error_code& error)
{
	if (error)
		return;
	if (!IsFounderMe())
		return;
	if (InGame())
		return;
	int autospect_trigger_time = Util::config().GetBattleLastAutoSpectTime();
	if (autospect_trigger_time == 0)
		return;
	time_t now = time(0);
	for (unsigned int i = 0; i < m_userlist.size(); ++i) {
		const CommonUserPtr usr = m_userlist[i];
		UserBattleStatus& status = usr->BattleStatus();
		if (status.IsBot() || status.spectator)
			continue;
		if (status.sync && status.ready)
			continue;
		if (usr == GetMe())
			continue;
		std::map<std::string, time_t>::const_iterator itor = m_ready_up_map.find(usr->Nick());
		if (itor != m_ready_up_map.end()) {
			if ((now - itor->second) > autospect_trigger_time) {
				ForceSpectator(usr, true);
			}
		}
	}
}

void Battle::SetInGame(bool value)
{
	time_t now = time(0);
	if (InGame() && !value) {
		for (int i = 0; i < long(m_userlist.size()); ++i) {
			const CommonUserPtr user = m_userlist[i];
			UserBattleStatus& status = user->BattleStatus();
			if (status.IsBot() || status.spectator)
				continue;
			if (status.ready && status.sync)
				continue;
			m_ready_up_map[user->Nick()] = now;
		}
	}
	IBattle::SetInGame(value);
}

void Battle::FixColors()
{
	if (!IsFounderMe())
		return;
	std::vector<lslColor> palette = GetFixColorsPalette(m_teams_sizes.size() + 1);
	std::vector<int> palette_use(palette.size(), 0);

	lslColor my_col = GetMe()->BattleStatus().color; // Never changes color of founder (me) :-)
	int my_diff = 0;
	int my_col_i = GetClosestFixColor(my_col, palette_use, my_diff);
	palette_use[my_col_i]++;
	std::set<int> parsed_teams;

	for (size_t i = 0; i < m_userlist.size(); i++) {
		const CommonUserPtr user = m_userlist.At(i);
		if (user == GetMe())
			continue; // skip founder ( yourself )
		UserBattleStatus& status = user->BattleStatus();
		if (status.spectator)
			continue;
		if (parsed_teams.find(status.team) != parsed_teams.end())
			continue; // skip duplicates
		parsed_teams.insert(status.team);

		lslColor& user_col = status.color;
		int user_col_i = GetClosestFixColor(user_col, palette_use, 60);
		palette_use[user_col_i]++;
		for (size_t j = 0; j < m_userlist.size(); ++j) {
			const CommonUserPtr other_user = m_userlist.At(j);
			if (other_user->BattleStatus().team == status.team) {
				ForceColor(other_user, palette[user_col_i]);
			}
		}
	}
}

bool PlayerRankCompareFunction(const ConstCommonUserPtr a, const ConstCommonUserPtr b) // should never operate on nulls. Hence, ASSERT_LOGIC is appropriate here.
{
	ASSERT_LOGIC(a, "fail in Autobalance, NULL player");
	ASSERT_LOGIC(b, "fail in Autobalance, NULL player");
	return (a->GetBalanceRank() > b->GetBalanceRank());
}

bool PlayerTeamCompareFunction(const ConstCommonUserPtr a, const ConstCommonUserPtr b) // should never operate on nulls. Hence, ASSERT_LOGIC is appropriate here.
{
	ASSERT_LOGIC(a, "fail in Autobalance, NULL player");
	ASSERT_LOGIC(b, "fail in Autobalance, NULL player");
	return (a->BattleStatus().team > b->BattleStatus().team);
}

struct Alliance {
	CommonUserVector players;
	float ranksum;
	int allynum;
	Alliance()
	    : ranksum(0)
	    , allynum(-1)
	{
	}
	Alliance(int i)
	    : ranksum(0)
	    , allynum(i)
	{
	}
	void AddPlayer(const CommonUserPtr player)
	{
		if (player) {
			players.push_back(player);
			ranksum += player->GetBalanceRank();
		}
	}
	void AddAlliance(const Alliance& other)
	{
		for (CommonUserVector::const_iterator i = other.players.begin(); i != other.players.end(); ++i)
			AddPlayer(*i);
	}
	bool operator<(const Alliance& other) const
	{
		return ranksum < other.ranksum;
	}
};

struct ControlTeam {
	CommonUserVector players;
	float ranksum;
	int teamnum;
	ControlTeam()
	    : ranksum(0)
	    , teamnum(-1)
	{
	}
	ControlTeam(int i)
	    : ranksum(0)
	    , teamnum(i)
	{
	}
	void AddPlayer(const CommonUserPtr player)
	{
		if (player) {
			players.push_back(player);
			ranksum += player->GetBalanceRank();
		}
	}
	void AddTeam(ControlTeam& other)
	{
		for (CommonUserVector::const_iterator i = other.players.begin(); i != other.players.end(); ++i)
			AddPlayer(*i);
	}
	bool operator<(const ControlTeam& other) const
	{
		return ranksum < other.ranksum;
	}
};

int my_random(int range)
{
	return rand() % range;
}

void shuffle(CommonUserVector& players) // proper shuffle.
{
	for (size_t i = 0; i < players.size(); ++i) // the players below i are shuffled, the players above arent
	{
		int rn = i + my_random(players.size() - i); // the top of shuffled part becomes random card from unshuffled part
		CommonUserPtr tmp = players[i];
		players[i] = players[rn];
		players[rn] = tmp;
	}
}

/*
bool ClanRemovalFunction(const std::map<std::string, Alliance>::value_type &v){
  return v.second.players.size()<2;
}
*/
/*
struct ClannersRemovalPredicate{
  std::map<std::string, Alliance> &clans;
  PlayerRemovalPredicate(std::map<std::string, Alliance> &clans_):clans(clans_)
  {
  }
  bool operator()(User *u) const{
    return clans.find(u->GetClan());
  }
}*/

void Battle::Autobalance(Enum::BalanceType balance_type, bool support_clans, bool strong_clans, int numallyteams)
{
	//    lslDebug("Autobalancing alliances, type=%d, clans=%d, strong_clans=%d, numallyteams=%d",balance_type, support_clans,strong_clans, numallyteams);

	std::vector<Alliance> alliances;
	if (numallyteams == 0 || numallyteams == -1) // 0 or 1 -> use num start rects
	{
		int ally = 0;
		for (unsigned int i = 0; i < GetNumRects(); ++i) {
			BattleStartRect sr = GetStartRect(i);
			if (sr.IsOk()) {
				ally = i;
				alliances.push_back(Alliance(ally));
				ally++;
			}
		}
		// make at least two alliances
		while (alliances.size() < 2) {
			alliances.push_back(Alliance(ally));
			ally++;
		}
	} else {
		for (int i = 0; i < numallyteams; i++)
			alliances.push_back(Alliance(i));
	}

	//for(i=0;i<alliances.size();++i)alliances[i].allynum=i;

	CommonUserVector players_sorted;
	players_sorted.reserve(m_userlist.size());

	for (size_t i = 0; i < m_userlist.size(); ++i) {
		CommonUserPtr usr = m_userlist[i];
		if (!usr->BattleStatus().spectator) {
			players_sorted.push_back(usr);
		}
	}

	// remove players in the same team so only one remains
	std::map<int, CommonUserPtr> dedupe_teams;
	for (std::vector<CommonUserPtr>::const_iterator it = players_sorted.begin(); it != players_sorted.end(); ++it) {
		dedupe_teams[(*it)->BattleStatus().team] = *it;
	}
	players_sorted.clear();
	players_sorted.reserve(dedupe_teams.size());
	for (std::map<int, CommonUserPtr>::const_iterator it = dedupe_teams.begin(); it != dedupe_teams.end(); ++it) {
		players_sorted.push_back(it->second);
	}

	shuffle(players_sorted);

	std::map<std::string, Alliance> clan_alliances;
	if (support_clans) {
		for (size_t i = 0; i < players_sorted.size(); ++i) {
			std::string clan = players_sorted[i]->GetClan();
			if (!clan.empty()) {
				clan_alliances[clan].AddPlayer(players_sorted[i]);
			}
		}
	};

	if (balance_type != Enum::balance_random)
		std::sort(players_sorted.begin(), players_sorted.end(), PlayerRankCompareFunction);

	if (support_clans) {
		std::map<std::string, Alliance>::iterator clan_it = clan_alliances.begin();
		while (clan_it != clan_alliances.end()) {
			Alliance& clan = (*clan_it).second;
			// if clan is too small (only 1 clan member in battle) or too big, dont count it as clan
			if ((clan.players.size() < 2) ||
			    (!strong_clans && (clan.players.size() >
					       ((players_sorted.size() + alliances.size() - 1) / alliances.size())))) {
				std::map<std::string, Alliance>::iterator next = clan_it;
				++next;
				clan_alliances.erase(clan_it);
				clan_it = next;
				continue;
			}
			std::sort(alliances.begin(), alliances.end());
			float lowestrank = alliances[0].ranksum;
			int rnd_k = 1; // number of alliances with rank equal to lowestrank
			while (size_t(rnd_k) < alliances.size()) {
				if (fabs(alliances[rnd_k].ranksum - lowestrank) > 0.01)
					break;
				rnd_k++;
			}
			alliances[my_random(rnd_k)].AddAlliance(clan);
			++clan_it;
		}
	}

	for (size_t i = 0; i < players_sorted.size(); ++i) {
		// skip clanners, those have been added already.
		if (clan_alliances.count(players_sorted[i]->GetClan()) > 0) {
			continue;
		}

		// find alliances with lowest ranksum
		// insert current user into random one out of them
		// since performance doesnt matter here, i simply sort alliances,
		// then find how many alliances in beginning have lowest ranksum
		// note that balance player ranks range from 1 to 1.1 now
		// i.e. them are quasi equal
		// so we're essentially adding to alliance with smallest number of players,
		// the one with smallest ranksum.

		std::sort(alliances.begin(), alliances.end());
		float lowestrank = alliances[0].ranksum;
		int rnd_k = 1; // number of alliances with rank equal to lowestrank
		while (size_t(rnd_k) < alliances.size()) {
			if (fabs(alliances[rnd_k].ranksum - lowestrank) > 0.01)
				break;
			rnd_k++;
		}
		alliances[my_random(rnd_k)].AddPlayer(players_sorted[i]);
	}

	const size_t totalplayers = m_userlist.size();
	for (size_t i = 0; i < alliances.size(); ++i) {
		for (size_t j = 0; j < alliances[i].players.size(); ++j) {
			ASSERT_LOGIC(alliances[i].players[j], "fail in Autobalance, NULL player");
			int balanceteam = alliances[i].players[j]->BattleStatus().team;
			for (size_t h = 0; h < totalplayers; h++) // change ally num of all players in the team
			{
				CommonUserPtr usr = m_userlist[h];
				if (usr->BattleStatus().team == balanceteam)
					ForceAlly(usr, alliances[i].allynum);
			}
		}
	}
}

void Battle::FixTeamIDs(Enum::BalanceType balance_type, bool support_clans, bool strong_clans, int numcontrolteams)
{
	//	wxLogMessage("Autobalancing teams, type=%d, clans=%d, strong_clans=%d, numcontrolteams=%d",balance_type, support_clans, strong_clans, numcontrolteams);
	std::vector<ControlTeam> control_teams;

	if (numcontrolteams == 0 || numcontrolteams == -1)
		numcontrolteams = m_userlist.size() - GetSpectators(); // 0 or -1 -> use num players, will use comshare only if no available team slots
	Enum::StartType position_type = (Enum::StartType)
	    Util::FromIntString(CustomBattleOptions()->getSingleValue("startpostype", LSL::Enum::EngineOption));
	if ((position_type == Enum::ST_Fixed) || (position_type == Enum::ST_Random)) // if fixed start pos type or random, use max teams = start pos count
	{
		try {
			const int mapposcount = LoadMap().info.positions.size();
			numcontrolteams = std::min(numcontrolteams, mapposcount);
		} catch (...) {
		}
	}

	if (numcontrolteams >= (int)(m_userlist.size() - GetSpectators())) // autobalance behaves weird when trying to put one player per team and i CBA to fix it, so i'll reuse the old code :P
	{
		// apparently tasserver doesnt like when i fix/force ids of everyone.
		std::set<int> allteams;
		const size_t numusers = m_userlist.size();
		for (size_t i = 0; i < numusers; ++i) {
			const CommonUserPtr user = m_userlist.At(i);
			if (!user->BattleStatus().spectator)
				allteams.insert(user->BattleStatus().team);
		}
		std::set<int> teams;
		int t = 0;
		for (size_t i = 0; i < m_userlist.size(); ++i) {
			const CommonUserPtr user = m_userlist.At(i);
			if (!user->BattleStatus().spectator) {
				if (teams.count(user->BattleStatus().team)) {
					while (allteams.count(t) || teams.count(t))
						t++;
					ForceTeam(m_userlist.At(i), t);
					teams.insert(t);
				} else {
					teams.insert(user->BattleStatus().team);
				}
			}
		}
		return;
	}
	for (int i = 0; i < numcontrolteams; i++)
		control_teams.push_back(ControlTeam(i));

	std::vector<CommonUserPtr> players_sorted;
	players_sorted.reserve(m_userlist.size());

	int player_team_counter = 0;

	for (size_t i = 0; i < m_userlist.size(); ++i) // don't count spectators
	{
		if (!m_userlist.At(i)->BattleStatus().spectator) {
			players_sorted.push_back(m_userlist.At(i));
			// -- server fail? it doesnt work right.
			//ForceTeam(m_userlist.At(i),player_team_counter);
			player_team_counter++;
		}
	}

	shuffle(players_sorted);

	std::map<std::string, ControlTeam> clan_teams;
	if (support_clans) {
		for (size_t i = 0; i < players_sorted.size(); ++i) {
			std::string clan = players_sorted[i]->GetClan();
			if (!clan.empty()) {
				clan_teams[clan].AddPlayer(players_sorted[i]);
			}
		}
	};

	if (balance_type != Enum::balance_random)
		std::sort(players_sorted.begin(), players_sorted.end(), PlayerRankCompareFunction);

	if (support_clans) {
		std::map<std::string, ControlTeam>::iterator clan_it = clan_teams.begin();
		while (clan_it != clan_teams.end()) {
			ControlTeam& clan = (*clan_it).second;
			// if clan is too small (only 1 clan member in battle) or too big, dont count it as clan
			if ((clan.players.size() < 2) || (!strong_clans && (clan.players.size() > ((players_sorted.size() + control_teams.size() - 1) / control_teams.size())))) {
				//				wxLogMessage("removing clan %s",(*clan_it).first.c_str());
				std::map<std::string, ControlTeam>::iterator next = clan_it;
				++next;
				clan_teams.erase(clan_it);
				clan_it = next;
				continue;
			}
			//			wxLogMessage( "Inserting clan %s", (*clan_it).first.c_str() );
			std::sort(control_teams.begin(), control_teams.end());
			float lowestrank = control_teams[0].ranksum;
			int rnd_k = 1; // number of alliances with rank equal to lowestrank
			while (size_t(rnd_k) < control_teams.size()) {
				if (fabs(control_teams[rnd_k].ranksum - lowestrank) > 0.01)
					break;
				rnd_k++;
			}
			//			wxLogMessage("number of lowestrank teams with same rank=%d", rnd_k );
			control_teams[my_random(rnd_k)].AddTeam(clan);
			++clan_it;
		}
	}

	for (size_t i = 0; i < players_sorted.size(); ++i) {
		// skip clanners, those have been added already.
		if (clan_teams.count(players_sorted[i]->GetClan()) > 0) {
			//			wxLogMessage( "clanner already added, nick=%s",players_sorted[i]->Nick().c_str() );
			continue;
		}

		// find teams with lowest ranksum
		// insert current user into random one out of them
		// since performance doesnt matter here, i simply sort teams,
		// then find how many teams in beginning have lowest ranksum
		// note that balance player ranks range from 1 to 1.1 now
		// i.e. them are quasi equal
		// so we're essentially adding to teams with smallest number of players,
		// the one with smallest ranksum.

		std::sort(control_teams.begin(), control_teams.end());
		float lowestrank = control_teams[0].ranksum;
		int rnd_k = 1; // number of alliances with rank equal to lowestrank
		while (size_t(rnd_k) < control_teams.size()) {
			if (fabs(control_teams[rnd_k].ranksum - lowestrank) > 0.01)
				break;
			rnd_k++;
		}
		//		wxLogMessage( "number of lowestrank teams with same rank=%d", rnd_k );
		control_teams[my_random(rnd_k)].AddPlayer(players_sorted[i]);
	}


	for (size_t i = 0; i < control_teams.size(); ++i) {
		for (size_t j = 0; j < control_teams[i].players.size(); ++j) {
			ASSERT_LOGIC(control_teams[i].players[j], "fail in Autobalance teams, NULL player");
			//			std::string msg = (boost::format( "setting player %s to team and ally %d" ) % control_teams[i].players[j]->Nick() % i).str();
			//			wxLogMessage( "%s", msg.c_str() );
			ForceTeam(control_teams[i].players[j], control_teams[i].teamnum);
			ForceAlly(control_teams[i].players[j], control_teams[i].teamnum);
		}
	}
}

void Battle::OnUnitsyncReloaded()
{
	//    IBattle::OnUnitsyncReloaded( data );
	if (m_is_self_in)
		SendMyBattleStatus();
}

void Battle::ShouldAutoUnspec()
{
	if (m_auto_unspec && !IsLocked() && GetMe()->BattleStatus().spectator) {
		if (GetNumActivePlayers() < m_opts.maxplayers) {
			ForceSpectator(GetMe(), false);
		}
	}
}

void Battle::SetChannel(const ChannelPtr channel)
{
	m_channel = channel;
}

void Battle::SetAutoUnspec(bool value)
{
	m_auto_unspec = value;
	ShouldAutoUnspec();
}

const ChannelPtr Battle::GetChannel()
{
	return m_channel;
}
} // namespace Battle {
} // namespace LSL {
