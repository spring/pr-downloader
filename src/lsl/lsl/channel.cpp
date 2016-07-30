/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "channel.h"

#include <lsl/user/user.h>

namespace LSL
{

Channel::Channel()
{
}

Channel::Channel(const std::string& /*name*/)
{
}

void Channel::OnChannelJoin(const ConstUserPtr /*user*/)
{
	assert(false);
}

void Channel::SetNumUsers(size_t /*numusers*/)
{
	assert(false);
}

void Channel::SetTopic(const std::string& topic)
{
	m_topic = topic;
}


} // namespace LSL {
