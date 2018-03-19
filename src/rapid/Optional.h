#pragma once

#include <cassert>
#include <utility>

namespace Rapid {

template<typename T>
class OptionalT
{
private:
	bool mHasValue;

	union
	{
		char mEmpty;
		T mValue;
	};

public:
	OptionalT()
	:
		mHasValue{false},
		mEmpty{}
	{}

	template<typename ... ArgsT>
	OptionalT(ArgsT ... Args)
	:
		mHasValue{true},
		mValue(std::forward<ArgsT>(Args) ...)
	{}

	~OptionalT()
	{
		if (mHasValue) mValue.~T();
	}

	operator bool()
	{
		return mHasValue;	
	}

	T & operator *()
	{
		assert(mHasValue);
		return mValue;
	}
};

}
