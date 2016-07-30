/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "ibattle.h"

#include <lslutils/debug.h>
#include <lslutils/misc.h>
#include <lslutils/conversion.h>
#include <lslutils/config.h>
#include <lslutils/autopointers.h>
#include <lslunitsync/unitsync.h>
#include <lslunitsync/optionswrapper.h>

//SL includes -- bad
#if HAVE_SPRINGLOBBY
#include "settings.h"
#else
#include <lslutils/config.h>
#endif

#include "signals.h"
#include "tdfcontainer.h"

#include <algorithm>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#define ASSERT_EXCEPTION(cond, msg)             \
	do {                                    \
		if (!(cond)) {                  \
			LSL_THROW(battle, msg); \
		}                               \
	} while (0)

namespace LSL
{
namespace Battle
{

boost::asio::io_service _io;
const boost::posix_time::seconds TIMER_INTERVAL(1000);
const unsigned int TIMER_ID = 101;

BattleOptions::BattleOptions()
    : battleid(-1)
    , battletype(Enum::BT_Played)
    , islocked(false)
    , ispassworded(false)
    , rankneeded(0)
    , lockexternalbalancechanges(false)
    , proxyhost("")
    , userelayhost(false)
    , relayhost("")
    , nattype(Enum::NAT_None)
    , port(Enum::DEFAULT_SERVER_PORT)
    , externaludpsourceport(Enum::DEFAULT_EXTERNAL_UDP_SOURCE_PORT)
    , internaludpsourceport(Enum::DEFAULT_EXTERNAL_UDP_SOURCE_PORT)
    , maxplayers(0)
    , spectators(0)
{
}

IBattle::IBattle()
    : m_map_loaded(false)
    , m_mod_loaded(false)
    , m_map_exists(false)
    , m_game_exists(false)
    , m_previous_local_mod_name(std::string())
    , m_opt_wrap(new OptionsWrapper())
    , m_ingame(false)
    , m_players_ready(0)
    , m_players_sync(0)
    , m_players_ok(0)
    , m_is_self_in(false)
    , m_generating_script(false)
    , m_timer(new boost::asio::deadline_timer(_io, TIMER_INTERVAL))
{
}


IBattle::~IBattle()
{
	m_timer->cancel();
	if (m_is_self_in)
		usync().UnSetCurrentArchive();
}

bool IBattle::IsSynced()
{
	LoadMod();
	LoadMap();
	bool synced = true;
	if (!m_host_map.hash.empty() || m_host_map.hash != "0")
		synced = synced && (m_local_map.hash == m_host_map.hash);
	if (!m_host_map.name.empty())
		synced = synced && (m_local_map.name == m_host_map.name);
	if (!m_host_game.hash.empty() || m_host_game.hash != "0")
		synced = synced && (m_local_game.hash == m_host_game.hash);
	if (!m_host_game.name.empty())
		synced = synced && (m_local_game.name == m_host_game.name);
	return synced;
}

std::vector<lslColor> IBattle::GetFixColorsPalette(int numteams) const
{
	return Util::GetBigFixColorsPalette(numteams);
}

lslColor IBattle::GetFixColor(int i) const
{
	int size = m_teams_sizes.size();
	std::vector<lslColor> palette = GetFixColorsPalette(size);
	return palette[i];
}

int IBattle::GetPlayerNum(const ConstCommonUserPtr user) const
{
	for (size_t i = 0; i < m_userlist.size(); ++i) {
		//is this wise? userlist is a map that changes order on insert
		if (m_userlist.At(i) == user)
			return i;
	}

	ASSERT_EXCEPTION(false, "The player is not in this game.");
	return lslNotFound;
}

class DismissColor
{
protected:
	typedef std::vector<lslColor>
	    ColorVec;
	const ColorVec& m_other;

public:
	DismissColor(const ColorVec& other)
	    : m_other(other)
	{
	}

	bool operator()(lslColor to_check)
	{
		return std::find(m_other.begin(), m_other.end(), to_check) != m_other.end();
	}
};

class AreColorsSimilarProxy
{
	const int m_mindiff;

public:
	AreColorsSimilarProxy(int mindiff)
	    : m_mindiff(mindiff)
	{
	}

