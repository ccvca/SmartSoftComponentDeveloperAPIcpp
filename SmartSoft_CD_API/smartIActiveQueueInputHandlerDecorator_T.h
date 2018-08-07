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

#ifndef SMARTSOFT_INTERFACES_SMARTIACTIVEQUEUEINPITHANDLERDECORATOR_H_
#define SMARTSOFT_INTERFACES_SMARTIACTIVEQUEUEINPITHANDLERDECORATOR_H_

#include "smartITask.h"
#include "smartIInputHandler_T.h"
#include "smartIQueryServerPattern_T.h"

// C++11 includes
#include <mutex>
#include <condition_variable>

namespace Smart {

/** This class decorates a passive IInputHandler and makes it active (with an active internal queue)
 *
 *  This class implements the <b>Decorator</b> design pattern to decorate
 *  a passive IInputHandler by making it active. Internally an active FIFO
 *  queue is created that stores all incoming input-handling requests. An
 *  internal Thread processes this queue by iteratively calling the
 *  process_queue_entry() method.
 */
template <class InputType>
class IActiveQueueInputHandlerDecorator
:	public IInputHandler<InputType>
,	virtual public ITask
{
private:
	std::mutex input_mutex;
	std::condition_variable_any input_cond_var;
	bool cancelled;
	// input-requests list
	std::list<InputType> input_list;
protected:
	/// pointer to the internal handler
	IInputHandler<InputType> *inner_handler;

	/** handler-callback implements IInputHandler interface
	 *
	 * This callback puts the handle-input request onto
	 * an internal FIFO queue and notifies an internal thread
	 * about the availability of a new entry.
	 */
	virtual void handle_input(const InputType& input) {
		std::unique_lock<std::mutex> lock (input_mutex);
		input_list.push_back(input);
		input_cond_var.notify_one();
	}

	/** process a single handle-input entry from the internal FIFO queue
	 *
	 * This method processes an entry from the internal FIFO queue
	 * by delegating the call to the inner-handler. This method is
	 * supposed to be called repeatedly from within an internal
	 * task. This method automatically blocks in case of an
	 * empty FIFO queue until new entries arrive.
	 */
	virtual void process_queue_entry() {
		std::unique_lock<std::mutex> lock (input_mutex);
		// wait in case of empty input-list
		if(input_list.empty()) input_cond_var.wait(lock);
		// return if processing was cancelled
		if(cancelled == true) return;
		// delegate handle-input of front item to inner-handler
		inner_handler->handle_input(input_list.front());
		// consume the front-item (as it has been handled)
		input_list.pop_front();
	}

	/** cancels processing internal FIFO requests
	 *
	 * This method signals the internal thread to stop processing
	 * internal FIFO requests. This is useful e.g. for an immediate
	 * shutdown.
	 */
	virtual void cancel_processing() {
		std::unique_lock<std::mutex> lock (input_mutex);
		// stop processing and release all waiting processing calls
		cancelled = true;
		input_cond_var.notify_all();
	}

	/** checks if processing has been signaled to stop
	 *
	 * If the method cancel_processing() has been called and this
	 * class is not yet destroyed, then this method returns true,
	 * or otherwise false.
	 *
	 * @return true if processing has been cancelled or false otherwise.
	 */
	inline bool processing_cancelled() {
		std::unique_lock<std::mutex> lock (input_mutex);
		return cancelled;
	}

	/** implements individual shutdown procedure
	 *
	 * The shutdown procedure for this class is:
	 * - call cancel_processing()
	 * - call TaskImpl::on_shutdown()
	 * The latter signals the internal thread to stop
	 * awaits until the thread exits.
	 */
	virtual void on_shutdown() {
		this->cancel_processing();
		this->stop();
	}

	/** this is the TaskImpl thread method
	 *
	 * This method implements the TaskImpl task.
	 * The main behavior is to repeatedly calling
	 * process_queue_entry() as long as
	 * processing_cancelled() returns false.
	 */
	virtual int task_execution() {
		while(!this->processing_cancelled()) {
			this->process_queue_entry();
		}
		return 0;
	}

public:
	/** Default constructor
	 *
	 * This constructor decorates an IInputHandler by (1) detaching it from its subject
	 * and by (2) redirecting the handle-input request to an internal FIFO queue which is processed
	 * in an internal thread. This constructor also automatically starts the internal thread.
	 *
	 * @param component The component pointer used to properly initialize the internally used ITask
	 * @param inner_handler The IInputHandler that is being decorated by an active FIFO queue
	 *
	 */
	IActiveQueueInputHandlerDecorator(IComponent *component, IInputHandler<InputType> *inner_handler)
	:	IInputHandler<InputType>(inner_handler->subject)
	,	ITask(component)
	,	inner_handler(inner_handler)
	,	cancelled(false)
	{
		// detach the inner-handler as its handle method will be called by this decorator
		this->inner_handler->detach_self();
	}

	/** Default destructor
	 *
	 * This destructor stops the internal thread and attaches the inner_handler back again to its subject.
	 * In this way, the inner-handler remains fully functional (although passive) even after this decorator
	 * has been destroyed.
	 */
	virtual ~IActiveQueueInputHandlerDecorator()
	{
		// give the handling responsibility back to the inner-handler
		this->inner_handler->attach_self();
	}
};

/** This class is a specialization of an IActiveQueueInputHandlerDecorator that simplifies
 * making an IQueryServerHandler active.
 */
template<class RequestType, class AnswerType, class QIDType>
using IActiveQueueQueryServerHandlerDecorator = IActiveQueueInputHandlerDecorator< QueryServerInputType<RequestType,QIDType> >;

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTIACTIVEQUEUEINPITHANDLERDECORATOR_H_ */
