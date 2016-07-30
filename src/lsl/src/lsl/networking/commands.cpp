/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "commands.h"

#include <utility>
#include <boost/shared_ptr.hpp>

#define NEWCMD(Name, Func, ...) \
	cmd_map_[Name] = CommandFactory<__VA_ARGS__>::make(&ServerImpl::Func, m_tas)

namespace LSL
{

//! mini factory to turn a given number of Tokens, a TASServer instance and a
//! member function pointer into an actual Command instance wrapped in a shared pointer
template <class T0 = NoToken, class T1 = NoToken, class T2 = NoToken, class T3 = NoToken, class T4 = NoToken, class T5 = NoToken,
	  class T6 = NoToken, class T7 = NoToken, class T8 = NoToken, class T9 = NoToken>
struct CommandFactory {
	typedef Command<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>
	    ActualCommandType;
	typedef boost::shared_ptr<CommandBase>
	    ReturnType;

	template <class F>
	static ReturnType make(F f, ServerImpl* tas)
	{
		ReturnType tmp(new ActualCommandType(f, tas));
		return tmp;
	}
};

CommandDictionary::CommandDictionary(ServerImpl* tas)
    : m_tas(tas)
{
	using namespace LSL;
	using namespace LSL::Tokens;
	NEWCMD("ADDUSER", OnNewUser, Word, Word, Int, Int);
	NEWCMD("TASSERVER", OnConnected, Word, Int, Word, Int);
	NEWCMD("ACCEPTED", OnLogin, Word);
	NEWCMD("MOTD", OnMotd, All);
	NEWCMD("CLIENTSTATUS", OnUserStatusChanged, Word, Int);
	//meh on 14 args >_>
	//    NEWCMD("BATTLEOPENED",OnBattleOpened,Int,Int,Int,Word,Word,Int,Int,Int,Int,Word,Sentence,Sentence,Sentence,Sentence);
	NEWCMD("JOINEDBATTLE", OnUserJoinedBattle, Int, Word, Word);
	NEWCMD("UPDATEBATTLEINFO", OnBattleInfoUpdated, Int, Int, Int, Word, Sentence);
	NEWCMD("LOGININFOEND", OnLoginInfoComplete, NoToken);
	NEWCMD("REMOVEUSER", OnUserQuit, Word);
	NEWCMD("BATTLECLOSED", OnBattleClosed, Int);
	NEWCMD("LEFTBATTLE", OnUserLeftBattle, Int, Word);
	NEWCMD("JOIN", OnJoinChannel, Word, Sentence);
	NEWCMD("SAID", OnChannelSaid, Word, Word, All);
	NEWCMD("JOINED", OnUserJoinedChannel, Word, Word);
	NEWCMD("LEFT", OnChannelPart, Word, Word, All);
	NEWCMD("CHANNELTOPIC", OnChannelTopic, Word, Word, Int, All);
	NEWCMD("SAIDEX", OnChannelAction, Word, Word, All);
	NEWCMD("CLIENTS", OnChannelJoinUserList, Word, All);
	NEWCMD("SAYPRIVATE", OnSayPrivateMessage, Word, All);
	NEWCMD("SAYPRIVATEEX", OnSayPrivateMessageEx, Word, All);
	NEWCMD("SAIDPRIVATEEX", OnSaidPrivateMessageEx, Word, All);
	NEWCMD("JOINBATTLE", OnJoinedBattle, Int, Word);
	NEWCMD("CLIENTBATTLESTATUS", OnClientBattleStatus, Word, Int, Int);
	NEWCMD("ADDSTARTRECT", OnBattleStartRectAdd, Int, Int, Int, Int, Int);
	NEWCMD("REMOVESTARTRECT", OnBattleStartRectRemove, Int);
	NEWCMD("ENABLEALLUNITS", OnBattleEnableAllUnits, NoToken);
	NEWCMD("ENABLEUNITS", OnBattleEnableUnits, All);
	NEWCMD("DISABLEUNITS", OnBattleDisableUnits, All);
	NEWCMD("CHANNEL", OnChannelListEntry, Word, Int, All);
	NEWCMD("ENDOFCHANNELS", OnChannelListEnd, NoToken);
	NEWCMD("REQUESTBATTLESTATUS", OnRequestBattleStatus, NoToken);
	NEWCMD("SAIDBATTLE", OnSaidBattle, Word, All);
	NEWCMD("SAIDBATTLEEX", OnBattleAction, Word, All);
	NEWCMD("AGREEMENT", OnAgreenmentLine, All);
	NEWCMD("AGREEMENTEND", OnAcceptAgreement, NoToken);
	NEWCMD("OPENBATTLE", OnHostedBattle, Int);
	NEWCMD("ADDBOT", OnBattleAddBot, Int, Word, Word, Int, Int, All);
	NEWCMD("UPDATEBOT", OnBattleUpdateBot, Int, Word, Int, Int);
	NEWCMD("REMOVEBOT", OnBattleRemoveBot, Int, Word);
	NEWCMD("RING", OnRing, Word);
	NEWCMD("SERVERMSG", OnServerMessage, All);
	NEWCMD("JOINBATTLEFAILED", OnJoinBattleFailed, All);
	NEWCMD("OPENBATTLEFAILED", OnOpenBattleFailed, All);
	NEWCMD("JOINFAILED", OnJoinChannelFailed, Word, All);
	NEWCMD("ACQUIREUSERID", OnGetHandle, NoToken);
	NEWCMD("FORCELEAVECHANNEL", OnKickedFromChannel, Word, Word, All);
	NEWCMD("DENIED", OnLoginFailed, All);
	NEWCMD("HOSTPORT", OnHostExternalUdpPort, Int);
	NEWCMD("UDPSOURCEPORT", OnMyExternalUdpSourcePort, Int);
	NEWCMD("CLIENTIPPORT", OnClientIPPort, Word, Word, Int);
	NEWCMD("SETSCRIPTTAGS", OnSetBattleInfo, All);
	NEWCMD("SCRIPTSTART", OnScriptStart, NoToken);
	NEWCMD("SCRIPTEND", OnScriptEnd, NoToken);
	NEWCMD("SCRIPT", OnScriptLine, All);
	NEWCMD("FORCEQUITBATTLE", OnKickedFromBattle, NoToken);
	NEWCMD("BROADCAST", OnServerBroadcast, All);
	NEWCMD("SERVERMSGBOX", OnServerMessageBox, All);
	NEWCMD("REDIRECT", OnRedirect, Word, Int);
	NEWCMD("MUTELISTBEGIN", OnMutelistBegin, All);
	NEWCMD("MUTELIST", OnMutelistItem, Word, All);
	NEWCMD("MUTELISTEND", OnMutelistEnd, NoToken);
	NEWCMD("OFFERFILE", OnFileDownload, Int, Sentence, Sentence, All);
}

void CommandDictionary::Process(const std::string& cmd, std::string& params) const
{
	const MapType::const_iterator it = cmd_map_.find(cmd);
	if (it != cmd_map_.end())
		it->second->process(params);
	LslError("no way to process command \"%s\" with parameters %s", cmd.c_str(), params.c_str());
}

} //namespace LSL {

#undef NEWCMD
