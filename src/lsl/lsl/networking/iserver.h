/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_ISERVER_H
#define LSL_ISERVER_H

#include <string>
#include <map>
#include <vector>
#include <boost/signals2/signal.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <lslutils/mutexwrapper.h>
#include <lslutils/crc.h>
#include <lsl/battle/enum.h>
#include <lsl/container/battlelist.h>
#include <lsl/container/channellist.h>
#include <lsl/container/userlist.h>

#include <lslutils/type_forwards.h>
#include "enums.h"

namespace LSL
{

namespace Battle
{
class IBattle;
struct BattleOptions;
}

const unsigned int FIRST_UDP_SOURCEPORT = 8300;

class Channel;
class User;
struct UserBattleStatus;
class Socket;
struct PingThread;
class ServerImpl;
struct UnitsyncMap;
struct UnitsyncGame;
struct UserStatus;
class Server;

struct PingThread
{
	PingThread(Server*, int)
	{
	}
	PingThread()
	{
	}
	void Init()
	{
	}
};

struct MuteListEntry
{
	const ConstUserPtr who;
	const std::string msg;
	MuteListEntry(const ConstUserPtr _who, const std::string& _msg)
	    : who(_who)
	    , msg(_msg)
	{
	}
};

typedef std::list<MuteListEntry>
    MuteList;

class Server : public boost::enable_shared_from_this<Server>
{
public:
	Server();
	virtual ~Server();

	friend class ServerImpl;

	boost::signals2::signal<void()> sig_NATPunchFailed;
	//! battle_id
	boost::signals2::signal<void(int)> sig_StartHostedBattle;
	//! seconds since ping received
	boost::signals2::signal<void(int)> sig_Pong;
	boost::signals2::signal<void()> sig_Timeout;
	//! success | msg | msg_id
	boost::signals2::signal<void(bool, std::string, int)> sig_SentMessage;
	//! user whose status changed | the changed status
	boost::signals2::signal<void(const ConstUserPtr, UserStatus)> sig_UserStatusChanged;
	//! was_online
	boost::signals2::signal<void(bool)> sig_Disconnected;
	//! the udp port
	boost::signals2::signal<void(int)> sig_MyInternalUdpSourcePort;

	void Connect(const std::string& servername, const std::string& addr, const int port);
	void Disconnect(const std::string& reason);
	bool IsConnected();

	void Logout();
	void Login(const std::string& user, const std::string& password);
	bool IsOnline() const;

	void TimerUpdate();

	void PartChannel(ChannelPtr channel);
	void JoinChannel(const std::string& channel, const std::string& key);
	void SayChannel(const ChannelPtr channel, const std::string& msg);

	void SayPrivate(const ConstCommonUserPtr user, const std::string& msg);
	void DoActionPrivate(const ConstCommonUserPtr user, const std::string& msg);

	IBattlePtr GetCurrentBattle();
	const ConstIBattlePtr GetCurrentBattle() const;

	void SetKeepaliveInterval(int seconds);
	int GetKeepaliveInterval();

	std::string GetRequiredSpring() const;
	void SetRequiredSpring(const std::string& version);

	void OnSocketConnected(bool connection_ok, const std::string& msg);

	const UserPtr GetMe() const;

	std::string GetServerName() const;

	void SetRelayIngamePassword(const CommonUserPtr user);

	UserPtr AcquireRelayhost();
	void SendScriptToProxy(const std::string& script);

	void SetPrivateUdpPort(int port);
	std::string GenerateScriptPassword() const;
	int RelayScriptSendETA(const std::string& script); //!in seconds

	void SendMyUserStatus();
	void SendMyBattleStatus(const UserBattleStatus& bs);
	void JoinBattle(const IBattlePtr battle, const std::string& password);
	void SayBattle(const IBattlePtr battle, const std::string& msg);
	void DoActionBattle(const IBattlePtr battle, const std::string& msg);
	void AddBot(const IBattlePtr battle, const std::string& nick, UserBattleStatus& status);
	void RemoveBot(const IBattlePtr battle, const CommonUserPtr user);
	void UpdateBot(const IBattlePtr battle, const CommonUserPtr user, const UserBattleStatus& incoming_status);
	void ForceSide(const IBattlePtr battle, const CommonUserPtr user, int side);
	void ForceTeam(const IBattlePtr battle, const CommonUserPtr user, int team);
	void ForceAlly(const IBattlePtr battle, const CommonUserPtr user, int ally);
	void ForceColor(const IBattlePtr battle, const CommonUserPtr user, const lslColor& color);
	void ForceSpectator(const IBattlePtr battle, const CommonUserPtr user, bool spectator);
	void BattleKickPlayer(const IBattlePtr battle, const CommonUserPtr user);
	void SetHandicap(const IBattlePtr battle, const CommonUserPtr user, int handicap);
	void SendUserPosition(const CommonUserPtr user);
	void SendScriptToClients(const std::string& script);
	void Ring(const ConstCommonUserPtr user);
	void StartHostedBattle();
	void LeaveBattle(const IBattlePtr battle);
	void SendHostInfo(Enum::HostInfo update);
	void SendHostInfo(const std::string& key);

	void RemoveUser(const CommonUserPtr user);
	void RemoveChannel(const ChannelPtr chan);
	void RemoveBattle(const IBattlePtr battle);

private:
	UserVector GetAvailableRelayHostList();
	void HandlePong(int replyid);

public:
	void OpenBattle(Battle::BattleOptions bo);

