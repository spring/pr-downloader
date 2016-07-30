/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_USER_H
#define LSL_USER_H

#include "common.h"

#include <lslutils/type_forwards.h>


namespace LSL
{

//! actual "Online user"
class User : public CommonUser
{
public:
	User(IServerPtr serv,
	     const std::string id = GetNewUserId(),
	     const std::string nick = "invalid",
	     const std::string country = "",
	     const int cpu = DEFAULT_CPU_ID);

	virtual ~User();

	// User interface

	ConstIServerPtr GetServer() const
	{
		return m_serv;
	}

	void Said(const std::string& message) const;
	void Say(const std::string& message) const;
	void DoAction(const std::string& message) const;

	void SendMyUserStatus() const;
	void SetStatus(const UserStatus& status);
	void SetCountry(const std::string& country);

	bool ExecuteSayCommand(const std::string& cmd) const;

	static std::string GetRankName(UserStatus::RankContainer rank);

	UserStatus::RankContainer GetRank() const;
	std::string GetClan() const;

	//bool operator< ( const ConstUserPtr other ) const { return m_nick < other.GetNick() ; }
	//User& operator= ( const ConstUserPtr other );
protected:
	// User variables
	IServerPtr m_serv;
};

} // namespace LSL {

#endif // LSL_USER_H
