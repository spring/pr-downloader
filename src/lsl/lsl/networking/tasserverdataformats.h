/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_TASDATADORMATS_H
#define LSL_TASDATADORMATS_H

namespace LSL
{

//! @brief Struct used internally by the TASServer class to get client status information.
struct TASClientstatus
{
	unsigned int in_game : 1;
	unsigned int away : 1;
	unsigned int rank : 3;
	unsigned int moderator : 1;
	unsigned int bot : 1;
};


//! @brief Union used internally by the TASServer class to get client status information.
union UTASClientStatus
{
	unsigned char byte;
	TASClientstatus tasdata;
};


//! @brief Struct used internally by the TASServer class to get battle status information.
//!TODO is that last member necessary? throws a warning baout bein used uninited
struct TASBattleStatus
{
	unsigned int : 1;
	unsigned int ready : 1;
	unsigned int team : 4;
	unsigned int ally : 4;
	unsigned int player : 1;
	unsigned int handicap : 7;
	unsigned int : 4;
	unsigned int sync : 2;
	unsigned int side : 4;
	unsigned int : 4;
};

//! @brief Union used internally by the TASServer class to get battle status information.
union UTASBattleStatus
{
	int data;
	TASBattleStatus tasdata;
};

//! @brief struct used internallby by tasserver to convert offer file bitfields
struct OfferFileData
{
	bool autoopen : 1;
	bool closelobbyondownload : 1;
	bool disconnectonrefuse : 1;
};

//! @brief Union used internally by the TASServer class to get battle status information.
union UTASOfferFileData
{
	int data;
	OfferFileData tasdata;
};


struct TASColor
{
	unsigned int red : 8;
	unsigned int green : 8;
	unsigned int blue : 8;
	unsigned int zero : 8;
};


union UTASColor
{
	int data;
	TASColor color;
};


/*

myteamcolor:  Should be 32-bit signed integer in decimal form (e.g. 255 and not FF) where each color channel should occupy 1 byte (e.g. in hexdecimal: $00BBGGRR, B = blue, G = green, R = red). Example: 255 stands for $000000FF.

*/
struct UserStatus;
struct UserBattleStatus;
UserStatus ConvTasclientstatus(TASClientstatus);
UserBattleStatus ConvTasbattlestatus(TASBattleStatus);
TASBattleStatus ConvTasbattlestatus(const UserBattleStatus&);
//IBattle::StartType IntToStartType( int start );
//NatType IntToNatType( int nat );
//IBattle::GameType IntToGameType( int gt );

} //namespace LSL {

#endif // LSL_TASDATADORMATS_H
