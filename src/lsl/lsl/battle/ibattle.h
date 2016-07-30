/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LIBLIBSPRINGLOBBY_HEADERGUARD_IBATTLE_H
#define LIBLIBSPRINGLOBBY_HEADERGUARD_IBATTLE_H

#include <lslutils/global_interfaces.h>
#include <lslutils/misc.h>
#include <lslutils/type_forwards.h>
#include <lsl/container/userlist.h>
#include <lslunitsync/data.h>

#include "enum.h"

#include <sstream>
#include <boost/scoped_ptr.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace LSL
{
namespace Battle
{

class IBattle;

//! \brief Container modeling a rectangle to place startunit in for given allyteam
struct BattleStartRect
{
	BattleStartRect()
	    : toadd(false)
	    , todelete(false)
	    , toresize(false)
	    , exist(false)
	    , ally(-1)
	    , top(-1)
	    , left(-1)
	    , right(-1)
	    , bottom(-1)
	{
	}

	bool toadd;
	bool todelete;
	bool toresize;
	bool exist;

	int ally;
	int top;
	int left;
	int right;
	int bottom;

	bool IsOk() const
	{
		return exist && !todelete;
	}
};

//! Container to split certain options off from IBattle implementations
struct BattleOptions
{
	BattleOptions();

	int battleid;
	Enum::BattleType battletype;

	/** \ingroup access restricitions @{ */
	bool islocked;
	bool ispassworded;
	std::string password;
	int rankneeded;
	bool lockexternalbalancechanges;
	/** @} */
	/** \ingroup relayhosting @{ */
	std::string proxyhost;
	bool userelayhost;
	std::string relayhost;
	/** @} */

	std::string founder;

	/** \ingroup connection settings @{ */
	Enum::NatType nattype;
	unsigned int port;
	std::string ip;
	unsigned int externaludpsourceport;
	unsigned int internaludpsourceport;
	/** @} */

	unsigned int maxplayers;
	unsigned int spectators;
	std::string maphash;
	std::string modhash;

	std::string description;
	std::string mapname;
	std::string modname;
};

/** \brief base model for all Battle types
 * \todo this is way too fat, at the very minimum pimple the internl processing
 **/
class IBattle : public HasKey<int>, public boost::enable_shared_from_this<IBattle>
{
public:
	int key() const
	{
		return GetBattleId();
	}
	static std::string className()
	{
		return "IBattle";
	}

	IBattle();
	virtual ~IBattle();

	//! docme
	struct TeamInfoContainer
	{
		bool exist;
		int TeamLeader;
		int StartPosX;
		int StartPosY;
		int AllyTeam;
		lslColor RGBColor;
		std::string SideName;
		int Handicap;
		int SideNum;
	};
	//! docme
	struct AllyInfoContainer
	{
		bool exist;
		int NumAllies;
		int StartRectLeft;
		int StartRectTop;
		int StartRectRight;
		int StartRectBottom;
	};


	virtual void SetHostMap(const std::string& mapname, const std::string& hash);
	virtual void SetLocalMap(const UnitsyncMap& map);
	virtual const UnitsyncMap& LoadMap();
	virtual std::string GetHostMapName() const;
	virtual std::string GetHostMapHash() const;

	virtual void SetProxy(const std::string& proxyhost);
	virtual std::string GetProxy() const;
	virtual bool IsProxy() const;
	virtual bool IsSynced(); //cannot be const
	virtual bool IsFounderMe() const;
	virtual bool IsFounder(const CommonUserPtr user) const;
	bool IsEveryoneReady() const;

	virtual int GetMyPlayerNum() const;
	virtual int GetPlayerNum(const ConstCommonUserPtr user) const;

	virtual void SetHostMod(const std::string& modname, const std::string& hash);
	virtual void SetLocalMod(const UnitsyncGame& mod);
	virtual const UnitsyncGame& LoadMod();
	virtual std::string GetHostGameName() const;
	virtual std::string GetHostGameHash() const;

	virtual bool MapExists() const;
	virtual bool GameExists() const;

	virtual BattleStartRect GetStartRect(unsigned int allyno) const;
	void OnUserAdded(const CommonUserPtr user);
	void OnUserBattleStatusUpdated(CommonUserPtr user, UserBattleStatus status);
	void OnUserRemoved(CommonUserPtr user);

	void ForceSide(const CommonUserPtr user, int side);
	void ForceAlly(const CommonUserPtr user, int ally);
	void ForceTeam(const CommonUserPtr user, int team);
	void ForceColor(const CommonUserPtr user, const lslColor& col);
	void ForceSpectator(const CommonUserPtr user, bool spectator);
	void SetHandicap(const CommonUserPtr user, int handicap);
	void KickPlayer(const CommonUserPtr user);

	virtual void AddStartRect(unsigned int allyno, unsigned int left, unsigned int top, unsigned int right, unsigned int bottom);
	virtual void RemoveStartRect(unsigned int allyno);
	virtual void ResizeStartRect(unsigned int allyno);
	virtual void StartRectRemoved(unsigned int allyno);
	virtual void StartRectResized(unsigned int allyno);
	virtual void StartRectAdded(unsigned int allyno);
	virtual void ClearStartRects();
	virtual unsigned int GetNumRects() const;
	virtual unsigned int GetLastRectIdx() const;
	virtual unsigned int GetNextFreeRectIdx() const;

	virtual int GetFreeTeam(bool excludeme = false) const;

	virtual const CommonUserPtr GetMe() = 0;
	virtual const ConstCommonUserPtr GetMe() const = 0;
	virtual void SetChannel(const ChannelPtr channel) = 0;
	virtual const ChannelPtr GetChannel() = 0;

	virtual void SendHostInfo(Enum::HostInfo update);
	virtual void SendHostInfo(const std::string& Tag);
	virtual void Update(const std::string& Tag);

	virtual unsigned int GetNumBots() const;
	virtual CommonUserPtr OnBotAdded(const std::string& nick, const UserBattleStatus& bs);

	virtual UserPosition GetFreePosition();
	virtual int GetFreeAlly(bool excludeme = false) const;

	virtual void RestrictUnit(const std::string& unitname, int count = 0);
	virtual void UnrestrictUnit(const std::string& unitname);
	virtual void UnrestrictAllUnits();
	virtual std::map<std::string, int> RestrictedUnits() const;

	virtual void OnUnitsyncReloaded();

	virtual OptionsWrapperPtr CustomBattleOptions()
	{
		return m_opt_wrap;
	}
	virtual const ConstOptionsWrapperPtr CustomBattleOptions() const
	{
		return m_opt_wrap;
	}

	virtual bool LoadOptionsPreset(const std::string& name);
	virtual void SaveOptionsPreset(const std::string& name);
	virtual std::string GetCurrentPreset();
	virtual void DeletePreset(const std::string& name);
	virtual StringVector GetPresetList();

	std::vector<lslColor> GetFixColorsPalette(int numteams) const;
	virtual int GetClosestFixColor(const lslColor& col, const std::vector<int>& excludes, int difference) const;
	lslColor GetFixColor(int i) const;
	virtual lslColor GetFreeColor(const ConstCommonUserPtr for_whom = UserPtr()) const;
	lslColor GetNewColor() const;
	int ColorDifference(const lslColor& a, const lslColor& b) const;

	const ConstCommonUserPtr GetFounder() const
	{
		return m_userlist.Get(m_opts.founder);
	}
	CommonUserPtr GetFounder()
	{
		return m_userlist.Get(m_opts.founder);
	}

	bool IsFull() const
	{
		return GetMaxPlayers() == GetNumActivePlayers();
	}

	ConstCommonUserVector Users() const
	{
		return m_userlist.Vectorize();
	}
	CommonUserVector Users()
	{
		return m_userlist.Vectorize();
	}
	unsigned int GetNumUsers()
	{
		return m_userlist.size();
	}
	unsigned int GetNumPlayers() const;
	unsigned int GetNumActivePlayers() const;
	unsigned int GetNumReadyPlayers() const
	{
		return m_players_ready;
	}
	unsigned int GetNumSyncedPlayers() const
	{
		return m_players_sync;
	}
	unsigned int GetNumOkPlayers() const
	{
		return m_players_ok;
	}

	int GetBattleId() const
	{
		return m_opts.battleid;
	}
	virtual int Id() const
	{
		return GetBattleId();
	}

	virtual void SetInGame(bool ingame);
	bool InGame() const
	{
		return m_ingame;
	}

	virtual void SetBattleType(Enum::BattleType type)
	{
		m_opts.battletype = type;
	}
	virtual Enum::BattleType GetBattleType()
	{
		return m_opts.battletype;
	}

	virtual void SetLocked(const bool islocked)
	{
		m_opts.islocked = islocked;
	}
	virtual bool IsLocked() const
	{
		return m_opts.islocked;
	}
	virtual void SetIsPassworded(const bool ispassworded)
	{
		m_opts.ispassworded = ispassworded;
	}
	virtual bool IsPassworded() const
	{
		return m_opts.ispassworded;
	}

	virtual void SetNatType(const Enum::NatType nattype)
	{
		m_opts.nattype = nattype;
	}
	virtual Enum::NatType GetNatType() const
	{
		return m_opts.nattype;
	}
	virtual void SetHostPort(unsigned int port)
	{
		m_opts.port = port;
	}

	virtual void SetMyExternalUdpSourcePort(unsigned int port)
	{
		m_opts.externaludpsourceport = port;
	}
	virtual unsigned int GetMyExternalUdpSourcePort()
	{
		return m_opts.externaludpsourceport;
	}

	virtual void SetMyInternalUdpSourcePort(unsigned int port)
	{
		m_opts.internaludpsourceport = port;
	}
	virtual unsigned int GetMyInternalUdpSourcePort()
	{
		return m_opts.internaludpsourceport;
	}

	virtual int GetHostPort() const
	{
		return m_opts.port;
	}
	virtual void SetFounder(const std::string& nick)
	{
		m_opts.founder = nick;
	}
	virtual void SetHostIp(const std::string& ip)
	{
		m_opts.ip = ip;
	}
	virtual std::string GetHostIp() const
	{
		return m_opts.ip;
	}

	virtual void SetMaxPlayers(const int& maxplayers)
	{
		m_opts.maxplayers = maxplayers;
	}
	virtual unsigned int GetMaxPlayers() const
	{
		return m_opts.maxplayers;
	}
	virtual void SetSpectators(const int spectators)
	{
		m_opts.spectators = spectators;
	}
	virtual int GetSpectators() const
	{
		return m_opts.spectators;
	}

	virtual void SetRankNeeded(const int& rankneeded)
	{
		m_opts.rankneeded = rankneeded;
	}
	virtual int GetRankNeeded() const
	{
		return m_opts.rankneeded;
	}

	// virtual void SetMapHash( const std::string& maphash ) { m_opts.maphash = maphash; }
	// virtual void SetMapname( const std::string& map ) { m_opts.mapname = map; }
	virtual void SetDescription(const std::string& desc)
	{
		m_opts.description = desc;
	}
	virtual std::string GetDescription() const
	{
		return m_opts.description;
	}
	// virtual void SetModname( const std::string& mod ) { m_opts.modname = mod; }

	void SetBattleOptions(const BattleOptions& options)
	{
		m_opts = options;
	}

	virtual void OnSelfLeftBattle();

	/// replay&savegame parsing stuff
	typedef std::map<int, TeamInfoContainer> TeamVec;
	typedef TeamVec::const_iterator TeamVecCIter;
	typedef TeamVec::iterator TeamVecIter;

	typedef std::map<int, AllyInfoContainer> AllyVec;
	typedef AllyVec::const_iterator AllyVecCIter;
	typedef AllyVec::iterator AllyVecIter;

	TeamVec GetParsedTeamsVec()
	{
		return m_parsed_teams;
	}
	AllyVec GetParsedAlliesVec()
	{
		return m_parsed_allies;
	}

	void SetParsedTeamsVec(const TeamVec& t)
	{
		m_parsed_teams = t;
	}
	void SetParsedAlliesVec(const AllyVec& a)
	{
		m_parsed_allies = a;
	}

	const BattleOptions& GetBattleOptions() const
	{
		return m_opts;
	}

	bool Equals(const ConstIBattlePtr other) const
	{
		return m_opts.battleid == other->GetBattleOptions().battleid;
	}

	virtual void DisableHostStatusInProxyMode(bool value)
	{
		m_generating_script = value;
	}

	virtual void UserPositionChanged(const CommonUserPtr usr);

	virtual void SetScript(const std::string& script)
	{
		m_script.str() = script;
	}
	virtual void AppendScriptLine(const std::string& line)
	{
		m_script << line;
	}
	virtual void ClearScript()
	{
		m_script.clear();
	}
	virtual std::string GetScript() const
	{
		return m_script.str();
	}

	virtual void SetPlayBackFilePath(const std::string& path)
	{
		m_playback_file_path = path;
	}
	virtual std::string GetPlayBackFilePath() const
	{
		return m_playback_file_path;
	}

	virtual void AddUserFromDemo(CommonUserPtr user);

	virtual void GetBattleFromScript(bool loadmapmod);

	virtual bool ShouldAutoStart() const;

	virtual void StartSpring() = 0;

	virtual std::map<int, int> GetAllySizes()
	{
		return m_ally_sizes;
	}
	virtual std::map<int, int> GetTeamSizes()
	{
		return m_teams_sizes;
	}

	std::map<std::string, std::string> m_script_tags; // extra script tags to reload in the case of map/mod reload

	long GetBattleRunningTime() const; // returns 0 if not started

	void LoadScriptMMOpts(const std::string& sectionname, const TDF::PDataList& node);
	void LoadScriptMMOpts(const TDF::PDataList& node);

	CommonUserPtr GetUser(const std::string& nick);

private:
	void PlayerLeftTeam(int team);
	void PlayerLeftAlly(int ally);
	void PlayerJoinedTeam(int team);
	void PlayerJoinedAlly(int ally);

	bool m_map_loaded;
	bool m_mod_loaded;
	bool m_map_exists;
	bool m_game_exists;
	UnitsyncMap m_local_map;
	UnitsyncGame m_local_game;
	UnitsyncMap m_host_map;
	UnitsyncGame m_host_game;
	std::string m_previous_local_mod_name;

	std::map<std::string, int> m_restricted_units;

	OptionsWrapperPtr m_opt_wrap;

	bool m_ingame;

	std::map<unsigned int, BattleStartRect> m_rects;

	unsigned int m_players_ready;
	unsigned int m_players_sync;
	unsigned int m_players_ok; // players which are ready and in sync

	std::map<int, int> m_ally_sizes; // allyteam -> number of people in

	std::string m_preset;

	CommonUserList m_internal_bot_list;

	/// replay&savegame stuff
	std::stringstream m_script;
	std::string m_playback_file_path;
	TeamVec m_parsed_teams;
	AllyVec m_parsed_allies;
	CommonUserList m_internal_user_list; /// to store users from savegame/replay
	boost::posix_time::ptime m_start_time;

protected:
	BattleOptions m_opts;
	bool m_is_self_in;
	CommonUserList m_userlist;
	bool m_generating_script;
	boost::scoped_ptr<boost::asio::deadline_timer> m_timer;
	std::map<std::string, time_t> m_ready_up_map; // player name -> time counting from join/unspect
	std::map<int, int> m_teams_sizes;	     // controlteam -> number of people in
};

} // namespace Battle
} // namespace LSL

#endif // LIBLIBSPRINGLOBBY_HEADERGUARD_IBATTLE_H