	bool operator()(lslColor a, lslColor b)
	{
		return Util::AreColorsSimilar(a, b, m_mindiff);
	}
};

lslColor IBattle::GetFreeColor(const ConstCommonUserPtr user) const
{
	typedef std::vector<lslColor>
	    ColorVec;

	ColorVec current_used_colors;
	for (size_t i = 0; i < m_userlist.size(); ++i) {
		const UserBattleStatus& bs = m_userlist.At(i)->BattleStatus();
		current_used_colors.push_back(bs.color);
	}

	int inc = 1;
	while (true) {
		ColorVec fixcolorspalette = GetFixColorsPalette(m_teams_sizes.size() + inc++);
		ColorVec::iterator fixcolorspalette_new_end; //= std::unique( fixcolorspalette.begin(), fixcolorspalette.end(), AreColorsSimilarProxy( 20 ) );
		fixcolorspalette_new_end = std::remove_if(fixcolorspalette.begin(), fixcolorspalette.end(), DismissColor(current_used_colors));
		if (fixcolorspalette_new_end != fixcolorspalette.begin())
			return (*fixcolorspalette.begin());
	}
}

lslColor IBattle::GetNewColor() const
{
	return GetFreeColor();
}

int IBattle::ColorDifference(const lslColor& a, const lslColor& b) const // returns max difference of r,g,b.
{
	return std::max(abs(a.Red() - b.Red()), std::max(abs(a.Green() - b.Green()), abs(a.Blue() - b.Blue())));
}

int IBattle::GetFreeTeam(bool excludeme) const
{
	int lowest = 0;
	bool changed = true;
	while (changed) {
		changed = false;
		for (size_t i = 0; i < m_userlist.size(); i++) {
			const ConstCommonUserPtr user = m_userlist.At(i);
			if ((user == GetMe()) && excludeme)
				continue;
			if (user->BattleStatus().spectator)
				continue;
			if (user->BattleStatus().team == lowest) {
				lowest++;
				changed = true;
			}
		}
	}
	return lowest;
}

int IBattle::GetClosestFixColor(const lslColor& col, const std::vector<int>& excludes, int difference) const
{
	std::vector<lslColor> palette = GetFixColorsPalette(m_teams_sizes.size() + 1);
	int result = 0;
	for (size_t i = 0; i < palette.size(); ++i) {
		if ((i >= excludes.size()) || (!excludes[i])) {
			if (Util::AreColorsSimilar(palette[i], col, difference)) {
				return i;
			}
		}
	}
	return result;
}


void IBattle::SendHostInfo(Enum::HostInfo /*unused*/)
{
}

void IBattle::SendHostInfo(const std::string& /*unused*/)
{
}

void IBattle::Update(const std::string& /*unused*/)
{
}

void IBattle::OnUserAdded(const CommonUserPtr user)
{
	UserBattleStatus& bs = user->BattleStatus();
	bs.spectator = false;
	bs.ready = false;
	bs.sync = SYNC_UNKNOWN;
	if (!bs.IsBot() && IsFounderMe() && GetBattleType() == Enum::BT_Played) {
		bs.team = GetFreeTeam(user == GetMe());
		bs.ally = GetFreeAlly(user == GetMe());
		bs.color = GetFreeColor(user);
	}
	if (IsFounderMe() && ((bs.pos.x < 0) || (bs.pos.y < 0))) {
		UserPosition& pos = bs.pos;
		pos = GetFreePosition();
		UserPositionChanged(user);
	}
	if (!bs.spectator) {
		PlayerJoinedAlly(bs.ally);
		PlayerJoinedTeam(bs.team);
	}
	if (bs.spectator && IsFounderMe())
		m_opts.spectators++;
	if (!bs.spectator && !bs.IsBot()) {
		if (bs.ready)
			m_players_ready++;
		if (bs.sync)
			m_players_sync++;
		if (!bs.ready || !bs.sync)
			m_ready_up_map[user->Nick()] = time(0);
		if (bs.ready && bs.sync)
			m_players_ok++;
	}
}

CommonUserPtr IBattle::OnBotAdded(const std::string& nick, const UserBattleStatus& bs)
{
	CommonUserPtr bot(new CommonUser(User::GetNewUserId(), nick));
	m_internal_bot_list.Add(bot);
	bot->UpdateBattleStatus(bs);
	return bot;
}

unsigned int IBattle::GetNumBots() const
{
	return m_internal_bot_list.size();
}

unsigned int IBattle::GetNumPlayers() const
{
	return m_userlist.size() - GetNumBots();
}

unsigned int IBattle::GetNumActivePlayers() const
{
	return GetNumPlayers() - m_opts.spectators;
}

void IBattle::OnUserBattleStatusUpdated(CommonUserPtr user, UserBattleStatus status)
{
	UserBattleStatus previousstatus = user->BattleStatus();

	user->UpdateBattleStatus(status);
	unsigned int oldspeccount = m_opts.spectators;
	m_opts.spectators = 0;
	m_players_sync = 0;
	m_players_ready = 0;
	m_players_ok = 0;
	m_teams_sizes.clear();
	m_ally_sizes.clear();
	for (unsigned int i = 0; i < m_userlist.size(); i++) {
		const ConstCommonUserPtr loopuser = m_userlist.At(i);
		const UserBattleStatus& loopstatus = loopuser->BattleStatus();
		if (loopstatus.spectator)
			m_opts.spectators++;
		if (!loopstatus.IsBot()) {
			if (!loopstatus.spectator) {
				if (loopstatus.ready && loopstatus.spectator)
					m_players_ready++;
				if (loopstatus.sync)
					m_players_sync++;
				if (loopstatus.ready && loopstatus.sync)
					m_players_ok++;
				PlayerJoinedTeam(loopstatus.team);
				PlayerJoinedAlly(loopstatus.ally);
			}
		}
	}
	if (oldspeccount != m_opts.spectators) {
		if (IsFounderMe())
			SendHostInfo(Enum::HI_Spectators);
	}
	if (!status.IsBot()) {
		if ((status.ready && status.sync) || status.spectator) {
			std::map<std::string, time_t>::iterator itor = m_ready_up_map.find(user->Nick());
			if (itor != m_ready_up_map.end()) {
				m_ready_up_map.erase(itor);
			}
		}
		if ((!status.ready || !status.sync) && !status.spectator) {
			std::map<std::string, time_t>::iterator itor = m_ready_up_map.find(user->Nick());
			if (itor == m_ready_up_map.end()) {
				m_ready_up_map[user->Nick()] = time(0);
			}
		}
	}
}

bool IBattle::ShouldAutoStart() const
{
	if (InGame())
		return false;
	if (!IsLocked() && (GetNumActivePlayers() < m_opts.maxplayers))
		return false; // proceed checking for ready & symc players only if the battle is full or locked
	if (!IsEveryoneReady())
		return false;
	return true;
}

void IBattle::OnUserRemoved(CommonUserPtr user)
{
	UserBattleStatus& bs = user->BattleStatus();
	if (!bs.spectator) {
		PlayerLeftTeam(bs.team);
		PlayerLeftAlly(bs.ally);
	}
	if (!bs.spectator && !bs.IsBot()) {
		if (bs.ready)
			m_players_ready--;
		if (bs.sync)
			m_players_sync--;
		if (bs.ready && bs.sync)
			m_players_ok--;
	}
	if (IsFounderMe() && bs.spectator) {
		m_opts.spectators--;
		SendHostInfo(Enum::HI_Spectators);
	}
	if (user == GetMe()) {
		OnSelfLeftBattle();
	}
	m_userlist.Remove(user->Nick());
	if (!bs.IsBot())
		user->SetBattle(IBattlePtr());
	else {
		m_internal_bot_list.Remove(user->Nick());
	}
}


bool IBattle::IsEveryoneReady() const
{
	for (unsigned int i = 0; i < GetNumPlayers(); i++) {
		const ConstCommonUserPtr usr = m_userlist.At(i);
		const UserBattleStatus& status = usr->BattleStatus();
		if (status.IsBot())
			continue;
		if (status.spectator)
			continue;
		if (usr == GetMe())
			continue;
		if (!status.ready)
			return false;
		if (!status.sync)
			return false;
	}
	return true;
}

void IBattle::AddStartRect(unsigned int allyno, unsigned int left, unsigned int top, unsigned int right, unsigned int bottom)
{
	BattleStartRect sr;
	sr.ally = allyno;
	sr.left = left;
	sr.top = top;
	sr.right = right;
	sr.bottom = bottom;
	sr.toadd = true;
	sr.todelete = false;
	sr.toresize = false;
	sr.exist = true;
	m_rects[allyno] = sr;
}

void IBattle::RemoveStartRect(unsigned int allyno)
{
	std::map<unsigned int, BattleStartRect>::iterator rect_it = m_rects.find(allyno);
	if (rect_it == m_rects.end())
		return;

	rect_it->second.todelete = true;
	//BattleStartRect sr = m_rects[allyno];
	//sr.todelete = true;
	//m_rects[allyno] = sr;
}

void IBattle::ResizeStartRect(unsigned int allyno)
{
	std::map<unsigned int, BattleStartRect>::iterator rect_it = m_rects.find(allyno);
	if (rect_it == m_rects.end())
		return;

	rect_it->second.toresize = true;
	//BattleStartRect sr = m_rects[allyno];
	//&&sr.toresize = true;
	//m_rects[allyno] = sr;
}

void IBattle::StartRectRemoved(unsigned int allyno)
{
	std::map<unsigned int, BattleStartRect>::const_iterator rect_it = m_rects.find(allyno);
	if (rect_it == m_rects.end())
		return;

	if (rect_it->second.todelete)
		m_rects.erase(allyno);
}


void IBattle::StartRectResized(unsigned int allyno)
{
	std::map<unsigned int, BattleStartRect>::iterator rect_it = m_rects.find(allyno);
	if (rect_it == m_rects.end())
		return;

	rect_it->second.toresize = false;
	//BattleStartRect sr = m_rects[allyno];
	//sr.toresize = false;
	//m_rects[allyno] = sr;
}


void IBattle::StartRectAdded(unsigned int allyno)
{
	std::map<unsigned int, BattleStartRect>::iterator rect_it = m_rects.find(allyno);
	if (rect_it == m_rects.end())
		return;

	rect_it->second.toadd = false;
	//BattleStartRect sr = m_rects[allyno];
	//sr.toadd = false;
	//m_rects[allyno] = sr;
}


BattleStartRect IBattle::GetStartRect(unsigned int allyno) const
{
	std::map<unsigned int, BattleStartRect>::const_iterator rect_it = m_rects.find(allyno);
	if (rect_it != m_rects.end())
		return (*rect_it).second;
	return BattleStartRect();
}

//total number of start rects
unsigned int IBattle::GetNumRects() const
{
	return m_rects.size();
}

//key of last start rect in the map
unsigned int IBattle::GetLastRectIdx() const
{
	if (GetNumRects() > 0)
		return m_rects.rbegin()->first;

	return 0;
}

//return  the lowest currently unused key in the map of rects.
unsigned int IBattle::GetNextFreeRectIdx() const
{
	//check for unused allyno keys
	for (unsigned int i = 0; i <= GetLastRectIdx(); i++) {
		if (!GetStartRect(i).IsOk())
			return i;
	}
	return GetNumRects(); //if all rects are in use, or no elements exist, return first possible available allyno.
}

void IBattle::ClearStartRects()
{
	m_rects.clear();
}

void IBattle::ForceSide(const CommonUserPtr user, int side)
{
	if (IsFounderMe() || user->BattleStatus().IsBot()) {
		user->BattleStatus().side = side;
	}
}

void IBattle::ForceTeam(const CommonUserPtr user, int team)
{
	if (IsFounderMe() || user->BattleStatus().IsBot()) {
		if (!user->BattleStatus().spectator) {
			PlayerLeftTeam(user->BattleStatus().team);
			PlayerJoinedTeam(team);
		}
		user->BattleStatus().team = team;
	}
}


void IBattle::ForceAlly(const CommonUserPtr user, int ally)
{

	if (IsFounderMe() || user->BattleStatus().IsBot()) {
		if (!user->BattleStatus().spectator) {
			PlayerLeftAlly(user->BattleStatus().ally);
			PlayerJoinedAlly(ally);
		}
		user->BattleStatus().ally = ally;
	}
}


void IBattle::ForceColor(const CommonUserPtr user, const lslColor& col)
{
	if (IsFounderMe() || user->BattleStatus().IsBot()) {
		user->BattleStatus().color = col;
	}
}

void IBattle::PlayerJoinedTeam(int team)
{
	std::map<int, int>::const_iterator itor = m_teams_sizes.find(team);
	if (itor == m_teams_sizes.end())
		m_teams_sizes[team] = 1;
	else
		m_teams_sizes[team] = m_teams_sizes[team] + 1;
}

void IBattle::PlayerJoinedAlly(int ally)
{
	std::map<int, int>::const_iterator iter = m_ally_sizes.find(ally);
	if (iter == m_ally_sizes.end())
		m_ally_sizes[ally] = 1;
	else
		m_ally_sizes[ally] = m_ally_sizes[ally] + 1;
}

void IBattle::PlayerLeftTeam(int team)
{
	std::map<int, int>::iterator itor = m_teams_sizes.find(team);
	if (itor != m_teams_sizes.end()) {
		itor->second = itor->second - 1;
		if (itor->second == 0) {
			m_teams_sizes.erase(itor);
		}
	}
}

void IBattle::PlayerLeftAlly(int ally)
{
	std::map<int, int>::iterator iter = m_ally_sizes.find(ally);
	if (iter != m_ally_sizes.end()) {
		iter->second = iter->second - 1;
		if (iter->second == 0) {
			m_ally_sizes.erase(iter);
		}
	}
}

void IBattle::ForceSpectator(const CommonUserPtr user, bool spectator)
{
	if (IsFounderMe() || user->BattleStatus().IsBot()) {
		UserBattleStatus& status = user->BattleStatus();

		if (!status.spectator) // leaving spectator status
		{
			PlayerJoinedTeam(status.team);
			PlayerJoinedAlly(status.ally);
			if (status.ready && !status.IsBot())
				m_players_ready++;
		}

		if (spectator) // entering spectator status
		{
			PlayerLeftTeam(status.team);
			PlayerLeftAlly(status.ally);
			if (status.ready && !status.IsBot())
				m_players_ready--;
		}

		if (IsFounderMe()) {
			if (status.spectator != spectator) {
				if (spectator) {
					m_opts.spectators++;
				} else {
					m_opts.spectators--;
				}
				SendHostInfo(Enum::HI_Spectators);
			}
		}
		user->BattleStatus().spectator = spectator;
	}
}

void IBattle::SetHandicap(const CommonUserPtr user, int handicap)
{
	if (IsFounderMe() || user->BattleStatus().IsBot()) {
		user->BattleStatus().handicap = handicap;
	}
}


void IBattle::KickPlayer(const CommonUserPtr user)
{
	if (IsFounderMe() || user->BattleStatus().IsBot()) {
		OnUserRemoved(user);
	}
}

int IBattle::GetFreeAlly(bool excludeme) const
{
	int lowest = 0;
	bool changed = true;
	while (changed) {
		changed = false;
		for (unsigned int i = 0; i < m_userlist.size(); i++) {
			const ConstCommonUserPtr user = m_userlist.At(i);
			if ((user == GetMe()) && excludeme)
				continue;
			if (user->BattleStatus().spectator)
				continue;
			if (user->BattleStatus().ally == lowest) {
				lowest++;
				changed = true;
			}
		}
	}
	return lowest;
}

UserPosition IBattle::GetFreePosition()
{
	UserPosition ret;
	UnitsyncMap map = LoadMap();
	for (int i = 0; i < int(map.info.positions.size()); i++) {
		bool taken = false;
		for (unsigned int bi = 0; bi < m_userlist.size(); bi++) {
			ConstCommonUserPtr user = m_userlist.At(bi);
			const UserBattleStatus& status = user->BattleStatus();
			if (status.spectator)
				continue;
			if ((map.info.positions[i].x == status.pos.x) && (map.info.positions[i].y == status.pos.y)) {
				taken = true;
				break;
			}
		}
		if (!taken) {
			ret.x = Util::Clamp(map.info.positions[i].x, 0, map.info.width);
			ret.y = Util::Clamp(map.info.positions[i].y, 0, map.info.height);
			return ret;
		}
	}
	ret.x = map.info.width / 2;
	ret.y = map.info.height / 2;
	return ret;
}


void IBattle::SetHostMap(const std::string& mapname, const std::string& hash)
{
	if (mapname != m_host_map.name || hash != m_host_map.hash) {
		m_map_loaded = false;
		m_host_map.name = mapname;
		m_host_map.hash = hash;
		if (!m_host_map.hash.empty())
			m_map_exists = usync().MapExists(m_host_map.name, m_host_map.hash);
		else
			m_map_exists = usync().MapExists(m_host_map.name);
		if (m_map_exists)
			usync().PrefetchMap(m_host_map.name);
	}
}


void IBattle::SetLocalMap(const UnitsyncMap& map)
{
	if (map.name != m_local_map.name || map.hash != m_local_map.hash) {
		m_local_map = map;
		m_map_loaded = true;
		if (!m_host_map.hash.empty())
			m_map_exists = usync().MapExists(m_host_map.name, m_host_map.hash);
		else
			m_map_exists = usync().MapExists(m_host_map.name);
		if (m_map_exists)
			usync().PrefetchMap(m_host_map.name);
		if (IsFounderMe()) // save all rects infos
		{
		}
	}
}

const UnitsyncMap& IBattle::LoadMap()
{
	if (!m_map_loaded) {
		try {
			ASSERT_EXCEPTION(m_map_exists, "Map does not exist.");
			//			m_local_map = usync().GetMapEx( m_host_map.name );
			bool options_loaded = CustomBattleOptions()->loadOptions(LSL::Enum::MapOption, m_host_map.name);
			ASSERT_EXCEPTION(options_loaded, "couldn't load the map options");
			m_map_loaded = true;

		} catch (...) {
		}
	}
	return m_local_map;
}

std::string IBattle::GetHostMapName() const
{
	return m_host_map.name;
}

std::string IBattle::GetHostMapHash() const
{
	return m_host_map.hash;
}

void IBattle::SetHostMod(const std::string& modname, const std::string& hash)
{
	if (m_host_game.name != modname || m_host_game.hash != hash) {
		m_mod_loaded = false;
		m_host_game.name = modname;
		m_host_game.hash = hash;
		if (!m_host_game.hash.empty())
			m_game_exists = usync().GameExists(m_host_game.name, m_host_game.hash);
		else
			m_game_exists = usync().GameExists(m_host_game.name);
	}
}

void IBattle::SetLocalMod(const UnitsyncGame& mod)
{
	if (mod.name != m_local_game.name || mod.hash != m_local_game.hash) {
		m_previous_local_mod_name = m_local_game.name;
		m_local_game = mod;
		m_mod_loaded = true;
		if (!m_host_game.hash.empty())
			m_game_exists = usync().GameExists(m_host_game.name, m_host_game.hash);
		else
			m_game_exists = usync().GameExists(m_host_game.name);
	}
}

const UnitsyncGame& IBattle::LoadMod()
{
	if (!m_mod_loaded) {
		try {
			ASSERT_EXCEPTION(m_game_exists, "Mod does not exist.");
			m_local_game = usync().GetGame(m_host_game.name);
			bool options_loaded = CustomBattleOptions()->loadOptions(LSL::Enum::ModOption, m_host_game.name);
			ASSERT_EXCEPTION(options_loaded, "couldn't load the mod options");
			m_mod_loaded = true;
		} catch (...) {
		}
	}
	return m_local_game;
}

std::string IBattle::GetHostGameName() const
{
	return m_host_game.name;
}


std::string IBattle::GetHostGameHash() const
{
	return m_host_game.hash;
}


bool IBattle::MapExists() const
{
	return m_map_exists;
	//return usync().MapExists( m_map_name, m_map.hash );
}

bool IBattle::GameExists() const
{
	return m_game_exists;
	//return usync().GameExists( m_mod_name );
}

void IBattle::RestrictUnit(const std::string& unitname, int count)
{
	m_restricted_units[unitname] = count;
}

void IBattle::UnrestrictUnit(const std::string& unitname)
{
	std::map<std::string, int>::iterator pos = m_restricted_units.find(unitname);
	if (pos == m_restricted_units.end())
		return;
	m_restricted_units.erase(pos);
}

void IBattle::UnrestrictAllUnits()
{
	m_restricted_units.clear();
}

std::map<std::string, int> IBattle::RestrictedUnits() const
{
	return m_restricted_units;
}

void IBattle::OnSelfLeftBattle()
{
	GetMe()->BattleStatus().spectator = false; // always reset back yourself to player when rejoining
	m_timer->cancel();

	m_is_self_in = false;
	for (size_t j = 0; j < m_userlist.size(); ++j) {
		CommonUserPtr u = m_userlist.At(j);
		if (u->BattleStatus().IsBot()) {
			OnUserRemoved(u);
			Signals::sig_UserLeftBattle(shared_from_this(), u, true);
			j--;
		}
	}
	ClearStartRects();
	m_teams_sizes.clear();
	m_ally_sizes.clear();
	m_players_ready = 0;
	m_players_sync = 0;
	m_players_ok = 0;
	usync().UnSetCurrentArchive(); //left battle
}

void IBattle::OnUnitsyncReloaded()
{
	if (!m_host_game.hash.empty())
		m_game_exists = usync().GameExists(m_host_game.name, m_host_game.hash);
	else
		m_game_exists = usync().GameExists(m_host_game.name);
	if (!m_host_map.hash.empty())
		m_map_exists = usync().MapExists(m_host_map.name, m_host_map.hash);
	else
		m_map_exists = usync().MapExists(m_host_map.name);
}

static std::string FixPresetName(const std::string& name)
{
	// look name up case-insensitively
	const StringVector& presetList = Util::config().GetPresetList();
	const int index = Util::IndexInSequenceIf(presetList, Util::Predicates::CaseInsensitive(name));
	if (index == lslNotFound)
		return "";
	// set preset to the actual name, with correct case
	return presetList[index];
}

bool IBattle::LoadOptionsPreset(const std::string& name)
{
	std::string preset = FixPresetName(name);
	if (preset == "")
		return false; //preset not found
	m_preset = preset;

	for (unsigned int i = 0; i < LSL::Enum::LastOption; i++) {
		std::map<std::string, std::string> options = Util::config().GetHostingPreset(m_preset, i);
		if ((LSL::Enum::GameOption)i != LSL::Enum::PrivateOptions) {
			for (std::map<std::string, std::string>::const_iterator itor = options.begin(); itor != options.end(); ++itor) {
				CustomBattleOptions()->setSingleOption(itor->first, itor->second, (LSL::Enum::GameOption)i);
			}
		} else {
			if (!options["mapname"].empty()) {
				if (usync().MapExists(options["mapname"])) {
					/*					UnitsyncMap map = usync().GetMapEx( options["mapname"] );
					SetLocalMap( map );
					SendHostInfo( Enum::HI_Map );
*/
				}
				//				else if ( !ui().OnPresetRequiringMap( options["mapname"] ) ) {
				//					//user didn't want to download the missing map, so set to empty to not have it tried to be loaded again
				//					options["mapname"] = "";
				//					sett().SetHostingPreset( m_preset, i, options );
				//				}
			}

			for (unsigned int j = 0; j <= GetLastRectIdx(); ++j) {
				if (GetStartRect(j).IsOk())
					RemoveStartRect(j); // remove all rects that might come from map presets
			}
			SendHostInfo(Enum::HI_StartRects);

			unsigned int rectcount = Util::FromIntString(options["numrects"]);
			for (unsigned int loadrect = 0; loadrect < rectcount; loadrect++) {
				int ally = Util::FromIntString(options["rect_" + Util::ToIntString(loadrect) + "_ally"]);
				if (ally == 0)
					continue;
				AddStartRect(ally - 1, Util::FromIntString(options["rect_" + Util::ToIntString(loadrect) + "_left"]),
					     Util::FromIntString(options["rect_" + Util::ToIntString(loadrect) + "_top"]),
					     Util::FromIntString(options["rect_" + Util::ToIntString(loadrect) + "_right"]),
					     Util::FromIntString(options["rect_" + Util::ToIntString(loadrect) + "_bottom"]));
			}
			SendHostInfo(Enum::HI_StartRects);

			m_restricted_units.clear();
			const StringVector infos = Util::StringTokenize(options["restrictions"], "\t");
			for (const std::string unitinfo : infos) {
				RestrictUnit(Util::BeforeLast(unitinfo, "="),
					     Util::FromIntString(Util::AfterLast(unitinfo, "=")));
			}
			SendHostInfo(Enum::HI_Restrictions);
			Update((boost::format("%d_restrictions") % LSL::Enum::PrivateOptions).str());
		}
	}
	SendHostInfo(Enum::HI_Send_All_opts);
	Signals::sig_ReloadPresetList();
	return true;
}

void IBattle::SaveOptionsPreset(const std::string& name)
{
	//TODO presets needs to be modeled in a class of their own
	m_preset = FixPresetName(name);
	if (m_preset == "")
		m_preset = name; //new preset

	for (int i = 0; i < (int)Enum::LastOption; i++) {
		if ((LSL::Enum::GameOption)i != LSL::Enum::PrivateOptions) {
			Util::config().SetHostingPreset(m_preset, (LSL::Enum::GameOption)i, CustomBattleOptions()->getOptionsMap((LSL::Enum::GameOption)i));
		} else {
			std::map<std::string, std::string> opts;
			opts["mapname"] = GetHostMapName();
			unsigned int validrectcount = 0;
			if (Util::FromIntString(CustomBattleOptions()->getSingleValue("startpostype", LSL::Enum::EngineOption)) == Enum::ST_Choose) {
				unsigned int boxcount = GetLastRectIdx();
				for (unsigned int boxnum = 0; boxnum <= boxcount; boxnum++) {
					BattleStartRect rect = GetStartRect(boxnum);
					if (rect.IsOk()) {
						opts["rect_" + Util::ToIntString(validrectcount) + "_ally"] = Util::ToIntString(rect.ally + 1);
						opts["rect_" + Util::ToIntString(validrectcount) + "_left"] = Util::ToIntString(rect.left);
						opts["rect_" + Util::ToIntString(validrectcount) + "_top"] = Util::ToIntString(rect.top);
						opts["rect_" + Util::ToIntString(validrectcount) + "_bottom"] = Util::ToIntString(rect.bottom);
						opts["rect_" + Util::ToIntString(validrectcount) + "_right"] = Util::ToIntString(rect.right);
						validrectcount++;
					}
				}
			}
			opts["numrects"] = Util::ToIntString(validrectcount);

			std::stringstream restrictionsstring;
			for (std::map<std::string, int>::const_iterator itor = m_restricted_units.begin(); itor != m_restricted_units.end(); ++itor) {
				restrictionsstring << itor->first << '=' << Util::ToIntString(itor->second) << '\t';
			}
			opts["restrictions"] = restrictionsstring.str();

			Util::config().SetHostingPreset(m_preset, (LSL::Enum::GameOption)i, opts);
		}
	}
	Util::config().SaveSettings();
	Signals::sig_ReloadPresetList();
}

std::string IBattle::GetCurrentPreset()
{
	return m_preset;
}

void IBattle::DeletePreset(const std::string& name)
{
	std::string preset = FixPresetName(name);
	if (m_preset == preset)
		m_preset = "";
	Util::config().DeletePreset(preset);
	Signals::sig_ReloadPresetList();
}

StringVector IBattle::GetPresetList()
{
	return Util::config().GetPresetList();
}

void IBattle::UserPositionChanged(const CommonUserPtr /*unused*/)
{
}

void IBattle::AddUserFromDemo(CommonUserPtr user)
{
	user->BattleStatus().isfromdemo = true;
	m_internal_user_list.Add(user);
	m_userlist.Add(user);
}

void IBattle::SetProxy(const std::string& value)
{
	m_opts.proxyhost = value;
}

bool IBattle::IsProxy() const
{
	return !m_opts.proxyhost.empty();
}

std::string IBattle::GetProxy() const
{
	return m_opts.proxyhost;
}

bool IBattle::IsFounderMe() const
{
	return ((m_opts.founder == GetMe()->Nick()) || (IsProxy() && !m_generating_script));
}

bool IBattle::IsFounder(const CommonUserPtr user) const
{
	if (m_userlist.Exists(m_opts.founder)) {
		try {
			return GetFounder() == user;
		} catch (UserList::MissingItemException& m) {
			return false;
		}
	} else
		return false;
}

int IBattle::GetMyPlayerNum() const
{
	return GetPlayerNum(GetMe());
}


void IBattle::LoadScriptMMOpts(const std::string& sectionname, const TDF::PDataList& node)
{
	if (!node.ok())
		return;
	TDF::PDataList section(node->Find(sectionname));
	if (!section.ok())
		return;
	OptionsWrapperPtr opts = CustomBattleOptions();
	for (TDF::PNode n = section->First(); n != section->Last(); n = section->Next(n)) {
		if (!n.ok())
			continue;
		opts->setSingleOption(n->Name(), section->GetString(n->Name()));
	}
}

void IBattle::LoadScriptMMOpts(const TDF::PDataList& node)
{
	if (!node.ok())
		return;
	OptionsWrapperPtr opts = CustomBattleOptions();
	typedef std::map<std::string, std::string> optMap;
	optMap options = opts->getOptionsMap(LSL::Enum::EngineOption);
	for (optMap::const_iterator i = options.begin(); i != options.end(); ++i) {
		opts->setSingleOption(i->first, node->GetString(i->first, i->second));
	}
}

//! (koshi) don't delete commented things please, they might be need in the future and i'm lazy
void IBattle::GetBattleFromScript(bool loadmapmod)
{
	BattleOptions opts;
	std::stringstream ss(GetScript());
	TDF::PDataList script(TDF::ParseTDF(ss));

	TDF::PDataList replayNode(script->Find("GAME"));
	if (replayNode.ok()) {
		std::string modname = replayNode->GetString("GameType");
		std::string modhash = replayNode->GetString("ModHash");
		if (!modhash.empty())
			modhash = Util::MakeHashUnsigned(modhash);
		SetHostMod(modname, modhash);

		//don't have the maphash, what to do?
		//ui download function works with mapname if hash is empty, so works for now
		std::string mapname = replayNode->GetString("MapName");
		std::string maphash = replayNode->GetString("MapHash");
		if (!maphash.empty())
			maphash = Util::MakeHashUnsigned(maphash);
		SetHostMap(mapname, maphash);

		//        opts.ip         = replayNode->GetString( "HostIP" );
		//        opts.port       = replayNode->GetInt  ( "HostPort", DEFAULT_EXTERNAL_UDP_SOURCE_PORT );
		opts.spectators = 0;

		int playernum = replayNode->GetInt("NumPlayers", 0);
		int usersnum = replayNode->GetInt("NumUsers", 0);
		if (usersnum > 0)
			playernum = usersnum;
		//        int allynum = replayNode->GetInt  ( "NumAllyTeams", 1);
		//        int teamnum = replayNode->GetInt  ( "NumTeams", 1);

		StringVector sides;
		if (loadmapmod) {
			sides = usync().GetSides(modname);
		}

		IBattle::TeamVec parsed_teams = GetParsedTeamsVec();
		IBattle::AllyVec parsed_allies = GetParsedAlliesVec();

		//[PLAYERX] sections
		for (int i = 0; i < playernum; ++i) {
			TDF::PDataList player(replayNode->Find("PLAYER" + Util::ToIntString(i)));
			TDF::PDataList bot(replayNode->Find("AI" + Util::ToIntString(i)));
			if (player.ok() || bot.ok()) {
				if (bot.ok())
					player = bot;
				const CommonUserPtr user(new CommonUser(User::GetNewUserId(), player->GetString("Name"),
									boost::to_upper_copy(player->GetString("CountryCode"))));
				UserBattleStatus& status = user->BattleStatus();
				status.isfromdemo = true;
				status.spectator = player->GetInt("Spectator", 0);
				opts.spectators += user->BattleStatus().spectator;
				status.team = player->GetInt("Team");
				if (!status.spectator) {
					PlayerJoinedTeam(status.team);
				}
				status.sync = true;
				status.ready = true;
				if (status.spectator)
					m_opts.spectators++;
				else {
					if (!bot.ok()) {
						if (status.ready)
							m_players_ready++;
						if (status.sync)
							m_players_sync++;
						if (status.sync && status.ready)
							m_players_ok++;
					}
				}

				//! (koshi) changed this from ServerRankContainer to RankContainer
				user->Status().rank = (UserStatus::RankContainer)player->GetInt("Rank", -1);

				if (bot.ok()) {
					status.aishortname = bot->GetString("ShortName");
					status.aiversion = bot->GetString("Version");
					int ownerindex = bot->GetInt("Host");
					TDF::PDataList aiowner(replayNode->Find("PLAYER" + Util::ToIntString(ownerindex)));
					if (aiowner.ok()) {
						status.owner = aiowner->GetString("Name");
					}
				}

				IBattle::TeamInfoContainer teaminfos = parsed_teams[user->BattleStatus().team];
				if (!teaminfos.exist) {
					TDF::PDataList team(replayNode->Find("TEAM" + Util::ToIntString(user->BattleStatus().team)));
					if (team.ok()) {
						teaminfos.exist = true;
						teaminfos.TeamLeader = team->GetInt("TeamLeader", 0);
						teaminfos.StartPosX = team->GetInt("StartPosX", -1);
						teaminfos.StartPosY = team->GetInt("StartPosY", -1);
						teaminfos.AllyTeam = team->GetInt("AllyTeam", 0);
						teaminfos.RGBColor = lslColor::FromFloatString(team->GetString("RGBColor"));
						teaminfos.SideName = team->GetString("Side", "");
						teaminfos.Handicap = team->GetInt("Handicap", 0);
						int sidepos = Util::IndexInSequence(sides, teaminfos.SideName);
						teaminfos.SideNum = sidepos;
						parsed_teams[user->BattleStatus().team] = teaminfos;
					}
				}
				if (teaminfos.exist) {
					status.ally = teaminfos.AllyTeam;
					status.pos.x = teaminfos.StartPosX;
					status.pos.y = teaminfos.StartPosY;
					status.color = teaminfos.RGBColor;
					status.handicap = teaminfos.Handicap;
					if (!status.spectator) {
						PlayerJoinedAlly(status.ally);
					}
					if (teaminfos.SideNum >= 0)
						status.side = teaminfos.SideNum;
					IBattle::AllyInfoContainer allyinfos = parsed_allies[user->BattleStatus().ally];
					if (!allyinfos.exist) {
						TDF::PDataList ally(replayNode->Find("ALLYTEAM" + Util::ToIntString(user->BattleStatus().ally)));
						if (ally.ok()) {
							allyinfos.exist = true;
							allyinfos.NumAllies = ally->GetInt("NumAllies", 0);
							allyinfos.StartRectLeft = ally->GetInt("StartRectLeft", 0);
							allyinfos.StartRectTop = ally->GetInt("StartRectTop", 0);
							allyinfos.StartRectRight = ally->GetInt("StartRectRight", 0);
							allyinfos.StartRectBottom = ally->GetInt("StartRectBottom", 0);
							parsed_allies[user->BattleStatus().ally] = allyinfos;
							AddStartRect(user->BattleStatus().ally, allyinfos.StartRectTop, allyinfos.StartRectTop, allyinfos.StartRectRight, allyinfos.StartRectBottom);
						}
					}
				}

				AddUserFromDemo(user);
			}
		}
		SetParsedTeamsVec(parsed_teams);
		SetParsedAlliesVec(parsed_allies);

		//MMoptions, this'll fail unless loading map/mod into wrapper first
		if (loadmapmod) {
			LoadScriptMMOpts("mapoptions", replayNode);
			LoadScriptMMOpts("modoptions", replayNode);
		}

		opts.maxplayers = playernum;
	}
	SetBattleOptions(opts);
}

void IBattle::SetInGame(bool ingame)
{
	m_ingame = ingame;
	if (m_ingame)
		m_start_time = boost::posix_time::second_clock::universal_time();
	else
		m_start_time = boost::date_time::not_a_date_time;
}

long IBattle::GetBattleRunningTime() const
{
	if (!InGame())
		return 0;
	if (m_start_time == boost::date_time::not_a_date_time)
		return 0;
	boost::posix_time::time_duration td(
	    boost::posix_time::second_clock::universal_time() - m_start_time);
	return td.seconds();
}

CommonUserPtr IBattle::GetUser(const std::string& nick)
{
	return m_userlist.FindByNick(nick);
}

} // namespace Battle
} // namespace LSL
