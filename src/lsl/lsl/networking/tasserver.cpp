/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "tasserver.h"

#include <boost/algorithm/string.hpp>
#include <lslunitsync/optionswrapper.h>

#include <lib/base64/base64.h>
#include <lib/md5/md5.h>
#include <lslutils/conversion.h>
#include <lslutils/debug.h>
#include <lsl/battle/battle.h>

#include "socket.h"
#include "commands.h"
#include "tasserverdataformats.h"

#define ASSERT_EXCEPTION(cond, msg)             \
	do {                                    \
		if (!(cond)) {                  \
			LSL_THROW(server, msg); \
		}                               \
	} while (0)

namespace LSL
{

ServerImpl::ServerImpl(Server* serv)
    : m_cmd_dict(new CommandDictionary(this))
    , m_sock(new Socket())
    , m_keepalive(15)
    , m_ping_timeout(40)
    , m_ping_interval(10)
    , m_server_rate_limit(800)
    , m_udp_private_port(0)
    , m_id_transmission(true)
    , m_message_size_limit(1024)
    , m_redirecting(false)
    , m_connected(false)
    , m_online(false)
    , m_buffer("")
    , m_udp_reply_timeout(0)
    , m_iface(serv)
{
// FIXME:
// m_sock->sig_dataReceived.connect(boost::bind(&ServerImpl::ExecuteCommand, this, _1, _2));
}

void ServerImpl::ExecuteCommand(const std::string& cmd, std::string& inparams, int replyid)
{
	if (cmd == "PONG")
		m_iface->HandlePong(replyid);
	else
		m_cmd_dict->Process(cmd, inparams);
}

void ServerImpl::GetInGameTime(const std::string& user)
{
	SendCmd("GETINGAMETIME", user);
}

void ServerImpl::KickUser(const std::string& user)
{
	SendCmd("KICKUSER", user);
}

void ServerImpl::BanUser(const std::string& user)
{
	SendCmd("BAN", user);
}

void ServerImpl::UnBanUser(const std::string& user)
{
	SendCmd("UNBAN", user);
}

void ServerImpl::GetBanList()
{
	SendCmd("BANLIST");
}

void ServerImpl::SetChannelTopic(const std::string& channel, const std::string& topic)
{
	SendCmd("CHANNELTOPIC", boost::replace_all_copy(topic, "\n", "\\n"));
}

void ServerImpl::SendChannelMessage(const std::string& channel, const std::string& message)
{
	SendCmd("CHANNELMESSAGE", message);
}

void ServerImpl::GetIP(const std::string& user)
{
	SendCmd("GETIP", user);
}

void ServerImpl::GetChannelMutelist(const std::string& channel)
{
	SendCmd("MUTELIST", channel);
}

void ServerImpl::ChangePassword(const std::string& oldpassword, const std::string& newpassword)
{
	SendCmd("CHANGEPASSWORD", GetPasswordHash(oldpassword) + " " + GetPasswordHash(newpassword));
}

//void ServerImpl::GetMD5(const std::string& text, const std::string& newpassword )
//{
//    return GetPasswordHash(params);
//}

void ServerImpl::Rename(const std::string& newnick)
{
	SendCmd("RENAMEACCOUNT", newnick);
}

void ServerImpl::_Disconnect(const std::string& reason)
{
	SendCmd("EXIT", reason); // EXIT command for new protocol compatibility
}

void ServerImpl::Ping()
{
	SendCmd("PING");
	GetPingList()[GetLastPingID()] = time(0);
}

void ServerImpl::GetLastLoginTime(const std::string& user)
{
	SendCmd("GETLASTLOGINTIME", user);
}

void ServerImpl::GetUserIP(const std::string& user)
{
	SendCmd("FINDIP", user);
}

void ServerImpl::GetLastUserIP(const std::string& user)
{
	SendCmd("GETLASTIP", user);
}

int ServerImpl::Register(const std::string& addr, const int port, const std::string& nick,
			 const std::string& password, std::string& reason)
{
	assert(false);
#if 0
	FakeNetClass temp;
	Socket tempsocket( temp, true, true );
	tempsocket.Connect( addr, port );
	if ( tempsocket.State() != Enum::SS_Open )
		return false;

	std::string data = tempsocket.Receive().BeforeLast("\n");
	if ( data.find( "\r" ) != std::string::npos )
		data = Util::BeforeLast( data, "\r" );
	if ( GetWordParam( data ) != "TASServer" )
		return false;

	tempsocket.Send( "REGISTER " + nick + " " + GetPasswordHash( password ) + "\n" );

	data = tempsocket.Receive().BeforeLast('\n');
	tempsocket.Disconnect();
    if ( data.find( "\r" ) != std::string::npos ) data = data.BeforeLast('\r');
	if ( data.length() > 0 )
	{
		return 1;
	}
	std::string cmd = GetWordParam( data );
	if ( cmd == "REGISTRATIONACCEPTED")
	{
		return 0;
	}
	else if ( cmd == "REGISTRATIONDENIED" )
	{
        reason = data;
		return 2;
	}
#endif
	return 3;
}

bool ServerImpl::IsPasswordHash(const std::string& pass) const
{
	return pass.length() == 24 && pass[22] == '=' && pass[23] == '=';
}

std::string ServerImpl::GetPasswordHash(const std::string& pass) const
{
	if (IsPasswordHash(pass))
		return pass;

	MD5_CTX state;
	int di;

	MD5Init(&state);
	MD5Update(&state, (unsigned char*)pass.data(), pass.size());
	MD5Final(&state);
	/*	for (di = 0; di < 16; ++di)
		sprintf(hex_output + di * 2, "%02x", digest[di]);
*/
	return base64_encode(state.digest, 16);
}

void ServerImpl::Login(const std::string& user, const std::string& password)
{
	const std::string pass = GetPasswordHash(password);
	const std::string protocol = "\t 0.37";
	std::string localaddr = m_sock->GetLocalAddress();
	//    m_id_transmission = false;
	if (localaddr.length() < 1)
		localaddr = "*";
	//    SendCmd ( "LOGIN", user + " " + pass + " " + Util::GetHostCPUSpeed() + " "
	//			  + localaddr + " liblobby " + Util::GetLibLobbyVersion() + protocol  + "\ta sp");
	boost::format login_cmd("%s %s %s %d %s\t%s\ta m sp");
	SendCmd("LOGIN", (login_cmd % user % pass % 0 % localaddr % "lsl" % protocol).str());

	m_id_transmission = true;
}

void ServerImpl::RequestChannels()
{
	SendCmd("CHANNELS");
}

void ServerImpl::AcceptAgreement()
{
	SendCmd("CONFIRMAGREEMENT");
}

void ServerImpl::ExecuteCommand(const std::string& cmd, std::string& params)
{
	int replyid = 0;
	if (params[0] == '#') {
		std::string id = Util::AfterFirst(Util::BeforeFirst(params, " "), "#");
		params = Util::AfterFirst(params, " ");
		replyid = Util::FromIntString(id);
	}
	ExecuteCommand(cmd, params, replyid);
}

void ServerImpl::SendCmd(const std::string& cmd, const std::string& param)
{
	std::string msg;
	if (m_id_transmission) {
		GetLastID()++;
		msg = msg + "#" + Util::ToIntString(GetLastID()) + " ";
	}
	if (param.empty())
		msg = msg + cmd + "\n";
	else
		msg = msg + cmd + " " + param + "\n";
	/*bool send_success =*/ m_sock->SendData(msg);
	//assert(send_success);
}

void ServerImpl::JoinChannel(const std::string& channel, const std::string& key)
{
	//JOIN channame [key]
	SendCmd("JOIN", channel + " " + key);
}

void ServerImpl::PartChannel(const std::string& channel)
{
	SendCmd("LEAVE", channel);
}

void ServerImpl::DoActionChannel(const std::string& channel, const std::string& msg)
{
	SendCmd("SAYEX", channel + " " + msg);
}

void ServerImpl::SayChannel(const std::string& channel, const std::string& msg)
{
	SendCmd("SAY", channel + " " + msg);
}

void ServerImpl::SayPrivate(const std::string& nick, const std::string& msg)
{
	SendCmd("SAYPRIVATE", nick + " " + msg);
}

void ServerImpl::DoActionPrivate(const std::string& nick, const std::string& msg)
{
	SendCmd("SAYPRIVATEEX", nick + " " + msg);
}

void ServerImpl::SayBattle(int /*unused*/, const std::string& msg)
{
	SendCmd("SAYBATTLE", msg);
}

void ServerImpl::DoActionBattle(int /*unused*/, const std::string& msg)
{
	SendCmd("SAYBATTLEEX", msg);
}

void ServerImpl::ModeratorSetChannelTopic(const std::string& channel, const std::string& topic)
{
	SendCmd("CHANNELTOPIC", channel + " " + boost::replace_all_copy(topic, "\n", "\\n"));
}

void ServerImpl::ModeratorSetChannelKey(const std::string& channel, const std::string& key)
{
	SendCmd("SETCHANNELKEY", channel + " " + key);
}

void ServerImpl::ModeratorMute(const std::string& channel, const std::string& nick, int duration, bool byip)
{
	//	SendCmd( "MUTE", channel + " " + nick + " " + boost::format( "%d"), duration) + (byip?" ip":"")  );
	SendCmd("MUTE", boost::format("%s %s %s %d %s") % channel % nick % duration % (byip ? " ip" : ""));
}


void ServerImpl::ModeratorUnmute(const std::string& channel, const std::string& nick)
{
	SendCmd("UNMUTE", channel + " " + nick);
}

void ServerImpl::ModeratorKick(const std::string& channel, const std::string& reason)
{
	SendCmd("KICKUSER", channel + " " + reason);
}

void ServerImpl::ModeratorBan(const std::string& /*unused*/, bool /*unused*/)
{
	// FIXME ServerImpl::ModeratorBan not yet implemented
}

void ServerImpl::ModeratorUnban(const std::string& /*unused*/)
{
	// FIXME ServerImpl::ModeratorUnban not yet implemented
}

void ServerImpl::ModeratorGetIP(const std::string& nick)
{
	SendCmd("GETIP", nick);
}

void ServerImpl::ModeratorGetLastLogin(const std::string& nick)
{
	SendCmd("GETLASTLOGINTIME", nick);
}

void ServerImpl::ModeratorGetLastIP(const std::string& nick)
{
	SendCmd("GETLASTIP", nick);
}

void ServerImpl::ModeratorFindByIP(const std::string& ipadress)
{
	SendCmd("FINDIP", ipadress);
}

void ServerImpl::AdminGetAccountAccess(const std::string& /*unused*/)
{
	// FIXME ServerImpl::AdminGetAccountAccess not yet implemented
}


void ServerImpl::AdminChangeAccountAccess(const std::string& /*unused*/, const std::string& /*unused*/)
{
	// FIXME ServerImpl::AdminChangeAccountAccess not yet implemented
}

void ServerImpl::AdminSetBotMode(const std::string& nick, bool isbot)
{
	SendCmd("SETBOTMODE", nick + " " + (isbot ? "1" : "0"));
}

void ServerImpl::HostBattle(Battle::BattleOptions bo)
{
	boost::format cmd("0 %d %s %d %d %s %d %s %s\t%s\t");
	cmd % bo.nattype % (bo.password.empty() ? "*" : bo.password) % bo.port % bo.maxplayers % Util::MakeHashSigned(bo.modhash) % bo.rankneeded % Util::MakeHashSigned(bo.maphash) % bo.mapname % bo.description % bo.modname;

	m_delayed_open_command = "";
	if (!bo.userelayhost) {
		SendCmd("OPENBATTLE", cmd);
	} else {
		m_delayed_open_command = cmd.str();
	}

	// OPENBATTLE type natType password port maphash {map} {title} {modname}
}

void ServerImpl::JoinBattle(const IBattlePtr battle, const std::string& password, const std::string& scriptpassword)
{
	SendCmd("JOINBATTLE", Util::ToIntString(battle->Id()) + " " + password + " " + scriptpassword);
}

void ServerImpl::LeaveBattle(const int& /*unused*/)
{
	//LEAVEBATTLE
	SendCmd("LEAVEBATTLE");
}

void ServerImpl::SendHostInfo(Enum::HostInfo update)
{
	if (!m_current_battle)
		return;
	if (!m_current_battle->IsFounderMe())
		return;

	if ((update & (Enum::HI_Map | Enum::HI_Locked | Enum::HI_Spectators)) > 0) {
		// UPDATEBATTLEINFO SpectatorCount locked maphash {mapname}
		std::string cmd = (boost::format("%d %d ") % m_current_battle->GetSpectators() % m_current_battle->IsLocked()).str();
		cmd += Util::MakeHashSigned(m_current_battle->LoadMap().hash) + " ";
		cmd += m_current_battle->LoadMap().name;
		RelayCmd("UPDATEBATTLEINFO", cmd);
	}
	int relayhostmessagesize = 0;
	if (m_relay_host_bot) {
		relayhostmessagesize = m_relay_host_bot->Nick().length() + 10 + 1 + 1 + 14 + 1; // bot name + SAYPRIVATE + space + "!" + SETSCRIPTTAGS + \n
	}
	if ((update & Enum::HI_Send_All_opts) > 0) {
		std::string cmd;
		OptionsWrapper::stringTripleVec optlistMap = m_current_battle->CustomBattleOptions()->getOptions(LSL::Enum::MapOption);
		for (LSL::OptionsWrapper::stringTripleVec::const_iterator it = optlistMap.begin(); it != optlistMap.end(); ++it) {
			std::string newcmd = "game/mapoptions/" + it->first + "=" + it->second.second + "\t";
			if (int(relayhostmessagesize + cmd.length() + newcmd.length()) > m_message_size_limit) {
				RelayCmd("SETSCRIPTTAGS", cmd);
				cmd = "";
			}
			cmd += newcmd;
		}
		OptionsWrapper::stringTripleVec optlistMod = m_current_battle->CustomBattleOptions()->getOptions(LSL::Enum::ModOption);
		for (LSL::OptionsWrapper::stringTripleVec::const_iterator it = optlistMod.begin(); it != optlistMod.end(); ++it) {
			std::string newcmd = "game/modoptions/" + it->first + "=" + it->second.second + "\t";
			if (int(relayhostmessagesize + cmd.length() + newcmd.length()) > m_message_size_limit) {
				RelayCmd("SETSCRIPTTAGS", cmd);
				cmd = "";
			}
			cmd += newcmd;
		}
		OptionsWrapper::stringTripleVec optlistEng = m_current_battle->CustomBattleOptions()->getOptions(LSL::Enum::EngineOption);
		for (LSL::OptionsWrapper::stringTripleVec::const_iterator it = optlistEng.begin(); it != optlistEng.end(); ++it) {
			std::string newcmd = "game/" + it->first + "=" + it->second.second + "\t";
			if (int(relayhostmessagesize + cmd.length() + newcmd.length()) > m_message_size_limit) {
				RelayCmd("SETSCRIPTTAGS", cmd);
				cmd = "";
			}
			cmd += newcmd;
		}
		RelayCmd("SETSCRIPTTAGS", cmd);
	}

	if ((update & Enum::HI_StartRects) > 0) // Startrects should be updated.
	{
		unsigned int numrects = m_current_battle->GetLastRectIdx();
		for (unsigned int i = 0; i <= numrects; i++) // Loop through all, and remove updated or deleted.
		{
			Battle::BattleStartRect sr = m_current_battle->GetStartRect(i);
			if (!sr.exist)
				continue;
			if (sr.todelete) {
				RelayCmd("REMOVESTARTRECT", Util::ToIntString(i));
				m_current_battle->StartRectRemoved(i);
			} else if (sr.toadd) {
				RelayCmd("ADDSTARTRECT", boost::format("%d %d %d %d %d") % sr.ally % sr.left % sr.top % sr.right % sr.bottom);
				m_current_battle->StartRectAdded(i);
			} else if (sr.toresize) {
				RelayCmd("REMOVESTARTRECT", Util::ToIntString(i));
				RelayCmd("ADDSTARTRECT", boost::format("%d %d %d %d %d") % sr.ally % sr.left % sr.top % sr.right % sr.bottom);
				m_current_battle->StartRectResized(i);
			}
		}
	}
	if ((update & Enum::HI_Restrictions) > 0) {
		std::map<std::string, int> units = m_current_battle->RestrictedUnits();
		RelayCmd("ENABLEALLUNITS");
		if (!units.empty()) {
			std::stringstream msg;
			std::stringstream scriptmsg;
			for (std::map<std::string, int>::const_iterator itor = units.begin(); itor != units.end(); ++itor) {
				msg << itor->first + " ";
				scriptmsg << "game/restrict/" + itor->first + "=" + Util::ToIntString(itor->second) + '\t'; // this is a serious protocol abuse, but on the other hand, the protocol fucking suck and it's unmaintained so it will do for now
			}
			RelayCmd("DISABLEUNITS", msg.str());
			RelayCmd("SETSCRIPTTAGS", scriptmsg.str());
		}
	}
}

void ServerImpl::SendHostInfo(int type, const std::string& key)
{
	if (!m_current_battle)
		return;
	if (!m_current_battle->IsFounderMe())
		return;

	std::string cmd;

	if (type == LSL::Enum::MapOption) {
		cmd = "game/mapoptions/" + key + "=" + m_current_battle->CustomBattleOptions()->getSingleValue(key, LSL::Enum::MapOption);
	} else if (type == LSL::Enum::ModOption) {
		cmd = "game/modoptions/" + key + "=" + m_current_battle->CustomBattleOptions()->getSingleValue(key, LSL::Enum::ModOption);
	} else if (type == LSL::Enum::EngineOption) {
		cmd = "game/" + key + "=" + m_current_battle->CustomBattleOptions()->getSingleValue(key, LSL::Enum::EngineOption);
	}
	if (!m_current_battle->IsProxy())
		SendCmd("SETSCRIPTTAGS", cmd);
	else
		RelayCmd("SETSCRIPTTAGS", cmd);
}

void ServerImpl::SendHostInfo(const std::string& tag)
{
	std::string type = Util::BeforeFirst(tag, "_");
	std::string key = Util::AfterFirst(tag, "_");
	SendHostInfo(Util::FromIntString(type), key);
}

ChannelPtr ServerImpl::GetCreatePrivateChannel(const UserPtr user)
{
	if (!user)
		return ChannelPtr();
	std::string channame = "U" + user->Id();
	ChannelPtr channel = m_channels.Get(channame);
	if (!channel) {
		channel = m_channels.Add(new Channel(channame));
		m_iface->OnUserJoinedChannel(channel, user);
		m_iface->OnUserJoinedChannel(channel, m_me);
	}
	return channel;
}

void ServerImpl::SendRaw(const std::string& raw)
{
	SendCmd(raw);
}

void ServerImpl::RequestInGameTime(const std::string& nick)
{
	SendCmd("GETINGAMETIME", nick);
}

BattlePtr ServerImpl::AddBattle(const int& id)
{
	BattlePtr b(new Battle::Battle(m_iface->shared_from_this(), id));
	m_battles.Add(b);
	return b;
}

void ServerImpl::StartHostedBattle()
{
	assert(false);
}

void ServerImpl::RequestSpringUpdate(std::string& currentspringversion)
{
	SendCmd("REQUESTUPDATEFILE", "Spring " + currentspringversion);
}

////////////////////////////////////////////////////////////////////////////////
////                          parse & execute section                      /////
////////////////////////////////////////////////////////////////////////////////


void ServerImpl::OnAcceptAgreement()
{
	m_iface->OnAcceptAgreement(m_agreement);
	m_agreement = "";
}

void ServerImpl::OnNewUser(const std::string& nick, const std::string& country, int cpu, int id)
{
	std::string str_id;
	if (!id)
		str_id = Util::ToIntString(id);
	else
		str_id = User::GetNewUserId();
	UserPtr user = m_users.Get(str_id);
	if (!user) {
		user = UserPtr(new User(m_iface->shared_from_this(), str_id, nick, country, cpu));
	}
	user->SetCountry(country);
	user->SetCpu(cpu);
	user->SetNick(nick);
	m_iface->OnNewUser(user);
}

std::string ServerImpl::GetBattleChannelName(const BattlePtr battle)
{
	if (!battle)
		return "";
	return "B" + Util::ToIntString(battle->Id());
}

void ServerImpl::OnBattleOpened(int id, Enum::BattleType type, Enum::NatType nat, const std::string& nick,
				const std::string& host, int port, int maxplayers,
				bool haspass, int rank, const std::string& maphash, const std::string& map,
				const std::string& title, const std::string& mod)
{
	BattlePtr battle = AddBattle(id);
	const UserPtr user = m_users.FindByNick(nick);
	battle->OnUserAdded(user);
	battle->SetBattleType(type);
	battle->SetNatType(nat);
	battle->SetIsPassworded(haspass);
	battle->SetRankNeeded(rank);
	battle->SetDescription(title);

	m_iface->OnBattleOpened(battle);
	m_iface->OnBattleHostChanged(battle, user, host, port);
	if (user)
		m_iface->OnUserIP(user, host);
	m_iface->OnBattleMaxPlayersChanged(battle, maxplayers);
	m_iface->OnBattleMapChanged(battle, UnitsyncMap(map, maphash));
	m_iface->OnBattleModChanged(battle, UnitsyncGame(mod, ""));

	const std::string battlechanname = m_battles.GetChannelName(battle);
	ChannelPtr channel = m_channels.Get(battlechanname);
	if (!channel) {
		m_channels.Add(channel);
		battle->SetChannel(channel);
	}

	if (user->Status().in_game) {
		m_iface->OnBattleStarted(battle);
	}
}

void ServerImpl::OnUserStatusChanged(const std::string& nick, int intstatus)
{
	const ConstUserPtr user = m_users.FindByNick(nick);
	if (!user)
		return;
	UTASClientStatus tasstatus;
	tasstatus.byte = intstatus;
	const UserStatus status = ConvTasclientstatus(tasstatus.tasdata);
	m_iface->sig_UserStatusChanged(user, status);
	IBattlePtr battle = user->GetBattle();
	if (battle) {
		if (battle->GetFounder() == user)
			if (status.in_game != battle->InGame()) {
				battle->SetInGame(status.in_game);
				if (status.in_game)
					m_iface->OnBattleStarted(battle);
				else
					m_iface->OnBattleStopped(battle);
			}
	}
}

void ServerImpl::OnHostedBattle(int battleid)
{
	const BattlePtr battle = m_battles.Get(battleid);
	if (!battle)
		return;
	m_iface->OnSelfHostedBattle(battle);
	m_iface->OnSelfJoinedBattle(battle);
}

void ServerImpl::OnUserQuit(const std::string& nick)
{
	const UserPtr user = m_users.FindByNick(nick);
	if (!user)
		return;
	m_iface->OnUserQuit(user);
}

void ServerImpl::OnSelfJoinedBattle(int battleid, const std::string& hash)
{
	BattlePtr battle = m_battles.Get(battleid);
	if (!battle)
		return;
	m_current_battle = battle;
	battle->SetHostMod(battle->GetHostGameName(), hash);

	UserBattleStatus& bs = m_me->BattleStatus();
	bs.spectator = false;

	m_iface->OnUserJoinedBattle(battle, m_me);
}

void ServerImpl::OnStartHostedBattle()
{
	IBattlePtr battle = m_current_battle;
	battle->SetInGame(true);
	m_iface->OnBattleStarted(battle);
}

void ServerImpl::OnClientBattleStatus(const std::string& nick, int intstatus, int colorint)
{
	IBattlePtr battle = m_current_battle;
	UserPtr user = m_users.FindByNick(nick);
	if (!battle)
		return;
	if (!user)
		return;
	UTASBattleStatus tasbstatus;
	UserBattleStatus bstatus;
	UTASColor color;
	tasbstatus.data = intstatus;
	bstatus = ConvTasbattlestatus(tasbstatus.tasdata);
	color.data = colorint;
	bstatus.color = lslColor(color.color.red, color.color.green, color.color.blue);
	if (user->GetBattle() != battle)
		return;
	user->BattleStatus().color_index = bstatus.color_index;
	m_iface->OnClientBattleStatus(battle, user, bstatus);
}

void ServerImpl::OnUserJoinedBattle(int battleid, const std::string& nick, const std::string& userScriptPassword)
{
	BattlePtr battle = m_battles.Get(battleid);
	if (!battle)
		return;
	UserPtr user = m_users.FindByNick(nick);
	if (!user)
		return;
	battle->OnUserAdded(user);
	m_iface->OnUserJoinedBattle(battle, user);
	if (user == m_me)
		m_current_battle = battle;
	m_iface->OnUserScriptPassword(user, userScriptPassword);
	const ChannelPtr channel = battle->GetChannel();
	if (channel)
		m_iface->OnUserJoinedChannel(channel, user);

	if (user == battle->GetFounder()) {
		if (user->Status().in_game) {
			m_iface->OnBattleStarted(battle);
		}
	}
}

void ServerImpl::OnUserLeftBattle(int battleid, const std::string& nick)
{
	UserPtr user = m_users.FindByNick(nick);
	BattlePtr battle = m_battles.Get(battleid);
	if (!user)
		return;
	if (battle) {
		const ChannelPtr channel = battle->GetChannel();
		if (channel)
			m_iface->OnUserLeftChannel(channel, user);
	}
	m_iface->OnUserLeftBattle(battle, user);
	if (user == m_me)
		m_current_battle = IBattlePtr();
}

void ServerImpl::OnBattleInfoUpdated(int battleid, int spectators, bool locked, const std::string& maphash, const std::string& mapname)
{
	BattlePtr battle = m_battles.Get(battleid);
	if (!battle)
		return;
	if (battle->GetSpectators() != spectators)
		m_iface->OnBattleSpectatorCountUpdated(battle, spectators);
	if (battle->IsLocked() != locked)
		m_iface->OnBattleSpectatorCountUpdated(battle, locked);
	if (battle->GetHostMapName() != mapname)
		m_iface->OnBattleMapChanged(battle, UnitsyncMap(mapname, maphash));
}

void ServerImpl::OnSetBattleOption(std::string key, const std::string& value)
{
	IBattlePtr battle = m_current_battle;
	if (!battle)
		return;
	battle->m_script_tags[key] = value;
	if (key.substr(0, 5) == "game/") {
		key = Util::AfterFirst(key, "/");
		//TODO the original had modoptions here???
		if (key.substr(0, 8) == "restrict") {
			m_iface->OnBattleDisableUnit(battle, Util::AfterFirst(key, "/"), Util::FromIntString(value));
		} else if ((key.substr(0, 4) == "team") && key.find("startpos") != std::string::npos) {
			int team = Util::FromIntString(Util::BeforeFirst(key, "/").substr(4, std::string::npos));
			if (key.find("startposx") != std::string::npos) {
				for (const CommonUserPtr player : battle->Users()) {
					UserBattleStatus& status = player->BattleStatus();
					if (status.team == team) {
						status.pos.x = Util::FromIntString(value);
						m_iface->OnUserStartPositionUpdated(battle, player, status.pos);
					}
				}
			} else if (key.find("startposy") != std::string::npos) {
				for (const CommonUserPtr player : battle->Users()) {
					UserBattleStatus& status = player->BattleStatus();
					if (status.team == team) {
						status.pos.y = Util::FromIntString(value);
						m_iface->OnUserStartPositionUpdated(battle, player, status.pos);
					}
				}
			}
		}
	} else
		m_iface->OnSetBattleOption(battle, key, value);
}

void ServerImpl::OnSetBattleInfo(std::string infos)
{
	IBattlePtr battle = m_current_battle;
	if (!battle)
		return;
	for (const std::string command :
	     Util::StringTokenize(infos, "\t")) {
		const std::string key = boost::algorithm::to_lower_copy(Util::BeforeFirst(command, "="));
		const std::string value = Util::AfterFirst(command, "=");
		OnSetBattleOption(key, value);
	}
}

void ServerImpl::OnBattleClosed(int battleid)
{
	BattlePtr battle = m_battles.Get(battleid);
	if (!battle)
		return;
	m_iface->OnBattleClosed(battle);
}

void ServerImpl::OnBattleDisableUnits(const std::string& unitlist)
{
	IBattlePtr battle = m_current_battle;
	if (!battle)
		return;
	const StringVector units = Util::StringTokenize(unitlist, " ");
	for (const std::string unit : units) {
		m_iface->OnBattleDisableUnit(battle, unit, 0);
	}
}

void ServerImpl::OnBattleDisableUnit(const std::string& unitname, int count)
{
	IBattlePtr battle = m_current_battle;
	if (!battle)
		return;
	m_iface->OnBattleDisableUnit(battle, unitname, count);
}

void ServerImpl::OnBattleEnableUnits(const std::string& unitnames)
{
	IBattlePtr battle = m_current_battle;
	if (!battle)
		return;
	const StringVector unitlist = Util::StringTokenize(unitnames, " ");
	m_iface->OnBattleEnableUnits(battle, unitlist);
}

void ServerImpl::OnBattleEnableAllUnits()
{
	m_iface->OnBattleEnableAllUnits(m_current_battle);
}

void ServerImpl::OnJoinChannel(const std::string& channel, const std::string& rest)
{
	ChannelPtr chan = m_channels.Get("#" + channel);
	if (!chan)
		return;
	m_iface->OnUserJoinedChannel(chan, m_me);
}

void ServerImpl::OnJoinChannelFailed(const std::string& name, const std::string& reason)
{
	ChannelPtr chan = m_channels.Get("#" + name);
	if (!chan)
		chan = m_channels.Add(new Channel("#" + name));
	m_iface->OnJoinChannelFailed(chan, reason);
}

void ServerImpl::OnChannelJoin(const std::string& name, const std::string& who)
{
	ChannelPtr channel = m_channels.Get("#" + name);
	UserPtr user = m_users.FindByNick(who);
	if (!channel)
		return;
	if (!user)
		return;
	m_iface->OnUserJoinedChannel(channel, user);
}

void ServerImpl::OnChannelJoinUserList(const std::string& channel_name, const std::string& usernames)
{
	ChannelPtr channel = m_channels.Get("#" + channel_name);
	if (!channel)
		return;
	UserVector users;
	for (const std::string nick : Util::StringTokenize(usernames, " ")) {
		UserPtr user = m_users.FindByNick(nick);
		if (!user)
			continue;
		users.push_back(user);
	}
	m_iface->OnChannelJoinUserList(channel, users);
}

void ServerImpl::OnJoinedBattle(const int battleid, const std::string& msg)
{
	assert(false);
}

void ServerImpl::OnLogin(const std::string& msg)
{
	assert(false);
}

void ServerImpl::OnUserJoinedChannel(const std::string& channel_name, const std::string& who)
{
	ChannelPtr channel = m_channels.Get("#" + channel_name);
	UserPtr user = m_users.FindByNick(who);
	if (!channel)
		return;
	if (!user)
		return;
	m_iface->OnUserJoinedChannel(channel, user);
}

void ServerImpl::OnChannelSaid(const std::string& channel_name, const std::string& who, const std::string& message)
{
	ChannelPtr channel = m_channels.Get("#" + channel_name);
	UserPtr user = m_users.FindByNick(who);
	if (!channel)
		return;
	if (!user)
		return;
	m_iface->OnChannelSaid(channel, user, message);
}

void ServerImpl::OnChannelPart(const std::string& channel_name, const std::string& who, const std::string& message)
{
	ChannelPtr channel = m_channels.Get("#" + channel_name);
	UserPtr user = m_users.FindByNick(who);
	if (!channel)
		return;
	if (!user)
		return;
	m_iface->OnChannelPart(channel, user, message);
}

void ServerImpl::OnChannelTopic(const std::string& channel_name, const std::string& who, int /*unused*/, const std::string& message)
{
	ChannelPtr channel = m_channels.Get("#" + channel_name);
	if (!channel)
		return;
	UserPtr user = m_users.FindByNick(who);
	if (!user)
		return;
	m_iface->OnChannelTopic(channel, user,
				boost::replace_all_copy(message, "\\n", "\n"));
}

void ServerImpl::OnChannelAction(const std::string& channel_name, const std::string& who, const std::string& action)
{
	ChannelPtr channel = m_channels.Get("#" + channel_name);
	UserPtr user = m_users.FindByNick(who);
	if (!channel)
		return;
	if (!user)
		return;
	m_iface->OnChannelAction(channel, user, action);
}

//! our own outgoing messages, user is destinatary
void ServerImpl::OnSayPrivateMessageEx(const std::string& user, const std::string& action)
{
	UserPtr usr = m_users.FindByNick(user);
	if (!usr)
		return;
	ChannelPtr channel = GetCreatePrivateChannel(usr);
	m_iface->OnChannelAction(channel, m_me, action);
}

//! incoming messages, user is source
void ServerImpl::OnSaidPrivateMessageEx(const std::string& user, const std::string& action)
{
	UserPtr usr = m_users.FindByNick(user);
	if (!usr)
		return;
	ChannelPtr channel = GetCreatePrivateChannel(usr);
	m_iface->OnChannelAction(channel, usr, action);
}

//! our own outgoing messages, user is destinatary
void ServerImpl::OnSayPrivateMessage(const std::string& user, const std::string& message)
{
	UserPtr usr = m_users.FindByNick(user);
	if (!usr)
		return;
	ChannelPtr channel = GetCreatePrivateChannel(usr);
	m_iface->OnChannelSaid(channel, m_me, message);
}

//! incoming messages, user is source
void ServerImpl::OnSaidPrivateMessage(const std::string& user, const std::string& message)
{
	UserPtr usr = m_users.FindByNick(user);
	if (!usr)
		return;
	ChannelPtr channel = GetCreatePrivateChannel(usr);
	m_iface->OnChannelSaid(channel, usr, message);
}

void ServerImpl::OnSaidBattle(const std::string& nick, const std::string& msg)
{
	UserPtr usr = m_users.FindByNick(nick);
	if (!usr)
		return;
	IBattlePtr battle = m_current_battle;
	if (!battle)
		return;
	ChannelPtr channel = battle->GetChannel();
	if (!channel)
		return;
	m_iface->OnChannelSaid(channel, usr, msg);
}

void ServerImpl::OnBattleAction(const std::string& nick, const std::string& msg)
{
	UserPtr usr = m_users.FindByNick(nick);
	if (!usr)
		return;
	IBattlePtr battle = m_current_battle;
	if (!battle)
		return;
	ChannelPtr channel = battle->GetChannel();
	if (!channel)
		return;
	m_iface->OnChannelSaid(channel, usr, msg);
}

void ServerImpl::OnBattleStartRectAdd(int allyno, int left, int top, int right, int bottom)
{
	IBattlePtr battle = m_current_battle;
	if (!battle)
		return;
	m_iface->OnBattleStartRectAdd(battle, allyno, left, top, right, bottom);
}

void ServerImpl::OnBattleStartRectRemove(int allyno)
{
	IBattlePtr battle = m_current_battle;
	if (!battle)
		return;
	m_iface->OnBattleStartRectRemove(battle, allyno);
}

void ServerImpl::OnScriptStart()
{
	IBattlePtr battle = m_current_battle;
	if (!battle)
		return;
	battle->ClearScript();
}

void ServerImpl::OnScriptLine(const std::string& line)
{
	IBattlePtr battle = m_current_battle;
	if (!battle)
		return;
	battle->AppendScriptLine(line);
}

void ServerImpl::OnScriptEnd()
{
	IBattlePtr battle = m_current_battle;
	if (!battle)
		return;
	m_iface->OnBattleScript(battle, battle->GetScript());
}

void ServerImpl::OnMotd(const std::string& msg)
{
	//TODO: event
}

void ServerImpl::OnMutelistBegin(const std::string& channel)
{
	m_mutelist.clear();
	m_mutelist_current_channelname = channel;
}

void ServerImpl::OnMutelistItem(const std::string& mutee, const std::string& message)
{
	MuteListEntry entry(m_users.FindByNick(mutee), message);
	m_mutelist.push_back(entry);
}

void ServerImpl::OnMutelistEnd()
{
	ChannelPtr chan = m_channels.Get("#" + m_mutelist_current_channelname);
	m_mutelist_current_channelname = "";
	if (!chan)
		return;
	m_iface->OnMuteList(chan, m_mutelist);
}

void ServerImpl::OnChannelMessage(const std::string& channel, const std::string& msg)
{
	ChannelPtr chan = m_channels.Get(channel);
	if (!chan)
		return;
	m_iface->OnChannelMessage(chan, msg);
}

void ServerImpl::OnRing(const std::string& from)
{
	m_iface->OnRing(m_users.FindByNick(from));
}

void ServerImpl::OnKickedFromBattle()
{
	m_iface->OnKickedFromBattle(m_current_battle);
	m_iface->OnUserLeftBattle(m_current_battle, m_me);
}

void ServerImpl::OnKickedFromChannel(const std::string& channel, const std::string& fromWho, const std::string& message)
{
	ChannelPtr chan = m_channels.Get(channel);
	if (!chan)
		return;
	m_iface->OnKickedFromChannel(chan, fromWho, message);
	m_iface->OnUserLeftChannel(chan, m_me);
}

void ServerImpl::OnMyInternalUdpSourcePort(const unsigned int udpport)
{
	m_iface->OnUserInternalUdpPort(m_me, udpport);
}

void ServerImpl::OnMyExternalUdpSourcePort(const unsigned int udpport)
{
	m_iface->OnUserExternalUdpPort(m_me, udpport);
}

void ServerImpl::OnClientIPPort(const std::string& username, const std::string& ip, unsigned int udpport)
{
	UserPtr user = m_users.FindByNick(username);
	if (!user)
		return;

	m_iface->OnUserIP(user, ip);
	m_iface->OnUserExternalUdpPort(user, udpport);
}

void ServerImpl::OnHostExternalUdpPort(const int udpport)
{
	if (!m_current_battle)
		return;
	const CommonUserPtr host = m_current_battle->GetFounder();
	m_iface->OnUserExternalUdpPort(host, udpport);
	m_iface->OnBattleHostchanged(m_current_battle, udpport);
}

void ServerImpl::OnChannelListEntry(const std::string& channel, const int& numusers, const std::string& topic)
{
	ChannelPtr chan = m_channels.Get("#" + channel);
	if (!chan) {
		chan = m_channels.Add(new Channel("#" + channel));
	}
	chan->SetNumUsers(numusers);
	chan->SetTopic(topic);
}

void ServerImpl::OnAgreenmentLine(const std::string& line)
{
	m_agreement += line + "\n";
}

void ServerImpl::OnRequestBattleStatus()
{
	m_iface->OnRequestBattleStatus(m_current_battle);
}

void ServerImpl::OnBattleAddBot(int battleid, const std::string& nick, const std::string& owner, int intstatus, int intcolor, const std::string& aidll)
{
	BattlePtr battle = m_battles.Get(battleid);
	if (!battle)
		return;
	UTASBattleStatus tasbstatus;
	UserBattleStatus status;
	UTASColor color;
	tasbstatus.data = intstatus;
	status = ConvTasbattlestatus(tasbstatus.tasdata);
	color.data = intcolor;
	status.color = lslColor(color.color.red, color.color.green, color.color.blue);
	status.aishortname = aidll;
	status.owner = owner;
	UserPtr user(new User(m_iface->shared_from_this(), User::GetNewUserId(), nick));
	battle->OnUserAdded(user);
	m_iface->OnUserJoinedBattle(battle, user);
	m_iface->OnUserBattleStatusUpdated(battle, user, status);
}

void ServerImpl::OnBattleUpdateBot(int battleid, const std::string& nick, int intstatus, int intcolor)
{
	BattlePtr battle = m_battles.Get(battleid);
	if (!battle)
		return;
	UTASBattleStatus tasbstatus;
	UserBattleStatus status;
	UTASColor color;
	tasbstatus.data = intstatus;
	status = ConvTasbattlestatus(tasbstatus.tasdata);
	color.data = intcolor;
	status.color = lslColor(color.color.red, color.color.green, color.color.blue);
	CommonUserPtr user = battle->GetUser(nick);
	m_iface->OnUserBattleStatusUpdated(battle, user, status);
}

void ServerImpl::OnBattleRemoveBot(int battleid, const std::string& nick)
{
	BattlePtr battle = m_battles.Get(battleid);
	if (!battle)
		return;
	CommonUserPtr user = battle->GetUser(nick);
	if (!user)
		return;
	m_iface->OnUserLeftBattle(battle, user);
	if (user->BattleStatus().IsBot())
		m_iface->OnUserQuit(user);
}

void ServerImpl::SendCmd(const std::string& command, const boost::format& param)
{
	SendCmd(command, param.str());
}


void ServerImpl::Ring(const ConstCommonUserPtr user)
{
	if (m_current_battle && m_current_battle->IsProxy())
		RelayCmd("RING", user->Nick());
	else
		SendCmd("RING", user->Nick());
}

void ServerImpl::OnConnected(const std::string&, const int, const std::string&, const int)
{
}

void ServerImpl::OnLoginInfoComplete()
{
	//TODO: event
}

void ServerImpl::OnChannelListEnd()
{
}
void ServerImpl::OnServerMessage(const std::string& message)
{
}

void ServerImpl::OnServerMessageBox(const std::string& message)
{
}

void ServerImpl::OnJoinBattleFailed(const std::string& msg)
{
}


void ServerImpl::OnOpenBattleFailed(const std::string& msg)
{
}

void ServerImpl::OnLoginFailed(const std::string& reason)
{
}

void ServerImpl::OnServerBroadcast(const std::string& message)
{
}

void ServerImpl::OnRedirect(const std::string& address, int port)
{
	if (!address.length())
		return;
	if (!port)
		return;
}


void ServerImpl::RelayCmd(const std::string& command, const std::string& param)
{
	if (m_relay_host_bot) {
		m_iface->SayPrivate(m_relay_host_bot, "!" + command + " " + param);
	} else {
		SendCmd(command, param);
	}
}

void ServerImpl::RelayCmd(const std::string& command, const boost::format& param)
{
	RelayCmd(command, param.str());
}

} // namespace LSL {
