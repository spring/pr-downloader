/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_HEADERGUARD_BATTLE_ENUM_H
#define LSL_HEADERGUARD_BATTLE_ENUM_H

namespace LSL
{
namespace Enum
{

enum NatType {
	NAT_None = 0,
	NAT_Hole_punching,
	NAT_Fixed_source_ports
};


enum BattleType {
	BT_Played,
	BT_Replay,
	BT_Savegame
};

enum BalanceType {
	balance_divide,
	balance_random
};

enum StartType {
	ST_Fixed = 0,
	ST_Random = 1,
	ST_Choose = 2,
	ST_Pick = 3
};

enum GameType {
	GT_ComContinue = 0,
	GT_ComEnds = 1,
	GT_Lineage = 2
};

enum HostInfo {
	HI_None = 0,
	HI_Map = 1,
	HI_Locked = 2,
	HI_Spectators = 4,
	HI_StartResources = 8,
	HI_MaxUnits = 16,
	HI_StartType = 32,
	HI_GameType = 64,
	HI_Options = 128,
	HI_StartRects = 256,
	HI_Restrictions = 512,
	HI_Map_Changed = 1024,
	HI_Mod_Changed = 2048,
	HI_User_Positions = 4096,
	HI_Send_All_opts = 8192
};

const unsigned int DEFAULT_SERVER_PORT = 8452;
const unsigned int DEFAULT_EXTERNAL_UDP_SOURCE_PORT = 16941;

} //namespace Enum {
} //namespace LSL {

#endif // LSL_HEADERGUARD_BATTLE_ENUM_H
