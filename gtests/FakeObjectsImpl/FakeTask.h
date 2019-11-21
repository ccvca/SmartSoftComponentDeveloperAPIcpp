/*
 * FakeTask.h
 *
 *  Created on: Jul 24, 2019
 *      Author: alexej
 */

#ifndef FAKETASK_H_
#define FAKETASK_H_

#include <smartITask.h>

#include <future>
#include <atomic>

namespace Fake {

class FakeTask : public Smart::ITask
{
private:
	std::future<int> task_runner;
	std::atomic<bool> task_cancelled;

protected:

    /** Tests whether the thread has been signaled to stop.
     *
     * This method allows to implement cooperative thread stopping.
     *
     * @return true if stop was called or false otherwise.
     */
    virtual bool test_canceled() override;

    /** Blocks execution of the calling thread during the span of time specified by rel_time.
     *
     *  Thread-sleeping is sometimes platform-specific. This method encapsulates the
     *  blocking sleep. Calling this method blocks the execution of the calling thread
     *  for a time specified by rel_time.
     *
     *  @param duration relative time duration for the thread to sleep
     */
    virtual void sleep_for(const Smart::Duration &duration) override;

    /** Method which runs in a separate thread if activated.
     *
     *  The task_execution() method has to be provided (i.e. overloaded) by the user
     *  and it implements the activity of the task object.
     *
     *  @return 0 for all OK or -1 otherwise
     */
    virtual int task_execution() = 0;
public:
	FakeTask(Smart::IComponent *component = nullptr);
	virtual ~FakeTask() = default;

    /** Creates and starts a new thread (if not yet started)
     *
     *  A new thread is spawned if no thread has been spawned yet before
     *  for this instance.
     *
     *  @return 0 on success (and if thread has already been started) or -1 on failure
     */
    virtual int start() override;

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
    virtual int stop(const bool wait_till_stopped=true) override;
};

} /* namespace Fake */

#endif /* FAKETASK_H_ */
