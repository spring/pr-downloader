/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_HEADERGUARD_BATTLE_SIGNALS_H
#define LSL_HEADERGUARD_BATTLE_SIGNALS_H

#include <lslutils/type_forwards.h>
#include <boost/signals2/signal.hpp>

namespace LSL
{
namespace Signals
{

/** \addtogroup signals 
 *  @{
 */
//! battle that was left | User that left | user is a bot
static boost::signals2::signal<void(const ConstIBattlePtr, const ConstCommonUserPtr, bool)> sig_UserLeftBattle;
//! battle that updated | Tag that updated, or empty string
static boost::signals2::signal<void(const ConstIBattlePtr, std::string)> sig_BattleInfoUpdate;
//! Hosted battle that is ready to start
static boost::signals2::signal<void(const ConstIBattlePtr)> sig_BattleCouldStartHosted;
//! battle preset stuff has changed
static boost::signals2::signal<void()> sig_ReloadPresetList;
/** @}*/
}
} // namespace LSL { namespace Signals {

#endif // LSL_HEADERGUARD_BATTLE_SIGNALS_H
