/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_MUTEXWRAPPER_H
#define LSL_MUTEXWRAPPER_H

#include <boost/thread/mutex.hpp>
#include "logging.h"

namespace LSL
{

template <class T>
class MutexWrapper;

//! pure interface a single MutexWrapper interface
class AbstractMutexWrapper
{
public:
	virtual ~AbstractMutexWrapper()
	{
	}
	virtual void Lock() = 0;
	virtual void UnLock() = 0;
};

//! implements a temporary exclusive access object
template <class T>
class ScopedLocker
{
private:
	MutexWrapper<T>& mw;
	ScopedLocker(const ScopedLocker<T>& /*other*/)
	{
	} /// prevent copying
	ScopedLocker& operator=(const ScopedLocker& /*other*/)
	{
	} /// and assignment
public:
	explicit ScopedLocker(MutexWrapper<T>& mw_)
	    : mw(mw_)
	{
		mw.Lock();
	}
	~ScopedLocker()
	{
		mw.UnLock();
	}
	T& Get()
	{
		return mw.GetData();
	}
};
/*
class ScopedLocker
{
  private:
  AbstractMutexWrapper *mw;
  ScopedLocker(const ScopedLocker<T> &other){}/// prevent copying
  ScopedLocker&  operator= (const ScopedLocker& other){}/// and assignment
  public:
  explicit ScopedLocker(AbstractMutexWrapper &mw_):mw(*mw_){
    mw.Lock();
  }
  ~ScopedLocker(){
    mw.UnLock();
  }
};*/

//!
template <class T>
class MutexWrapper : public AbstractMutexWrapper
{
	boost::mutex mutex; /// critical section is same as mutex except on windows it only works within one process (i.e. program). I'm gonna call it mutex.
	T data;
	bool locked;

public:
	MutexWrapper()
	    : locked(false)
	{
	}
	virtual ~MutexWrapper()
	{
	}
	virtual void Lock()
	{
		mutex.lock();
		locked = true;
	}
	virtual void UnLock()
	{
		locked = false;
		mutex.unlock();
	}

protected:
	T& GetData()
	{
		if (!locked) {
			LslError("serious error in MutexWrapper usage : not locked, but Get() is called!");
		}
		return data;
	}
	friend class ScopedLocker<T>;
};

} // namespace LSL

#endif // LSL_MUTEXWRAPPER_H
