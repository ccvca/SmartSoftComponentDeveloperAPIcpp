/*
 * FakeTask.cpp
 *
 *  Created on: Jul 24, 2019
 *      Author: alexej
 */

#include "FakeTask.h"

#include <thread>

namespace Fake {

FakeTask::FakeTask(Smart::IComponent *component)
:	Smart::ITask(component)
,	task_cancelled(false)
{  }

bool FakeTask::test_canceled()
{
	return task_cancelled;
}
void FakeTask::sleep_for(const Smart::Duration &duration)
{
	std::this_thread::sleep_for(duration);
}

/** Creates and starts a new thread (if not yet started)
 *
 *  A new thread is spawned if no thread has been spawned yet before
 *  for this instance.
 *
 *  @return 0 on success (and if thread has already been started) or -1 on failure
 */
int FakeTask::start()
{
	task_runner = std::async(std::launch::async, &FakeTask::task_execution, this);
	return 0;
}

/** Stops the currently active thread (if it was started before)
 *
 *  The internal thread is signaled to stop. If wait_till_stopped
 *  is set to true then the call to this method blocks until the
 *  internal thread has actually exited (typically using
 *  thread.join() internally).
 *
 *  @param wait_till_stopped waits until the thread has been exited
 *
 *  @return 0 on success or -1 on failure
 */
int FakeTask::stop(const bool wait_till_stopped)
{
	task_cancelled = true;
	if(wait_till_stopped) {
		return task_runner.get();
	}
	return 0;
}

} /* namespace Fake */
