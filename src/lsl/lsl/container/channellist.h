/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LIBLIBSPRINGLOBBY_HEADERGUARD_CHANNELLIST_H
#define LIBLIBSPRINGLOBBY_HEADERGUARD_CHANNELLIST_H

#include "base.h"
#include <lsl/channel.h>

namespace LSL
{

//! container for channel pointers
class ChannelList : public ContainerBase<Channel>
{
};

} //end namespace LSL


#endif // LIBLIBSPRINGLOBBY_HEADERGUARD_CHANNELLIST_H
