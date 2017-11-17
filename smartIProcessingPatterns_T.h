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

#ifndef SMARTSOFT_INTERFACES_SMARTIPROCESSINGPATTERNS_H_
#define SMARTSOFT_INTERFACES_SMARTIPROCESSINGPATTERNS_H_

#include "smartITask.h"
#include "smartIInputHandler_T.h"

// C++11 includes
#include <mutex>
#include <condition_variable>

namespace Smart {

template <class InputType>
class IActiveQueueInputHandlerDecorator : public IInputHandler<InputType> {
private:
	std::mutex input_mutex;
	std::condition_variable_any input_cond_var;
	// input-requests list
	std::list<InputType> input_list;
protected:
	IInputHandler<InputType> *inner_handler;
	virtual void handle_input(const InputType& input) {
		std::unique_lock<std::mutex> lock (input_mutex);
		input_list.push_back(input);
		input_cond_var.notify_one();
	}

	virtual void process_queue_entry() {
		std::unique_lock<std::mutex> lock (input_mutex);
		if(input_list.empty()) input_cond_var.wait(lock);
		inner_handler->handle_input(input_list.front());
		input_list.pop_front();
	}

public:
	IActiveQueueInputHandlerDecorator(IInputSubject<InputType> *subject, IInputHandler<InputType> *inner_handler)
	:	IInputHandler<InputType>(subject)
	,	inner_handler(inner_handler)
	{
		// detach the inner-handler as its handle method will be called by this decorator
		this->subject->detach(inner_handler);
	}
	virtual ~IActiveQueueInputHandlerDecorator()
	{
		// gave the handling responsibility back to the inner-handler
		this->subject->attach(inner_handler);
	}
};

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTIPROCESSINGPATTERNS_H_ */
