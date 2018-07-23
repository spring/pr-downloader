/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "iserver.h"

#include "socket.h"
#include "commands.h"
#include "tasserverdataformats.h"

#include <lsl/battle/ibattle.h>
#include <lsl/user/user.h>

#include <boost/typeof/typeof.hpp>
#include <boost/algorithm/string.hpp>


namespace LSL
{

Server::Server()
    : m_impl(new ServerImpl(this))
{
	m_impl->m_sock->sig_doneConnecting.connect(
	    boost::bind(&Server::OnSocketConnected, this, _1, _2));
}

Server::~Server()
{
	delete m_impl->m_sock;
}

void Server::Connect(const std::string& /*servername */, const std::string& addr, const int port)
{
	m_impl->m_buffer = "";
	m_impl->m_sock->Connect(addr, port);
	m_impl->m_sock->SetSendRateLimit(m_impl->m_server_rate_limit);
	m_impl->m_connected = false;
	m_impl->m_online = false;
	//    m_redirecting = false;
	//    m_agreement = "";
	//m_impl->m_crc.ResetCRC();
	std::string handle = m_impl->m_sock->GetHandle();
	//if (handle.length() > 0)
	//	m_impl->m_crc.UpdateData(handle + addr);
}

void Server::Disconnect(const std::string& reason)
{
	if (!m_impl->m_connected) {
		return;
	}
	m_impl->_Disconnect(reason);
	m_impl->m_sock->Disconnect();
}

bool Server::IsOnline() const
{
	if (!m_impl->m_connected)
		return false;
	return m_impl->m_online;
}

bool Server::IsConnected()
{
	return (m_impl->m_sock->State() == Enum::SS_Open);
}

void Server::TimerUpdate()
{
	if (!IsConnected())
		return;
	if (m_impl->m_sock->InTimeout(m_impl->m_ping_timeout)) {
		sig_Timeout();
		Disconnect("timeout");
		return;
	}

	// joining battle with nat traversal:
	// if we havent finalized joining yet, and udp_reply_timeout seconds has passed since
	// we did UdpPing(our name) , join battle anyway, but with warning message that nat failed.
	// (if we'd receive reply from server, we'd finalize already)
	//
	time_t now = time(0);
	if (m_impl->m_last_udp_ping > 0 && (now - m_impl->m_last_udp_ping) > m_impl->m_udp_reply_timeout) {
		sig_NATPunchFailed();
	}

	// Is it time for a nat traversal PING?
	if ((m_impl->m_last_udp_ping + m_impl->m_keepalive) < now) {
		m_impl->m_last_udp_ping = now;
		// Nat travelsal "ping"
		const ConstIBattlePtr battle = GetCurrentBattle();
		if (battle && !battle->InGame()) {
			if (battle->GetNatType() == Enum::NAT_Hole_punching || battle->GetNatType() == Enum::NAT_Fixed_source_ports) {
				UdpPingTheServer();
				if (battle->IsFounderMe()) {
					UdpPingAllClients();
				}
			}
		}
	}
}

void Server::SayChannel(const ChannelPtr channel, const std::string& msg)
{
	m_impl->SayChannel(channel->Name(), msg);
}

void Server::HandlePong(int replyid)
{
	PingList& pinglist = m_impl->GetPingList();
	PingList::iterator itor = pinglist.find(replyid);
	if (itor != pinglist.end()) {
		sig_Pong(time(0) - itor->second);
		pinglist.erase(itor);
	}
}

void Server::JoinChannel(const std::string& channel, const std::string& key)
{
	m_impl->m_channel_pw[channel] = key;
	m_impl->JoinChannel(channel, key);
}

UserPtr Server::AcquireRelayhost()
{
	const unsigned int numbots = m_impl->m_relay_masters.size();
	if (numbots > 0) {
		srand(time(NULL));
		const unsigned int choice = rand() % numbots;
		m_impl->m_relay_host_manager = m_impl->m_relay_masters[choice];
		SayPrivate(m_impl->m_relay_host_manager, "!spawn");
		return m_impl->m_relay_host_manager;
	}
	return UserPtr();
}

void Server::OpenBattle(Battle::BattleOptions bo)
{
	if (bo.userelayhost) {
		AcquireRelayhost();
		m_impl->m_last_relay_host_password = bo.password;
	}

	if (bo.nattype > 0)
		UdpPingTheServer();
	m_impl->HostBattle(bo);
}

std::string Server::GenerateScriptPassword() const
{
	char buff[9];
	sprintf(buff, "%04x%04x", rand() & 0xFFFF, rand() & 0xFFFF);
	return std::string(buff);
}

void Server::JoinBattle(const IBattlePtr battle, const std::string& password)
{
	if (battle) {
		if (battle->GetNatType() == Enum::NAT_Hole_punching || battle->GetNatType() == Enum::NAT_Fixed_source_ports) {
			for (int n = 0; n < 5; ++n) // do 5 udp pings with tiny interval
			{
				UdpPingTheServer(GetMe()->Nick());
				m_impl->m_last_udp_ping = time(0);
			}
		}
		srand(time(NULL));
		m_impl->JoinBattle(battle, password, GenerateScriptPassword());
	}
}

void Server::SayBattle(const IBattlePtr battle, const std::string& msg)
{
	if (battle)
		m_impl->SayBattle(battle->Id(), msg);
}

void Server::DoActionBattle(const IBattlePtr battle, const std::string& msg)
{
	if (battle)
		m_impl->DoActionBattle(battle->Id(), msg);
}

void Server::Ring(const ConstCommonUserPtr user)
{
	if (user)
		m_impl->Ring(user);
}

void Server::StartHostedBattle()
{
	if (!m_impl->m_current_battle)
		return;
	if (!m_impl->m_current_battle->IsFounderMe())
		return;
	if (m_impl->m_current_battle->GetNatType() == Enum::NAT_Hole_punching || m_impl->m_current_battle->GetNatType() == Enum::NAT_Fixed_source_ports) {
		UdpPingTheServer();
		for (int i = 0; i < 5; ++i) {
			UdpPingAllClients();
		}
	}
	m_impl->StartHostedBattle();
	sig_StartHostedBattle(m_impl->m_current_battle->Id());
}

void Server::LeaveBattle(const IBattlePtr battle)
{
	if (!battle)
		return;
	m_impl->m_relay_host_bot = UserPtr();
	m_impl->LeaveBattle(battle->Id());
}

void Server::AddBot(const IBattlePtr battle, const std::string& nick, UserBattleStatus& status)
{
	if (!battle)
		return;

	UTASBattleStatus tasbs;
	tasbs.tasdata = ConvTasbattlestatus(status);
	UTASColor tascl;
	tascl.color.red = status.color.Red();
	tascl.color.green = status.color.Green();
	tascl.color.blue = status.color.Blue();
	tascl.color.zero = 0;
	//ADDBOT name battlestatus teamcolor {AIDLL}
	std::string ailib;
	ailib += status.aishortname; // + "|" + status.aiversion;
	m_impl->SendCmd("ADDBOT", nick + Util::ToIntString(tasbs.data) + " " + Util::ToIntString(tascl.data) + " " + ailib);
}

void Server::RemoveBot(const IBattlePtr battle, const CommonUserPtr user)
{
	if (!battle)
		return;
	if (!user)
		return;
	UserBattleStatus status = user->BattleStatus();
	if (!status.IsBot())
		return;

	//REMOVEBOT name
	m_impl->RelayCmd("REMOVEBOT", user->Nick());
}

void Server::UpdateBot(const IBattlePtr battle, const CommonUserPtr bot, const UserBattleStatus& status)
{
	if (!battle)
		return;
	if (!bot)
		return;
	if (!status.IsBot())
		return;

	UTASBattleStatus tasbs;
	tasbs.tasdata = ConvTasbattlestatus(status);
	UTASColor tascl;
	tascl.color.red = status.color.Red();
	tascl.color.green = status.color.Green();
	tascl.color.blue = status.color.Blue();
	tascl.color.zero = 0;
	//UPDATEBOT name battlestatus teamcolor
	boost::format params("%s %d %d");
	params % bot->Nick() % tasbs.data % tascl.data;
	if (!battle->IsProxy())
		m_impl->SendCmd("UPDATEBOT", params);
	else
		m_impl->RelayCmd("UPDATEBOT", params);
}

UserVector Server::GetAvailableRelayHostList()
{
	if (m_impl->m_relay_host_manager) {
		// this isn't blocking... so what is good for here?
		SayPrivate(m_impl->m_relay_host_manager, "!listmanagers");
	}
	UserVector ret;
	for (unsigned int i = 0; i < m_impl->m_relay_masters.size(); i++) {
		UserPtr manager = m_impl->m_relay_masters[i];
		// skip the manager is not connected or reports it's ingame ( no slots available ), or it's away ( functionality disabled )
		if (!manager)
			continue;
		if (manager->Status().in_game)
			continue;
		if (manager->Status().away)
			continue;
		ret.push_back(manager);
	}
	return ret;
}

void Server::SetRelayIngamePassword(const CommonUserPtr user)
{
	if (!user)
		return;
	if (!m_impl->m_current_battle)
		return;
	if (!m_impl->m_current_battle->InGame())
		return;
	m_impl->RelayCmd("SETINGAMEPASSWORD", user->Nick() + " " + user->BattleStatus().scriptPassword);
}

int Server::RelayScriptSendETA(const std::string& script)
{
	const StringVector strings = Util::StringTokenize(script, "\n");
	int relaylengthprefix = 10 + 1 + m_impl->m_relay_host_bot->Nick().length() + 2; // SAYPRIVATE + space + botname + space + exclamation mark length
	int length = script.length();
	length += relaylengthprefix + 11 + 1;			 // CLEANSCRIPT command size
	length += strings.size() * (relaylengthprefix + 16 + 1); // num lines * APPENDSCRIPTLINE + space command size ( \n is already counted in script.size)
	length += relaylengthprefix + 9 + 1;			 // STARTGAME command size
	return length / m_impl->m_sock->GetSendRateLimit();      // calculate time in seconds to upload script
}

void Server::SendScriptToProxy(const std::string& script)
{
	const StringVector strings = Util::StringTokenize(script, "\n");
	m_impl->RelayCmd("CLEANSCRIPT");
	for (StringVector::const_iterator itor; itor != strings.end(); ++itor) {
		m_impl->RelayCmd("APPENDSCRIPTLINE", *itor);
	}
	m_impl->RelayCmd("STARTGAME");
}

//! @brief Send udp ping.
//! @note used for nat travelsal.

unsigned int Server::UdpPing(unsigned int src_port, const std::string& target, unsigned int target_port, const std::string& message)
{
	unsigned int result = 0;
	assert(false);
	return result;
}

void Server::SendOrRelayCmd(bool toggle, const std::string& command, const std::string& param)
{
	if (toggle)
		m_impl->RelayCmd(command, param);
	else
		m_impl->SendCmd(command, param);
}

void Server::UdpPingTheServer(const std::string& message)
{
	const unsigned int port = UdpPing(m_impl->m_udp_private_port,
					  m_impl->m_addr,
					  m_impl->m_nat_helper_port,
					  message);
	if (port > 0) {
		m_impl->m_udp_private_port = port;
		sig_MyInternalUdpSourcePort(m_impl->m_udp_private_port);
	}
}

// copypasta from spring.cpp , to get users ordered same way as in tasclient.
struct UserOrder
{
	int index;			  // user number for m_users.Get
	int order;			  // user order (we'll sort by it)
	bool operator<(UserOrder b) const // comparison function for sorting
	{
		return order < b.order;
	}
};


void Server::UdpPingAllClients()
{
	if (!m_impl->m_current_battle)
		return;
	if (!m_impl->m_current_battle->IsFounderMe())
		return;

	// I'm gonna mimic tasclient's behavior.
	// It of course doesnt matter in which order pings are sent,
	// but when doing "fixed source ports", the port must be
	// FIRST_UDP_SOURCEPORT + index of user excluding myself
	// so users must be reindexed in same way as in tasclient
	// to get same source ports for pings.


	// copypasta from spring.cpp
	CommonUserVector ordered_users = m_impl->m_current_battle->Users();
	//TODO this uses ptr diff atm
	std::sort(ordered_users.begin(), ordered_users.end());

	int i = -1;
	for (const ConstCommonUserPtr user : ordered_users) {
		i++;
		if (!user)
			continue;
		const UserBattleStatus& status = user->BattleStatus();
		const std::string ip = status.ip;
		unsigned int port = status.udpport;
		const unsigned int src_port = m_impl->m_udp_private_port;
		if (m_impl->m_current_battle->GetNatType() == Enum::NAT_Fixed_source_ports) {
			port = FIRST_UDP_SOURCEPORT + i;
		}

		if (port != 0 && ip.length()) {
			UdpPing(src_port, ip, port, "hai!");
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
/////                       Internal Server Events                         /////
////////////////////////////////////////////////////////////////////////////////

void Server::OnSocketConnected(bool connection_ok, const std::string& msg)
{
	assert(connection_ok); //add proper error handling
	m_impl->m_connected = connection_ok;
	m_impl->m_online = false;
	m_impl->m_last_udp_ping = 0;
	m_impl->m_min_required_spring_ver = "";
	m_impl->m_relay_masters.clear();
	m_impl->GetLastPingID() = 0;
	m_impl->GetPingList().clear();
}

void Server::OnDisconnected()
{
	bool connectionwaspresent = m_impl->m_online ||
				    !m_impl->m_last_denied.length() ||
				    m_impl->m_redirecting;
	m_impl->m_connected = false;
	m_impl->m_online = false;
	m_impl->m_redirecting = false;
	m_impl->m_last_denied = "";
	m_impl->m_min_required_spring_ver = "";
	m_impl->m_relay_masters.clear();
	m_impl->GetLastPingID() = 0;
	m_impl->GetPingList().clear();
	// delete all users, battles, channels
	sig_Disconnected(connectionwaspresent);
}

void Server::OnSocketError(const Enum::SocketError& /*unused*/)
{
}


void Server::OnProtocolError(const Enum::Protocolerror /*unused*/)
{
}

////////////////////////////////////////////////////////////////////////////////
/////                        Command Server Events                         /////
////////////////////////////////////////////////////////////////////////////////
void Server::OnServerInitialData(const std::string& server_name, const std::string& server_ver, bool supported, const std::string& server_spring_ver, bool /*unused*/)
{
	m_impl->m_server_name = server_name;
	m_impl->m_server_ver = server_ver;
	m_impl->m_min_required_spring_ver = server_spring_ver;
}

void Server::OnNewUser(const UserPtr user)
{
	if (user->Nick() == "RelayHostManagerList") {
		m_impl->m_relay_host_manager = user;
		SayPrivate(m_impl->m_relay_host_manager, "!lm");
	}
}

void Server::OnUserStatus(const UserPtr user, UserStatus status)
{
	if (!user)
		return;
	UserStatus oldStatus = user->Status();
	user->SetStatus(status);
	//TODO: event
}

void Server::OnBattleStarted(const IBattlePtr battle)
{
	if (!battle)
		return;
	//TODO: event
}

void Server::OnBattleStopped(const IBattlePtr battle)
{
	if (!battle)
		return;
	//TODO: event
}

void Server::OnLogin(const UserPtr user)
{
	if (m_impl->m_online)
		return;
	m_impl->m_online = true;
	m_impl->m_me = user;
	m_impl->m_ping_thread = new PingThread(this, m_impl->m_ping_interval * 1000);
	m_impl->m_ping_thread->Init();
	//TODO: event
}

void Server::OnLogout()
{
	//TODO: event
}

void Server::OnUnknownCommand(const std::string& command, const std::string& params)
{
	//TODO: log
	//TODO: event
}

void Server::OnPong(long long ping_time)
{
	//TODO: event
}

void Server::OnUserQuit(const CommonUserPtr user)
{
	if (!user)
		return;
	if (user == m_impl->m_me)
		return;
	RemoveUser(user);
	//TODO: event
}

void Server::OnBattleOpened(const IBattlePtr battle)
{
	if (battle && battle->GetFounder() == m_impl->m_relay_host_bot) {
		battle->SetProxy(m_impl->m_relay_host_bot->Nick());
		JoinBattle(battle, m_impl->m_last_relay_host_password); // autojoin relayed host battles
	}
}

void Server::OnBattleMapChanged(const IBattlePtr battle, UnitsyncMap map)
{
	if (!battle)
		return;
	battle->SetHostMap(map.name, map.hash);
}

void Server::OnBattleModChanged(const IBattlePtr battle, UnitsyncGame mod)
{
	if (!battle)
		return;
	battle->SetHostMod(mod.name, mod.hash);
}

void Server::OnBattleMaxPlayersChanged(const IBattlePtr battle, int maxplayers)
{
	if (!battle)
		return;
	battle->SetMaxPlayers(maxplayers);
}

void Server::OnBattleHostChanged(const IBattlePtr battle, UserPtr host, const std::string& ip, int port)
{
	if (!battle)
		return;
	if (!host)
		battle->SetFounder(host->Nick());
	battle->SetHostIp(ip);
	battle->SetHostPort(port);
}

void Server::OnBattleSpectatorCountUpdated(const IBattlePtr battle, int spectators)
{
	if (!battle)
		return;
	battle->SetSpectators(spectators);
}

void Server::OnUserJoinedBattle(const IBattlePtr battle, const UserPtr user)
{
	if (!battle)
		return;
	if (!user)
		return;
	if (battle->IsProxy())
		m_impl->RelayCmd("SUPPORTSCRIPTPASSWORD"); // send flag to relayhost marking we support script passwords
}

void Server::OnAcceptAgreement(const std::string& agreement)
{
}

void Server::OnRing(const UserPtr from)
{
}

void Server::OnChannelMessage(const ChannelPtr channel, const std::string& msg)
{
	if (!channel)
		return;
}

void Server::OnBattleLockUpdated(const IBattlePtr battle, bool locked)
{
	if (!battle)
		return;
	battle->SetLocked(locked);
}

void Server::OnUserLeftBattle(const IBattlePtr battle, const CommonUserPtr user)
{
	if (!user)
		return;
	//bool isbot = user->BattleStatus().IsBot();
	user->BattleStatus().scriptPassword.clear();
	if (!battle)
		return;
	battle->OnUserRemoved(user);
	if (user == m_impl->m_me) {
		m_impl->m_relay_host_bot = UserPtr();
	}
	//TODO: event
}

void Server::OnBattleClosed(const IBattlePtr battle)
{
	RemoveBattle(battle);
	//TODO:event
}

void Server::OnBattleDisableUnit(const IBattlePtr battle, const std::string& unitname, int count)
{
	if (!battle)
		return;
	battle->RestrictUnit(unitname, count);
	//TODO: event
}

void Server::OnBattleEnableUnit(const IBattlePtr battle, const StringVector& unitnames)
{
	if (!battle)
		return;
	for (const std::string unit : unitnames) {
		battle->UnrestrictUnit(unit);
	}
	//TODO: event
}

void Server::OnBattleEnableAllUnits(const IBattlePtr battle)
{
	if (!battle)
		return;
	battle->UnrestrictAllUnits();
	//TODO: event
}


void Server::OnJoinChannelFailed(const ChannelPtr channel, const std::string& reason)
{
	if (!channel)
		return;
	//TODO: event
}

void Server::OnUserJoinedChannel(const ChannelPtr channel, const UserPtr user)
{
	if (!channel)
		return;
	if (!user)
		return;
	channel->OnChannelJoin(user);
	//TODO: event
}

void Server::OnKickedFromChannel(const ChannelPtr channel, const std::string& fromWho, const std::string& msg)
{
	if (!channel)
		return;
}

void Server::OnChannelSaid(const ChannelPtr channel, const CommonUserPtr user, const std::string& message)
{
	if (m_impl->m_relay_host_bot != 0 &&
	    channel == m_impl->m_channels.Get("U" + m_impl->m_relay_host_bot->Id())) {
		if (user == m_impl->m_me && message.length() > 0 && message[0] == '!')
			return;
		if (user == m_impl->m_relay_host_bot) {
			if (boost::starts_with(message, "UserScriptPassword")) {
				std::string msg_copy;
				GetWordParam(msg_copy); // skip the command keyword
				std::string usernick = GetWordParam(msg_copy);
				std::string userScriptPassword = GetWordParam(msg_copy);
				UserPtr usr = m_impl->m_users.Get(usernick);
				if (!usr)
					return;
				OnUserScriptPassword(user, userScriptPassword);
				return;
			}
		}
	}
	if (m_impl->m_relay_host_manager != 0 &&
	    channel == m_impl->m_channels.Get("U" + m_impl->m_relay_host_manager->Id())) {
		if (user == m_impl->m_me && message.length() > 0 && message[0] == '!')
			return;
		if (user == m_impl->m_relay_host_manager) {
			if (message.length() > 0 && message[0] == '\001') // error code
			{
			} else {
				m_impl->m_relay_host_bot = m_impl->m_users.FindByNick(message);
				return;
			}
		}
	}
	//    makes no sense to me
	//    if ( m_relay_masters.size() > 0
	//         && channel == m_channels.Get( "U" + Util::ToString(m_relay_masters.Id() ) ) )
	//	{
	//		if ( user == m_me && message == "!lm" )
	//			return;
	//        if ( user == m_relay_masters )
	//		{
	//			if ( boost::starts_with(message,std::string("list ") ) )
	//			{
	//                std::string list = Util::AfterFirst( message, " " ) ;
	//                m_relay_masters = Util::StringTokenize( list, "\t" );
	//                return;
	//			}
	//		}
	//	}
}

void Server::OnChannelPart(ChannelPtr channel, UserPtr user, const std::string& message)
{
}

void Server::OnBattleStartRectAdd(const IBattlePtr battle, int allyno, int left, int top, int right, int bottom)
{
	if (!battle)
		return;
	battle->AddStartRect(allyno, left, top, right, bottom);
	battle->StartRectAdded(allyno);
}

void Server::OnBattleStartRectRemove(const IBattlePtr battle, int allyno)
{
	if (!battle)
		return;
	battle->RemoveStartRect(allyno);
	battle->StartRectRemoved(allyno);
}

void Server::OnFileDownload(bool autolaunch, bool autoclose, bool /*disconnectonrefuse*/, const std::string& FileName, const std::string& url, const std::string& description)
{
	// HUH?
	//	UTASOfferFileData parsingdata;
	//	parsingdata.data = GetIntParam( params );
}

void Server::OnBattleScript(const IBattlePtr battle, const std::string& script)
{
	if (!battle)
		return;
	battle->GetBattleFromScript(true);
}

void Server::OnMuteList(const ChannelPtr channel, const MuteList& mutelist)
{
}

void Server::OnKickedFromBattle(const IBattlePtr battle)
{
	if (!battle)
		return;
}


void Server::OnUserInternalUdpPort(const CommonUserPtr user, int udpport)
{
	if (!user)
		return;
}

void Server::OnUserExternalUdpPort(const CommonUserPtr user, int udpport)
{
	if (!user)
		return;
	user->BattleStatus().udpport = udpport;
}

void Server::OnUserIP(const CommonUserPtr user, const std::string& ip)
{
	if (!user)
		return;
	user->BattleStatus().ip = ip;
}

void Server::OnChannelJoinUserList(const ChannelPtr channel, const UserVector& users)
{
}

void Server::OnSelfHostedBattle(IBattlePtr battle)
{
}

void Server::OnSelfJoinedBattle(IBattlePtr battle)
{
}

void Server::OnSetBattleOption(IBattlePtr battle, const std::string& param, const std::string& value)
{
}

void Server::OnRequestBattleStatus(IBattlePtr battle)
{
	//	if(!m_battle) return;
}

void Server::OnUserScriptPassword(const CommonUserPtr user, const std::string& pw)
{
	assert(false);
}

void Server::OnBattleHostchanged(IBattlePtr battle, int udpport)
{
}

void Server::OnUserBattleStatusUpdated(IBattlePtr battle, CommonUserPtr user, const UserBattleStatus& status)
{
}

int Server::GetNextAvailableID()
{
	return 1;
}

void Server::SayPrivate(const ConstCommonUserPtr user, const std::string& msg)
{
	m_impl->SayPrivate(user->Nick(), msg);
}

void Server::DoActionPrivate(const ConstCommonUserPtr user, const std::string& msg)
{
	m_impl->DoActionPrivate(user->Nick(), msg);
}

void Server::SendHostInfo(Enum::HostInfo update)
{
	m_impl->SendHostInfo(update);
}

void Server::SendHostInfo(const std::string& key)
{
	m_impl->SendHostInfo(key);
}

void Server::RemoveUser(const CommonUserPtr user)
{
	m_impl->m_users.Remove(user->key());
}

void Server::RemoveChannel(const ChannelPtr chan)
{
	m_impl->m_channels.Remove(chan->key());
}

void Server::RemoveBattle(const IBattlePtr battle)
{
	m_impl->m_battles.Remove(battle->key());
}

void Server::SendMyBattleStatus(const UserBattleStatus& bs)
{
	UTASBattleStatus tasbs;
	tasbs.tasdata = ConvTasbattlestatus(bs);
	UTASColor tascl;
	tascl.color.red = bs.color.Red();
	tascl.color.green = bs.color.Green();
	tascl.color.blue = bs.color.Blue();
	tascl.color.zero = 0;
	//MYBATTLESTATUS battlestatus myteamcolor
	m_impl->SendCmd("MYBATTLESTATUS", boost::format("%d %d") % tasbs.data % tascl.data);
}

void Server::SendMyUserStatus()
{
	const UserStatus& us = GetMe()->Status();
	UTASClientStatus taus;
	taus.tasdata.in_game = us.in_game;
	taus.tasdata.away = us.away;
	taus.tasdata.rank = us.rank;
	taus.tasdata.moderator = us.moderator;
	taus.tasdata.bot = us.bot;
	m_impl->SendCmd("MYSTATUS", Util::ToIntString(taus.byte));
}

void Server::ForceSide(const IBattlePtr battle, const CommonUserPtr user, int side)
{
	if (!battle)
		return;
	if (!user)
		return;
	UserBattleStatus status = user->BattleStatus();
	if (!m_impl->m_current_battle->IsFounderMe())
		return;
	if (user == m_impl->m_me) {
		status.side = side;
		SendMyBattleStatus(status);
		return;
	}

	if (status.IsBot()) {
		status.side = side;
		UpdateBot(battle, user, status);
	}
}

void Server::ForceTeam(const IBattlePtr battle, const CommonUserPtr user, int team)
{
	if (!battle)
		return;
	if (!user)
		return;
	UserBattleStatus status = user->BattleStatus();
	if (status.IsBot()) {
		status.team = team;
		UpdateBot(battle, user, status);
		return;
	}
	if (user == m_impl->m_me) {
		status.team = team;
		SendMyBattleStatus(status);
		return;
	}
	if (!m_impl->m_current_battle->IsFounderMe())
		return;

	//FORCETEAMNO username teamno
	SendOrRelayCmd(m_impl->m_current_battle->IsProxy(), "FORCETEAMNO", user->Nick() + " " + Util::ToIntString(team));
}

void Server::ForceAlly(const IBattlePtr battle, const CommonUserPtr user, int ally)
{
	if (!battle)
		return;
	if (!user)
		return;
	UserBattleStatus status = user->BattleStatus();

	if (status.IsBot()) {
		status.ally = ally;
		UpdateBot(battle, user, status);
		return;
	}

	if (user == m_impl->m_me) {
		status.ally = ally;
		SendMyBattleStatus(status);
		return;
	}
	if (!m_impl->m_current_battle->IsFounderMe())
		return;
	//FORCEALLYNO username teamno
	SendOrRelayCmd(m_impl->m_current_battle->IsProxy(), "FORCEALLYNO", user->Nick() + " " + Util::ToIntString(ally));
}

void Server::ForceColor(const IBattlePtr battle, const CommonUserPtr user, const lslColor& rgb)
{
	if (!battle)
		return;
	if (!user)
		return;
	UserBattleStatus status = user->BattleStatus();

	if (status.IsBot()) {
		status.color = rgb;
		UpdateBot(battle, user, status);
		return;
	}
	if (user == m_impl->m_me) {
		status.color = rgb;
		SendMyBattleStatus(status);
		return;
	}
	if (!m_impl->m_current_battle->IsFounderMe())
		return;

	UTASColor tascl;
	tascl.color.red = rgb.Red();
	tascl.color.green = rgb.Green();
	tascl.color.blue = rgb.Blue();
	tascl.color.zero = 0;
	//FORCETEAMCOLOR username color
	SendOrRelayCmd(m_impl->m_current_battle->IsProxy(), "FORCETEAMCOLOR", user->Nick() + " " + Util::ToIntString(tascl.data));
}

void Server::ForceSpectator(const IBattlePtr battle, const CommonUserPtr user, bool spectator)
{
	if (!battle)
		return;
	if (!user)
		return;
	UserBattleStatus status = user->BattleStatus();

	if (status.IsBot()) {
		status.spectator = spectator;
		UpdateBot(battle, user, status);
		return;
	}
	if (user == m_impl->m_me) {
		status.spectator = spectator;
		SendMyBattleStatus(status);
		return;
	}
	if (!m_impl->m_current_battle->IsFounderMe())
		return;

	//FORCESPECTATORMODE username
	SendOrRelayCmd(m_impl->m_current_battle->IsProxy(), "FORCESPECTATORMODE", user->Nick());
}

void Server::BattleKickPlayer(const IBattlePtr battle, const CommonUserPtr user)
{
	if (!battle)
		return;
	if (!user)
		return;
	UserBattleStatus status = user->BattleStatus();

	if (status.IsBot()) {
		RemoveBot(battle, user);
		return;
	}
	if (user == m_impl->m_me) {
		LeaveBattle(battle);
		return;
	}
	if (!m_impl->m_current_battle->IsFounderMe())
		return;

	if (!m_impl->m_current_battle->IsProxy()) {
		// reset his password to something random, so he can't rejoin
		user->BattleStatus().scriptPassword = (boost::format("%04x%04x") % (rand() & 0xFFFF) % (rand() & 0xFFFF)).str();
		SetRelayIngamePassword(user);
	}
	//KICKFROMBATTLE username
	SendOrRelayCmd(m_impl->m_current_battle->IsProxy(), "KICKFROMBATTLE", user->Nick());
}

void Server::SetHandicap(const IBattlePtr battle, const CommonUserPtr user, int handicap)
{
	if (!battle)
		return;
	if (!user)
		return;
	UserBattleStatus status = user->BattleStatus();

	if (status.IsBot()) {
		status.handicap = handicap;
		UpdateBot(battle, user, status);
		return;
	}

	if (!m_impl->m_current_battle->IsFounderMe())
		return;

	//HANDICAP username value
	SendOrRelayCmd(m_impl->m_current_battle->IsProxy(), "HANDICAP", user->Nick() + " " + Util::ToIntString(handicap));
}

void Server::SendUserPosition(const CommonUserPtr user)
{
	if (!m_impl->m_current_battle)
		return;
	if (!m_impl->m_current_battle->IsFounderMe())
		return;
	if (!user)
		return;

	UserBattleStatus status = user->BattleStatus();
	std::string msgx = "game/Team" + Util::ToIntString(status.team) + "/StartPosX=" + Util::ToIntString(status.pos.x);
	std::string msgy = "game/Team" + Util::ToIntString(status.team) + "/StartPosY=" + Util::ToIntString(status.pos.y);
	std::string netmessage = msgx + "\t" + msgy;
	m_impl->RelayCmd("SETSCRIPTTAGS", netmessage);
}

void Server::SendScriptToClients(const std::string& script)
{
	m_impl->RelayCmd("SCRIPTSTART");
	const StringVector lines = Util::StringTokenize(script, "\n");
	for (StringVector::iterator itor; itor != lines.end(); ++itor) {
		m_impl->RelayCmd("SCRIPT", *itor);
	}
	m_impl->RelayCmd("SCRIPTEND");
}


void Server::OnClientBattleStatus(IBattlePtr battle, UserPtr user, UserBattleStatus bstatus)
{
}

void Server::OnBattleEnableUnits(IBattlePtr battle, const StringVector unitlist)
{
}

void Server::OnUserStartPositionUpdated(IBattlePtr battle, CommonUserPtr player, const UserPosition& pos)
{
}

//**************Get/Setters ******************
IBattlePtr Server::GetCurrentBattle()
{
	return m_impl->m_current_battle;
}
const ConstIBattlePtr Server::GetCurrentBattle() const
{
	return m_impl->m_current_battle;
}

void Server::SetKeepaliveInterval(int seconds)
{
	m_impl->m_keepalive = seconds;
}
int Server::GetKeepaliveInterval()
{
	return m_impl->m_keepalive;
}

std::string Server::GetRequiredSpring() const
{
	return m_impl->m_min_required_spring_ver;
}
void Server::SetRequiredSpring(const std::string& version)
{
	m_impl->m_min_required_spring_ver = version;
}

const UserPtr Server::GetMe() const
{
	return m_impl->m_me;
}
std::string Server::GetServerName() const
{
	return m_impl->m_server_name;
}

void Server::SetPrivateUdpPort(int port)
{
	m_impl->m_udp_private_port = port;
}

void Server::OnUserLeftChannel(ChannelPtr channel, UserPtr user)
{
}

void Server::OnChannelAction(ChannelPtr channel, UserPtr user, const std::string& action)
{
}

void Server::OnChannelTopic(ChannelPtr channel, UserPtr user, const std::string& message)
{
}

void Server::Login(const std::string& user, const std::string& password)
{
	m_impl->Login(user, password);
}

//END **************Get/Setters ******************
} // namespace LSL
