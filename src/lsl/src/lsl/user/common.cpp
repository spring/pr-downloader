#include "common.h"

#include <lslutils/conversion.h>


namespace LSL
{

void CommonUser::UpdateBattleStatus(const UserBattleStatus& status)
{
	// total 17 members to update.
	m_bstatus.team = status.team;
	m_bstatus.ally = status.ally;
	m_bstatus.color = status.color;
	m_bstatus.color_index = status.color_index;
	m_bstatus.handicap = status.handicap;
	m_bstatus.side = status.side;
	m_bstatus.sync = status.sync;
	m_bstatus.spectator = status.spectator;
	m_bstatus.ready = status.ready;
	if (!status.aishortname.empty())
		m_bstatus.aishortname = status.aishortname;
	if (!status.airawname.empty())
		m_bstatus.airawname = status.airawname;
	if (!status.aiversion.empty())
		m_bstatus.aiversion = status.aiversion;
	if (!(status.aitype > 0))
		m_bstatus.aitype = status.aitype;
	if (!status.owner.empty())
		m_bstatus.owner = status.owner;
	if (status.pos.x > 0)
		m_bstatus.pos.x = status.pos.x;
	if (status.pos.y > 0)
		m_bstatus.pos.y = status.pos.y;

	// update ip and port if those were set.
	if (!status.ip.empty())
		m_bstatus.ip = status.ip;
	if (status.udpport != 0)
		m_bstatus.udpport = status.udpport; // 15
}

void CommonUser::SetStatus(const UserStatus& status)
{
	m_status = status;
}

CommonUser::CommonUser(const std::string id, const std::string nick, const std::string country, const int cpu)
    : m_nick(nick)
    , m_country(country)
    , m_id(id)
    , m_cpu(cpu)
{
}

std::string CommonUser::GetNewUserId()
{
	// if server didn't send any account id to us, fill with an always increasing number
	//TODO: needs to be a pool of sorts?
	static unsigned int m_account_id_count = 0;
	return Util::ToIntString(m_account_id_count++);
}

const IBattlePtr CommonUser::GetBattle() const
{
	return m_battle;
}

void CommonUser::SetBattle(IBattlePtr battle)
{
	m_battle = battle;
}

float CommonUser::GetBalanceRank() const
{
	return 1.0 + 0.1 * float(Status().rank - UserStatus::RANK_1) / float(UserStatus::RANK_8 - UserStatus::RANK_1);
}

} //namespace LSL {
