#ifndef SPRINGLOBBY_HEADERGUARD_SPRINGPROCESS_H
#define SPRINGLOBBY_HEADERGUARD_SPRINGPROCESS_H

#include <string>
#include <lslutils/type_forwards.h>

namespace LSL
{

class Spring;

class SpringProcess
{
public:
	SpringProcess(const Spring& sp);
	~SpringProcess();

	void OnExit();

	void SetCommand(const std::string& cmd);

	void Create()
	{
	}
	int Run();

protected:
	const Spring& m_sp;
	std::string m_cmd;
	int m_exit_code;
};

} // namespace LSL {

#endif // SPRINGLOBBY_HEADERGUARD_SPRINGPROCESS_H
