/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef OFFLINEBATTLE_H_INCLUDED
#define OFFLINEBATTLE_H_INCLUDED

#include "ibattle.h"

namespace LSL
{
namespace Battle
{

class OfflineBattle : public IBattle
{
public:
	OfflineBattle(const int id);
	OfflineBattle();
	OfflineBattle(const OfflineBattle&);
	OfflineBattle& operator=(const OfflineBattle&);
	~OfflineBattle()
	{
	}
	virtual const CommonUserPtr GetMe()
	{
		return m_me;
	}
	virtual const ConstCommonUserPtr GetMe() const
	{
		return m_me;
	}
	bool IsFounderMe() const
	{
		return true;
	}
	void StartSpring();

protected:
	int m_id;
	UserPtr m_me;
};

} // namespace Battle {
} // namespace LSL {

#endif // OFFLINEBATTLE_H_INCLUDED
