/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifdef _MSC_VER
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#include <winsock2.h>
#endif // _MSC_VER

#include <stdexcept>
#include <vector>
#include <algorithm>
#include <fstream>
#include <clocale>

#include "spring.h"
#include "springprocess.h"
#include <lslutils/debug.h>
#include <lslutils/conversion.h>
#include <lslutils/config.h>
#include <lsl/container/userlist.h>
#include <lsl/battle/battle.h>
#include <lsl/battle/singleplayer.h>
#include <lsl/battle/offline.h>
#include <lsl/user/user.h>
#include <lslunitsync/unitsync.h>
#include <lslunitsync/optionswrapper.h>
#include <lsl/battle/tdfcontainer.h>
#include <lslutils/globalsmanager.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>

#ifdef __WXMAC__
#include <wx/filename.h>
#endif

namespace LSL
{

Spring& spring()
{
	static LSL::Util::LineInfo<Spring> m(AT);
	static LSL::Util::GlobalObjectHolder<Spring, LSL::Util::LineInfo<Spring> > m_spring(m);
	return m_spring;
}

Spring::Spring()
    : m_process(0)
    , m_running(false)
{
}

Spring::~Spring()
{
	delete m_process;
}

bool Spring::IsRunning() const
{
	return m_running;
}

bool Spring::RunReplay(const std::string& filename)
{
	LslDebug("launching spring with replay: %s", filename.c_str());

	return LaunchSpring("\"" + filename + "\"");
}

bool Spring::Run(const IBattlePtr battle)
{
	boost::filesystem::path path; // = sett().GetCurrentUsedDataDir();
	path /= "script.txt";
	try {
		boost::filesystem::ofstream f(path);
		if (!f.is_open()) {
			LslError("Access denied to script.txt at %s", path.string().c_str());
		}
		battle->DisableHostStatusInProxyMode(true);
		f << WriteScriptTxt(battle);
		battle->DisableHostStatusInProxyMode(false);
		f.close();
	} catch (std::exception& e) {
		LslError("Couldn't write script.txt, exception caught:\n %s", e.what());
		return false;
	} catch (...) {
		LslError("Couldn't write script.txt");
		return false;
	}

	std::string cmd;
	//! TODO
	//    if ( battle->GetAutoHost().GetEnabled() )
	//    {
	//        // -m, --minimise          Start minimised
	//        // -q [T], --quit=[T]      Quit immediately on game over or after T seconds
	//        cmd = "--minimise";
	//    }
	cmd += std::string(" \"" + path.string() + "\"");
	return LaunchSpring(cmd);
}

//bool Spring::Run( Battle::SinglePlayerBattle& battle )
//{
//    boost::filesystem::path path = sett().GetCurrentUsedDataDir();
//    path /= "script.txt";
//    std::string cmd = "\"" + path.string().c_str() + "\"";
//    try {
//        boost::filesystem::ofstream f( path );
//        if ( !f.is_open() ) {
//            LslError( "Access denied to script.txt at %s", path.string().c_str() );
//        }
//        battle->DisableHostStatusInProxyMode( true );
//        f << WriteScriptTxt(battle);
//        battle->DisableHostStatusInProxyMode( false );
//        f.close();
//    }
//    catch ( std::exception& e ) {
//        LslError( "Couldn't write script.txt, exception caught:\n %s", e.what() );
//        return false;
//    }
//    catch (...) {
//        LslError( "Couldn't write script.txt" );
//        return false;
//    }
//    return LaunchSpring( cmd );
//}

//bool Spring::Run(NoGuiSinglePlayerBattle &battle)
//{
//    boost::filesystem::path path = sett().GetCurrentUsedDataDir();
//    path /= "script.txt";
//    std::string cmd = "\"" + path.string().c_str() + "\"";
//    try {
//        boost::filesystem::ofstream f( path );
//        if ( !f.is_open() ) {
//            LslError( "Access denied to script.txt at %s", path.string().c_str() );
//        }
//        battle->DisableHostStatusInProxyMode( true );
//        f << WriteScriptTxt(battle);
//        battle->DisableHostStatusInProxyMode( false );
//        f.close();
//    }
//    catch ( std::exception& e ) {
//        LslError( "Couldn't write script.txt, exception caught:\n %s", e.what() );
//        return false;
//    }
//    catch (...) {
//        LslError( "Couldn't write script.txt" );
//        return false;
//    }
//    return LaunchSpring( cmd );
//}

bool Spring::Run(const std::string& script)
{
	boost::filesystem::path path; // = sett().GetCurrentUsedDataDir();
	path /= "script.txt";
	std::string cmd = std::string(" \"" + path.string() + "\"");
	try {
		boost::filesystem::ofstream f(path);
		if (!f.is_open()) {
			LslError("Access denied to script.txt at %s", path.string().c_str());
		}
		f << script;
		f.close();
	} catch (std::exception& e) {
		LslError("Couldn't write script.txt, exception caught:\n %s", e.what());
		return false;
	} catch (...) {
		LslError("Couldn't write script.txt");
		return false;
	}
	return LaunchSpring(cmd);
}

bool Spring::Run(Battle::OfflineBattle& battle)
{
	std::string path = battle.GetPlayBackFilePath();
	return LaunchSpring("\"" + path + "\"");
}

bool Spring::LaunchSpring(const std::string& params)
{
	if (m_running) {
		LslError("Spring already running!");
		return false;
	}
	//    if ( !Util::FileExists( sett().GetCurrentUsedSpringBinary() ) ) {
	//        LslError( "spring binary not found at set location: %s", sett().GetCurrentUsedSpringBinary().c_str() );
	return false;
	//    }

	std::string configfileflags; // = sett().GetCurrentUsedSpringConfigFilePath();
	if (!configfileflags.empty()) {
		configfileflags = "--config=\"" + configfileflags + "\" ";
	}

	std::string cmd; // =  "\"" + sett().GetCurrentUsedSpringBinary();
#ifdef __WXMAC__
	wxChar sep = wxFileName::GetPathSeparator();
	std::string binary = Util::config().GetCurrentUsedSpringBinary();
	if (binary.substr(binary.rfind('.') + 1) == "app")
		cmd += sep + std::string("Contents") + sep + std::string("MacOS") + sep + std::string("spring"); // append app bundle inner path
#endif
	cmd += "\" " + configfileflags + params;

	LslDebug("spring call params: %s", cmd.c_str());
	if (m_process == 0)
		m_process = new SpringProcess(*this);
	m_process->Create();
	m_process->SetCommand(cmd);
	m_process->Run();

	m_running = true;
	return true;
}


void Spring::OnTerminated(int event)
{
	m_running = false;

	int exit_code = 0l;
	sig_springStopped(exit_code, "");
}


std::string Spring::WriteScriptTxt(const IBattlePtr battle) const
{
	std::stringstream ret;

	TDF::TDFWriter tdf(ret);

	// Start generating the script.
	tdf.EnterSection("GAME");

	tdf.AppendStr("HostIP", battle->GetHostIp());
	if (battle->IsFounderMe()) {
		if (battle->GetNatType() == Enum::NAT_Hole_punching)
			tdf.AppendInt("HostPort", battle->GetMyInternalUdpSourcePort());
		else
			tdf.AppendInt("HostPort", battle->GetHostPort());
	} else {
		tdf.AppendInt("HostPort", battle->GetHostPort());
		if (battle->GetNatType() == Enum::NAT_Hole_punching) {
			tdf.AppendInt("SourcePort", battle->GetMyInternalUdpSourcePort());
		}
		/*        else if ( sett().GetClientPort() != 0)
        {
            tdf.AppendStr( "SourcePort", sett().GetClientPort() ); /// this allows to play with broken router by setting SourcePort to some forwarded port.
        } */
	}
	tdf.AppendInt("IsHost", battle->IsFounderMe());

	const ConstCommonUserPtr me = battle->GetMe();
	tdf.AppendStr("MyPlayerName", me->Nick());

	if (!me->BattleStatus().scriptPassword.empty()) {
		tdf.AppendStr("MyPasswd", me->BattleStatus().scriptPassword);
	}

	if (!battle->IsFounderMe()) {
		tdf.LeaveSection();
		return ret.str();
	}

	/**********************************************************************************
                                                                        Host-only section
            **********************************************************************************/

	tdf.AppendLineBreak();

	tdf.AppendStr("ModHash", battle->LoadMod().hash);
	tdf.AppendStr("MapHash", battle->LoadMap().hash);

	tdf.AppendStr("Mapname", battle->GetHostMapName());
	tdf.AppendStr("GameType", battle->GetHostGameName());

	tdf.AppendLineBreak();

	switch (battle->GetBattleType()) {
		case Enum::BT_Played:
			break;
		case Enum::BT_Replay: {
			std::string path = battle->GetPlayBackFilePath();
			//!TODO this did nothing with wx?!?
			//        if ( path.find("/") != std::string::npos )
			//            path.BeforeLast('/');
			tdf.AppendStr("DemoFile", path);
			tdf.AppendLineBreak();
			break;
		}
		case Enum::BT_Savegame: {
			std::string path = battle->GetPlayBackFilePath();
			//!TODO this did nothing with wx?!?
			//        if ( path.find("/") != std::string::npos )
			//            path.BeforeLast('/');
			tdf.AppendStr("Savefile", path);
			tdf.AppendLineBreak();
			break;
		}
		default:
			break;
	}

	long startpostype = Util::FromIntString(
	    battle->CustomBattleOptions()->getSingleValue("startpostype", LSL::Enum::EngineOption));

	std::vector<StartPos> remap_positions;
	if (battle->IsProxy() && (startpostype != Enum::ST_Pick) && (startpostype != Enum::ST_Choose)) {
		std::set<int> parsedteams;
		unsigned int NumTeams = 0;
		for (const ConstCommonUserPtr usr : battle->Users()) {
			const UserBattleStatus& status = usr->BattleStatus();
			if (status.spectator)
				continue;
			if (parsedteams.find(status.team) != parsedteams.end())
				continue; // skip duplicates
			parsedteams.insert(status.team);
			NumTeams++;
		}

		MapInfo infos = battle->LoadMap().info;
		unsigned int nummapstartpositions = infos.positions.size();
		unsigned int copysize = std::min(nummapstartpositions, NumTeams);
		remap_positions = std::vector<StartPos>(infos.positions.begin(), infos.positions.begin() + copysize); // only add the first x positions

		if (startpostype == Enum::ST_Random) {
			random_shuffle(remap_positions.begin(), remap_positions.end()); // shuffle the positions
		}
	}
	if (battle->IsProxy()) {
		if ((startpostype == Enum::ST_Random) || (startpostype == Enum::ST_Fixed)) {
			tdf.AppendInt("startpostype", Enum::ST_Pick);
		} else
			tdf.AppendInt("startpostype", startpostype);
	} else
		tdf.AppendInt("startpostype", startpostype);

	tdf.EnterSection("mapoptions");
	LSL::OptionsWrapper::stringTripleVec optlistMap = battle->CustomBattleOptions()->getOptions(LSL::Enum::MapOption);
	for (LSL::OptionsWrapper::stringTripleVec::const_iterator it = optlistMap.begin(); it != optlistMap.end(); ++it) {
		tdf.AppendStr(it->first, it->second.second);
	}
	tdf.LeaveSection();


	tdf.EnterSection("modoptions");
	tdf.AppendInt("relayhoststartpostype", startpostype); // also save the original wanted setting
	LSL::OptionsWrapper::stringTripleVec optlistMod = battle->CustomBattleOptions()->getOptions(LSL::Enum::ModOption);
	for (LSL::OptionsWrapper::stringTripleVec::const_iterator it = optlistMod.begin(); it != optlistMod.end(); ++it) {
		tdf.AppendStr(it->first, it->second.second);
	}
	tdf.LeaveSection();

	std::map<std::string, int> units = battle->RestrictedUnits();
	tdf.AppendInt("NumRestrictions", units.size());
	tdf.EnterSection("RESTRICT");
	int restrictcount = 0;
	for (std::map<std::string, int>::const_iterator itor = units.begin(); itor != units.end(); ++itor) {
		tdf.AppendStr("Unit" + Util::ToIntString(restrictcount), itor->first);
		tdf.AppendInt("Limit" + Util::ToIntString(restrictcount), itor->second);
		restrictcount++;
	}
	tdf.LeaveSection();


	tdf.AppendLineBreak();

	if (battle->IsProxy()) {
		tdf.AppendInt("NumPlayers", battle->GetNumPlayers() - 1);
		tdf.AppendInt("NumUsers", battle->GetNumUsers() - 1);
	} else {
		tdf.AppendInt("NumPlayers", battle->GetNumPlayers());
		tdf.AppendInt("NumUsers", battle->GetNumUsers());
	}
	tdf.AppendLineBreak();

	typedef std::map<int, int> ProgressiveTeamsVec;
	typedef ProgressiveTeamsVec::iterator ProgressiveTeamsVecIter;
	ProgressiveTeamsVec teams_to_sorted_teams; // original team -> progressive team
	int free_team = 0;
	std::map<const ConstCommonUserPtr, int> player_to_number; // player -> ordernumber
	srand(time(NULL));
	int i = 0;
	const unsigned int NumUsers = battle->Users().size();
	for (const ConstCommonUserPtr user : battle->Users()) {
		const UserBattleStatus& status = user->BattleStatus();
		if (!status.spectator) {
			ProgressiveTeamsVecIter itor = teams_to_sorted_teams.find(status.team);
			if (itor == teams_to_sorted_teams.end()) {
				teams_to_sorted_teams[status.team] = free_team;
				free_team++;
			}
		}
		if (battle->IsProxy() && (user->Nick() == battle->GetFounder()->Nick()))
			continue;
		if (status.IsBot())
			continue;
		tdf.EnterSection("PLAYER" + Util::ToIntString(i));
		tdf.AppendStr("Name", user->Nick());
		tdf.AppendStr("CountryCode", boost::algorithm::to_lower_copy(user->GetCountry()));
		tdf.AppendInt("Spectator", status.spectator);
		tdf.AppendInt("Rank", (int)user->GetRank());
		tdf.AppendInt("IsFromDemo", int(status.isfromdemo));
		if (!status.scriptPassword.empty()) {
			tdf.AppendStr("Password", status.scriptPassword);
		}
		if (!status.spectator) {
			tdf.AppendInt("Team", teams_to_sorted_teams[status.team]);
		} else {
			int speccteam = 0;
			if (!teams_to_sorted_teams.empty())
				speccteam = rand() % teams_to_sorted_teams.size();
			tdf.AppendInt("Team", speccteam);
		}
		tdf.LeaveSection();
		player_to_number[user] = i;
		i++;
	}

	unsigned int k = 0;
	for (const ConstCommonUserPtr user : battle->Users()) {
		const UserBattleStatus& status = user->BattleStatus();
		if (!status.IsBot())
			continue;
		tdf.EnterSection("AI" + Util::ToIntString(k));
		tdf.AppendStr("Name", user->Nick());		// AI's nick;
		tdf.AppendStr("ShortName", status.aishortname); // AI libtype
		tdf.AppendStr("Version", status.aiversion);     // AI libtype version
		tdf.AppendInt("Team", teams_to_sorted_teams[status.team]);
		tdf.AppendInt("IsFromDemo", int(status.isfromdemo));
		tdf.AppendInt("Host", player_to_number[battle->GetUser(status.owner)]);
		tdf.EnterSection("Options");
		int optionmapindex = battle->CustomBattleOptions()->GetAIOptionIndex(user->Nick());
		if (optionmapindex > 0) {
			LSL::OptionsWrapper::stringTripleVec optlistMod_ = battle->CustomBattleOptions()->getOptions((LSL::Enum::GameOption)optionmapindex);
			for (LSL::OptionsWrapper::stringTripleVec::const_iterator it = optlistMod_.begin(); it != optlistMod_.end(); ++it) {
				tdf.AppendStr(it->first, it->second.second);
			}
		}
		tdf.LeaveSection();
		tdf.LeaveSection();
		player_to_number[user] = k;
		k++;
	}

	tdf.AppendLineBreak();

	std::set<int> parsedteams;
	StringVector sides = usync().GetSides(battle->GetHostGameName());
	for (const ConstCommonUserPtr usr : battle->Users()) {
		const UserBattleStatus& status = usr->BattleStatus();
		if (status.spectator)
			continue;
		if (parsedteams.find(status.team) != parsedteams.end())
			continue; // skip duplicates
		parsedteams.insert(status.team);

		tdf.EnterSection("TEAM" + Util::ToIntString(teams_to_sorted_teams[status.team]));
		/*        if ( !usync().VersionSupports( LSL::USYNC_GetSkirmishAI ) && status.IsBot() )
        {
            tdf.AppendStr( "AIDLL", status.aishortname );
            tdf.AppendStr( "TeamLeader", player_to_number[battle->GetUser( status.owner )] ); // bot owner is the team leader
        }
        else*/
		{
			if (status.IsBot()) {
				tdf.AppendInt("TeamLeader", player_to_number[battle->GetUser(status.owner)]);
			} else {
				tdf.AppendInt("TeamLeader", player_to_number[usr]);
			}
		}
		if (battle->IsProxy()) {
			if (startpostype == Enum::ST_Pick) {
				tdf.AppendInt("StartPosX", status.pos.x);
				tdf.AppendInt("StartPosZ", status.pos.y);
			} else if ((startpostype == Enum::ST_Fixed) || (startpostype == Enum::ST_Random)) {
				int teamnumber = teams_to_sorted_teams[status.team];
				if (teamnumber < int(remap_positions.size())) // don't overflow
				{
					StartPos position = remap_positions[teamnumber];
					tdf.AppendInt("StartPosX", position.x);
					tdf.AppendInt("StartPosZ", position.y);
				}
			}
		} else {
			if (startpostype == Enum::ST_Pick) {
				tdf.AppendInt("StartPosX", status.pos.x);
				tdf.AppendInt("StartPosZ", status.pos.y);
			}
		}

		tdf.AppendInt("AllyTeam", status.ally);
		tdf.AppendStr("RGBColor", lslColor::ToFloatString(status.color));

		unsigned int side = status.side;
		if (side < sides.size())
			tdf.AppendStr("Side", sides[side]);
		tdf.AppendInt("Handicap", status.handicap);
		tdf.LeaveSection();
	}

	tdf.AppendLineBreak();

	unsigned int maxiter = std::max(NumUsers, battle->GetLastRectIdx() + 1);
	std::set<int> parsedallys;
	for (unsigned int i = 0; i < maxiter; i++) {
		const ConstCommonUserPtr usr = battle->Users()[i];
		const UserBattleStatus& status = usr->BattleStatus();
		Battle::BattleStartRect sr = battle->GetStartRect(i);
		if (status.spectator && !sr.IsOk())
			continue;
		int ally = status.ally;
		if (status.spectator)
			ally = i;
		if (parsedallys.find(ally) != parsedallys.end())
			continue; // skip duplicates
		sr = battle->GetStartRect(ally);
		parsedallys.insert(ally);

		tdf.EnterSection("ALLYTEAM" + Util::ToIntString(ally));
		tdf.AppendInt("NumAllies", 0);
		if (startpostype == Enum::ST_Choose) {
			if (sr.IsOk()) {
				const char* old_locale = std::setlocale(LC_NUMERIC, "C");

				tdf.AppendStr("StartRectLeft", boost::str(boost::format("%.3f") % (sr.left / 200.0)));
				tdf.AppendStr("StartRectTop", boost::str(boost::format("%.3f") % (sr.top / 200.0)));
				tdf.AppendStr("StartRectRight", boost::str(boost::format("%.3f") % (sr.right / 200.0)));
				tdf.AppendStr("StartRectBottom", boost::str(boost::format("%.3f") % (sr.bottom / 200.0)));

				std::setlocale(LC_NUMERIC, old_locale);
			}
		}
		tdf.LeaveSection();
	}

	tdf.LeaveSection();

	return ret.str();
}

} // namespace LSL {
