/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_HEADERGUARD_SINGLEPLAYERBATTLE_H
#define LSL_HEADERGUARD_SINGLEPLAYERBATTLE_H

#include "ibattle.h"
#include <lsl/user/user.h>

namespace LSL
{
namespace Battle
{

class SinglePlayerBattle : public IBattle
{
public:
	SinglePlayerBattle();
	virtual ~SinglePlayerBattle();

	// (koshi) these are never called
	//    unsigned int AddBot( int ally, int posx, int posy, int handicap, const std::string& aidll );
	//    void UpdateBot( unsigned int index, int ally, int posx, int posy, int side );

	bool IsFounderMe() const
	{
		return true;
	}

	virtual const CommonUserPtr GetMe()
	{
		return m_me;
	}
	virtual const ConstCommonUserPtr GetMe() const
	{
		return m_me;
	}

	void SendHostInfo(Enum::HostInfo update);
	void SendHostInfo(const std::string& /*unused*/)
	{
	}

	void Update(const std::string& Tag);

	void StartSpring();

protected:
	void RemoveUnfittingBots();

	UserPtr m_me;
};

} // namespace Battle {
} // namespace LSL {

#endif // LSL_HEADERGUARD_SINGLEPLAYERBATTLE_H
