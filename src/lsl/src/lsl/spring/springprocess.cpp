/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "springprocess.h"
#include "spring.h"
#include <lslutils/debug.h>
#include <lslutils/conversion.h>
#include <lslutils/logging.h>

namespace LSL
{

SpringProcess::SpringProcess(const Spring& sp)
    : m_sp(sp)
    , m_exit_code(0)
{
}

SpringProcess::~SpringProcess()
{
}

void SpringProcess::SetCommand(const std::string& cmd)
{
	m_cmd = cmd;
}

void SpringProcess::OnExit()
{
	//    m_exit_code
}

int SpringProcess::Run()
{
	m_exit_code = system(m_cmd.c_str());
	LslDebug("Spring closed.");
	return 0;
}

} // namespace LSL {
