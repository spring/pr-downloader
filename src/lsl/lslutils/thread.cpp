/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "thread.h"
#include <algorithm>
#include <mutex>
#include <lslutils/logging.h>

namespace
{
struct WorkItemCompare
{
	bool operator()(const LSL::WorkItem* a, const LSL::WorkItem* b)
	{
		return a->GetPriority() < b->GetPriority();
	}
};
}

namespace LSL
{

bool WorkItem::Cancel()
{
	LslDebug("cancelling WorkItem %p", this);
	if (m_queue == NULL)
		return false;
	return m_queue->Remove(this);
}

void WorkItemQueue::Push(WorkItem* item)
{
	if (item == NULL)
		return;
	std::scoped_lock lock(m_lock);
	m_queue.push_back(item);
	std::push_heap(m_queue.begin(), m_queue.end(), WorkItemCompare());
	item->m_queue = this;
	m_cond.notify_one();
}

WorkItem* WorkItemQueue::Pop()
{
	std::scoped_lock lock(m_lock);
	if (m_queue.empty())
		return NULL;
	WorkItem* item = m_queue.front();
	std::pop_heap(m_queue.begin(), m_queue.end(), WorkItemCompare());
	m_queue.pop_back();
	item->m_queue = NULL;
	return item;
}

bool WorkItemQueue::Remove(WorkItem* item)
{
	std::scoped_lock lock(m_lock);
	if (m_queue.empty())
		return false;
	// WARNING: this destroys the heap...
	std::vector<WorkItem*>::iterator new_end =
	    std::remove(m_queue.begin(), m_queue.end(), item);
	// did a WorkerThread process the item just before we got here?
	if (new_end == m_queue.end())
		return false;
	m_queue.erase(new_end, m_queue.end());
	// recreate the heap...
	std::make_heap(m_queue.begin(), m_queue.end(), WorkItemCompare());
	item->m_queue = NULL;
	return true;
}

void WorkItemQueue::Cancel()
{
	m_dying = true;
	m_cond.notify_all(); // wake up worker thread
}


WorkerThread::WorkerThread()
    : m_thread(new std::thread(&WorkItemQueue::Process, &m_workeritemqueue))
{
}

void WorkerThread::DoWork(WorkItem* item, int priority, bool toBeDeleted)
{
	//    LslDebug( "scheduling WorkItem %p, prio = %d", item, priority );
	item->m_priority = priority;
	item->m_toBeDeleted = toBeDeleted;
	m_workeritemqueue.Push(item);
}

void WorkerThread::Wait()
{
	m_workeritemqueue.Cancel(); //don't start new tasks / wake up worker thread
	if (m_thread != NULL) {
		m_thread->join(); //now wait for thread to exit
	}
	delete m_thread;
	m_thread = NULL;
}

WorkerThread::~WorkerThread()
{
	Wait();
}

WorkItemQueue::WorkItemQueue()
    : m_dying(false)
{
}

WorkItemQueue::~WorkItemQueue()
{
	Cancel();
}

void WorkItemQueue::Process()
{
	while (!m_dying) {
		WorkItem* item = NULL;
		std::unique_lock<std::mutex> lock(m_mutex);
		while ((!m_dying) && (item = Pop())) {
			try {
				//                LslDebug( "running WorkItem %p, prio = %d", item, item->m_priority );
				item->Run();
			} catch (std::exception& e) {
				// better eat all exceptions thrown by WorkItem::Run(),
				// don't want to let the thread die on a single faulty WorkItem.
				LslDebug("WorkerThread caught exception thrown by WorkItem::Run -- %s", e.what());
			} catch (...) {
				LslDebug("WorkerThread caught exception thrown by WorkItem::Run");
			}
			CleanupWorkItem(item);
		}
		// cleanup leftover WorkItems
		while ((item = Pop()) != NULL) {
			CleanupWorkItem(item);
		}
		if (!m_dying)
			//wait for the next Push
			m_cond.wait(lock);
	}
}

void WorkItemQueue::CleanupWorkItem(WorkItem* item)
{
	if (item->m_toBeDeleted) {
		try {
			delete item;
			item = NULL;
		} catch (std::exception& e) {
			// idem, eat exceptions from destructor
			LslDebug("WorkerThread caught exception thrown by WorkItem::~WorkItem -- %s", e.what());
		}
	}
}

} // namespace LSL
