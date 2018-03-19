#pragma once

#include <utility>

namespace Rapid {

template<typename DeleterT>
class ScopeGuardT
{
private:
	bool mActive;
	DeleterT mDeleter;

public:
	ScopeGuardT() = delete;
	ScopeGuardT(ScopeGuardT const &) = delete;
	ScopeGuardT(ScopeGuardT &&) = delete;
	ScopeGuardT & operator =(ScopeGuardT const &) = delete;
	ScopeGuardT & operator =(ScopeGuardT &&) = delete;

	ScopeGuardT(DeleterT Deleter)
	:
		mActive{true},
		mDeleter(std::move(Deleter))
	{}

	~ScopeGuardT() noexcept
	{
		if (mActive) mDeleter();
	}

	void dismiss()
	{
		mActive = false;
	}
};

template<typename DeleterT>
ScopeGuardT<DeleterT> makeScopeGuard(DeleterT Deleter)
{
	return {std::move(Deleter)};
}

}
