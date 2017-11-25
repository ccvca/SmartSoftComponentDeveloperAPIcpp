//===================================================================================
//
//  Copyright (C) 2017 Alex Lotz, Dennis Stampfer, Matthias Lutz, Christian Schlegel
//
//        lotz@hs-ulm.de
//        stampfer@hs-ulm.de
//        lutz@hs-ulm.de
//        schlegel@hs-ulm.de
//
//        Servicerobotik Ulm
//        Christian Schlegel
//        Ulm University of Applied Sciences
//        Prittwitzstr. 10
//        89075 Ulm
//        Germany
//
//  This file is part of the SmartSoft Component-Developer C++ API.
//
//  Redistribution and use in source and binary forms, with or without modification,
//  are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  3. Neither the name of the copyright holder nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
//  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
//  OF THE POSSIBILITY OF SUCH DAMAGE.
//
//===================================================================================

#ifndef SMARTSOFT_INTERFACES_SMARTITASK_H_
#define SMARTSOFT_INTERFACES_SMARTITASK_H_

#include "smartIShutdownObserver.h"
#include "smartIComponent.h"

namespace Smart {

/** Encapsulates a user-level thread.
 *
 * ITasks should be used to implement user-level threads within a component.
 * ITasks are managed by the provided IComponent instance. For example,
 * during component shutdown, the ITask are automatically triggered to stop().
 * In this way, user-threads can properly shutdown and clean-up their local resources
 * such as e.g. closing internally used device drivers. Please overload the on_shutdown()
 * upcall method in case you need to perform individual cleanup procedures.
 */
class ITask : public IShutdownObserver {
protected:
	/// internal pointer to the component
	IComponent *component;

	/** Default implementation of the IShutdownObserver interface
	 *
	 * 	The default shutdown procedure is to call the stop() method which triggers
	 * 	the thread to stop and awaits until it is closed using the internal join method.
	 */
	virtual void on_shutdown() {
		this->stop();
	}

    /** Tests whether the thread has been signaled to stop.
     *
     * This method allows to implement cooperative thread stopping.
     *
     * @return true if stop was called or false otherwise.
     */
    virtual bool test_canceled() = 0;
public:
	/// Default constructor
	ITask(IComponent *component)
	:	IShutdownObserver(component)
	,	component(component)
	{  }

	/// Default destructor
	virtual ~ITask()
	{  }

    /** Method which runs in a separate thread if activated.
     *
     *  The svc() method has to be provided (i.e. overloaded) by the user
     *  and it implements the activity of the task object.
     *
     *  @return 0 for all OK or -1 otherwise
     */
    virtual int svc (void) = 0;

    /** Creates and starts a new thread (if not yet started)
     *
     *  A new thread is spawned if no thread has been spawned yet before
     *  for this instance.
     *
     *  @return 0 on success (and if thread has already been started) or -1 on failure
     */
    virtual int start() = 0;

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
    virtual int stop(const bool wait_till_stopped=true) = 0;
};

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTITASK_H_ */
