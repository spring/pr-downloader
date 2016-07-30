/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include <lslutils/globalsmanager.h>
#include <assert.h>
#include <vector>

#include "lslutils/conversion.h"
#include "lslutils/logging.h"

namespace LSL
{
namespace Util
{

class GlobalsManager
{
public:
	GlobalsManager();
	bool RegisterGlobal(IGlobalObjectHolder* obj);
	void DestroyAll();
	virtual ~GlobalsManager();

private:
	std::vector<IGlobalObjectHolder*> globals;
	bool de_initialized;
};

GlobalsManager* mgr = NULL;
bool destroyed = false;

GlobalsManager* globals()
{
	if (destroyed)
		return NULL;
	if (!mgr) {
		mgr = new GlobalsManager;
	}
	return mgr;
}

void DestroyGlobals()
{
	if (mgr) {
		mgr->DestroyAll();
		delete mgr;
		mgr = NULL;
	}
	destroyed = true;
}

bool IGlobalObjectHolder::RegisterSelf()
{
	if (!globals())
		return false;
	return globals()->RegisterGlobal(this);
}

GlobalsManager::GlobalsManager()
    : de_initialized(false)
{
}

bool GlobalsManager::RegisterGlobal(IGlobalObjectHolder* obj)
{
	if (de_initialized) {
		return false;
	}
	globals.push_back(obj);
	return true;
}

void GlobalsManager::DestroyAll()
{
	for (std::vector<IGlobalObjectHolder*>::iterator i = globals.begin(); i != globals.end(); ++i) {
		(*i)->Nullify();
	}

	for (std::vector<IGlobalObjectHolder*>::iterator i = globals.begin(); i != globals.end(); ++i) {
		try {
			(*i)->Destroy();
		} catch (std::runtime_error& e) {
			LslDebug("GlobalsManager::DestroyAll(), runtume_error '%s' when destroying", e.what());
		}
	}
	globals.clear();
	de_initialized = true;
}

GlobalsManager::~GlobalsManager()
{
	assert(globals.empty());
}

} // namespace LSL {
} // namespace Util {