	void OnSocketError(const Enum::SocketError& /*unused*/);
	void OnProtocolError(const Enum::Protocolerror /*unused*/);
	void OnNewUser(const UserPtr user);
	void OnUserStatus(const UserPtr user, UserStatus status);
	void OnServerInitialData(const std::string& server_name,
				 const std::string& server_ver,
				 bool supported,
				 const std::string& server_spring_ver,
				 bool /*unused*/);
	void OnDisconnected();
	void OnLogin(const UserPtr user);
	void OnLogout();
	void OnUnknownCommand(const std::string& command, const std::string& params);
	void OnPong(long long ping_time);
	void OnUserQuit(const CommonUserPtr user);

	void OnBattleStarted(const IBattlePtr battle);
	void OnBattleStopped(const IBattlePtr battle);
	void OnBattleOpened(const IBattlePtr battle);
	void OnBattleMapChanged(const IBattlePtr battle, UnitsyncMap map);
	void OnBattleModChanged(const IBattlePtr battle, UnitsyncGame mod);
	void OnBattleMaxPlayersChanged(const IBattlePtr battle, int maxplayers);
	void OnBattleHostChanged(const IBattlePtr battle, UserPtr host, const std::string& ip, int port);
	void OnBattleSpectatorCountUpdated(const IBattlePtr battle, int spectators);
	void OnBattleLockUpdated(const IBattlePtr battle, bool locked);
	void OnBattleClosed(const IBattlePtr battle);
	void OnBattleDisableUnit(const IBattlePtr battle, const std::string& unitname, int count);
	void OnBattleEnableUnit(const IBattlePtr battle, const StringVector& unitnames);
	void OnBattleEnableAllUnits(const IBattlePtr battle);
	void OnBattleStartRectAdd(const IBattlePtr battle, int allyno, int left, int top, int right, int bottom);
	void OnBattleStartRectRemove(const IBattlePtr battle, int allyno);
	void OnBattleScript(const IBattlePtr battle, const std::string& script);
	void OnKickedFromBattle(const IBattlePtr battle);

	void OnChannelMessage(const ChannelPtr channel, const std::string& msg);
	void OnJoinChannelFailed(const ChannelPtr channel, const std::string& reason);
	void OnKickedFromChannel(const ChannelPtr channel, const std::string& fromWho, const std::string& msg);
	void OnChannelSaid(const ChannelPtr channel, const CommonUserPtr user, const std::string& message);
	void OnChannelPart(ChannelPtr channel, UserPtr user, const std::string& message);
	void OnChannelTopic(ChannelPtr channel, UserPtr user, const std::string& message);
	void OnChannelAction(ChannelPtr channel, UserPtr user, const std::string& action);
	void OnUserLeftChannel(ChannelPtr channel, UserPtr user);

	void OnUserJoinedBattle(const IBattlePtr battle, const UserPtr user);
	void OnUserLeftBattle(const IBattlePtr battle, const CommonUserPtr user);
	void OnUserJoinedChannel(const ChannelPtr channel, const UserPtr user);
	void OnAcceptAgreement(const std::string& agreement);
	void OnRing(const UserPtr from);
	void OnFileDownload(bool autolaunch, bool autoclose, bool /*disconnectonrefuse*/, const std::string& FileName, const std::string& url, const std::string& description);
	void OnMuteList(const ChannelPtr channel, const MuteList& mutelist);
	void OnUserInternalUdpPort(const CommonUserPtr user, int udpport);
	void OnUserExternalUdpPort(const CommonUserPtr user, int udpport);
	void OnUserIP(const CommonUserPtr user, const std::string& ip);
	void OnChannelJoinUserList(const ChannelPtr channel, const UserVector& users);
	void OnRequestBattleStatus(IBattlePtr battle);
	void OnSelfHostedBattle(IBattlePtr battle);
	void OnSelfJoinedBattle(IBattlePtr battle);
	void OnSetBattleOption(IBattlePtr battle, const std::string& param, const std::string& value);
	void OnClientBattleStatus(IBattlePtr battle, UserPtr user, UserBattleStatus bstatus);
	void OnBattleEnableUnits(IBattlePtr battle, const StringVector unitlist);
	void OnUserStartPositionUpdated(IBattlePtr battle, CommonUserPtr player, const UserPosition& pos);

	void OnUserScriptPassword(const CommonUserPtr user, const std::string& pw);
	void OnBattleHostchanged(IBattlePtr battle, int udpport);
	void OnUserBattleStatusUpdated(IBattlePtr battle, CommonUserPtr user, const UserBattleStatus& status);

	int GetNextAvailableID();

private:
	void UdpPingTheServer(const std::string& message = ""); /// used for nat travelsal. pings the server.
	//! used when hosting with nat holepunching. has some rudimentary support for fixed source ports.
	void UdpPingAllClients();
	//! full parameters version, used to ping all clients when hosting.
	unsigned int UdpPing(unsigned int src_port, const std::string& target, unsigned int target_port, const std::string& message);
	//! toogle == true -> RelayCmd, else SendCmd
	void SendOrRelayCmd(bool toggle, const std::string& command, const std::string& param);

	ServerImpl* m_impl;
};

} //namespace LSL

#endif //LSL_ISERVER_H
