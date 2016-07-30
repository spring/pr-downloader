#include "user.h"
#include <lsl/networking/iserver.h>
#include <lsl/battle/battle.h>

#include <boost/algorithm/string.hpp>

namespace LSL
{

void User::Said(const std::string& /*message*/) const
{
}

void User::Say(const std::string& message) const
{
	m_serv->SayPrivate(shared_from_this(), message);
}

void User::DoAction(const std::string& message) const
{
	m_serv->DoActionPrivate(shared_from_this(), message);
}

void User::SetStatus(const UserStatus& status)
{
	m_status = status;
	// If user is host of a game, then his in_game status tells if the game is on!
	if (m_battle) {
		try {
			const ConstCommonUserPtr user = m_battle->GetFounder();
			if (user->Nick() == m_nick) {
				m_battle->Update("");
			}
		} catch (...) {
		}
	}
}

void User::SetCountry(const std::string& country)
{
	m_country = country;
}

void User::SendMyUserStatus() const
{
	m_serv->SendMyUserStatus();
}

bool User::ExecuteSayCommand(const std::string& cmd) const
{
	if (boost::to_lower_copy(Util::BeforeFirst(cmd, " ")) == "/me") {
		m_serv->DoActionPrivate(shared_from_this(), Util::AfterFirst(cmd, " "));
		return true;
	} else
		return false;
}

UserStatus::RankContainer User::GetRank() const
{
	return Status().rank;
}

std::string User::GetRankName(UserStatus::RankContainer rank)
{
	//TODO: better interface to ranks?
	//TODO: these were translatable before
	switch (rank) {
		case UserStatus::RANK_1:
			return "Newbie";
		case UserStatus::RANK_2:
			return "Beginner";
		case UserStatus::RANK_3:
			return "Average";
		case UserStatus::RANK_4:
			return "Above average";
		case UserStatus::RANK_5:
			return "Experienced";
		case UserStatus::RANK_6:
			return "Highly experienced";
		case UserStatus::RANK_7:
			return "Veteran";
		case UserStatus::RANK_8:
			return "Badly needs to get laid";
		default:
			return "Unknown";
	}
}

std::string User::GetClan() const
{
	std::string tmp = Util::AfterFirst(m_nick, "[");
	if (tmp != m_nick) {
		std::string clan = Util::BeforeFirst(tmp, "]");
		if (clan != tmp)
			return clan;
	}
	return "";
}

User::User(IServerPtr serv,
	   const std::string id,
	   const std::string nick,
	   const std::string country,
	   const int cpu)
    : CommonUser(id, nick, country, cpu)
    , m_serv(serv)
{
}

User::~User()
{
}

} // namespace LSL {
