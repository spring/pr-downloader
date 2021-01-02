/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LIBUNITSYNCPP_THREAD_H
#define LIBUNITSYNCPP_THREAD_H

#include <thread>
#include <condition_variable>
#include <boost/noncopyable.hpp>
#include <vector>

namespace LSL
{

class WorkItemQueue;


/** @brief Abstraction of a piece of work to be done by WorkerThread
    Inherit this class to define concrete work items. */
class WorkItem : public boost::noncopyable
{
public:
	/** @brief Construct a new WorkItem */
	WorkItem()
	    : m_priority(0)
	    , m_toBeDeleted(true)
	    , m_queue(NULL)
	{
	}

	/** @brief Destructor */
	virtual ~WorkItem()
	{
	}

	/** @brief Implement this in derived class to do the work */
	virtual void Run() = 0;

	/** @brief Cancel this WorkItem, remove it from queue
        @return true if it was removed, false otherwise */
	bool Cancel();

	int GetPriority() const
	{
		return m_priority;
	}

private:
	int m_priority;		     ///< Priority of item, highest is run first
	volatile bool m_toBeDeleted; ///< Should this item be deleted after it has run?
	WorkItemQueue* m_queue;

	friend class WorkItemQueue;
	friend class WorkerThread;
};


/** @brief Priority queue of work items
 *	this is processed via a boost thread from \ref WorkerThread::operator ()
 * */
class WorkItemQueue : public boost::noncopyable
{
public:
	WorkItemQueue();
	~WorkItemQueue();
	/** @brief thread entry point */
	void Process();
	/** @brief Push more work onto the queue */
	void Push(WorkItem* item);
	/** @brief Remove a specific workitem from the queue
        @return true if it was removed, false otherwise */
	bool Remove(WorkItem* item);
	//! dangerous
	void Cancel();

private:
	/** @brief Pop one work item from the queue
        @return A work item or NULL when the queue is empty */
	WorkItem* Pop();

private:
	friend class std::thread;
	void CleanupWorkItem(WorkItem* item);

	std::mutex m_mutex;
	std::mutex m_lock;
	std::condition_variable m_cond;
	// this is a priority queue maintained as a heap stored in a vector :o
	std::vector<WorkItem*> m_queue;
	bool m_dying;
};


/** @brief Thread which processes WorkItems in it's WorkItemQueue */
class WorkerThread : public boost::noncopyable
{
public:
	WorkerThread();
	~WorkerThread();
	/** @brief Adds a new WorkItem to the queue */
	void DoWork(WorkItem* item, int priority = 0, bool toBeDeleted = true);
	//! joins underlying thread
	void Wait();

private:
	friend class std::thread;
	WorkItemQueue m_workeritemqueue;
	std::thread* m_thread;
	std::mutex m_mutex;
};

} // namespace LSL

#endif // LIBUNITSYNCPP_THREAD_H
