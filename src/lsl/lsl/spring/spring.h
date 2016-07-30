/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef SPRINGLOBBY_HEADERGUARD_SPRING_H
#define SPRINGLOBBY_HEADERGUARD_SPRING_H

#include <lslutils/type_forwards.h>
#include <boost/signals2.hpp>

namespace LSL
{
namespace Battle
{
class OfflineBattle;
}
class SpringProcess;

class Spring
{
public:
	explicit Spring();
	~Spring();

	bool IsRunning() const;
	/**
     * @brief executes spring with abs path to script
     * @param script
     * @return true on launch success, false otherwise
     */
	bool Run(const std::string& script);
	bool Run(const IBattlePtr battle);
	bool Run(Battle::OfflineBattle& battle);

	/** @brief executes spring with replay abs path
     * @param filename the full path for the replayfile
     * @return true on launch success, false otherwise
     **/
	bool RunReplay(const std::string& filename);

	std::string WriteScriptTxt(const IBattlePtr battle) const;
	void OnTerminated(int event);

	boost::signals2::signal<void(int, std::string)> sig_springStopped;
	boost::signals2::signal<void()> sig_springStarted;

protected:
	bool LaunchSpring(const std::string& params);

	SpringProcess* m_process;
	bool m_running;
};

Spring& spring();

} // namespace LSL {

#endif // SPRINGLOBBY_HEADERGUARD_SPRING_H
