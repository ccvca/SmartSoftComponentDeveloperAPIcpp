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

#ifndef SMARTIPROCESSINGPATTERNS_T_H_
#define SMARTIPROCESSINGPATTERNS_T_H_

#include <list>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include "smartITask.h"
#include "smartIQueryServerPattern_T.h"

namespace Smart {

template<class RequestType, class AnswerType>
class IActiveQueryServerHandler
:	public Smart::IQueryServerHandler<RequestType,AnswerType>
,	virtual public ITask
{
public:
	// these aliases can be used in derived classes to simplify creation of the constructors
	using IQueryServer = IQueryServerPattern<RequestType,AnswerType>;
	using IQueryServerHandlerPtr = std::shared_ptr<Smart::IQueryServerHandler<RequestType,AnswerType>>;

	IActiveQueryServerHandler(IQueryServerHandlerPtr inner_handler_ptr)
	:	inner_handler_ptr(inner_handler_ptr)
	,	signalled_to_stop(false)
	{  }
	virtual ~IActiveQueryServerHandler() = default;

private:
	IQueryServerHandlerPtr inner_handler_ptr;
	std::mutex mutex;
	std::condition_variable handler_cond_var;

	struct RequestEntry {
		IQueryServer *server;
		QueryIdPtr id;
		RequestType request;
	};
	std::list<RequestEntry> request_list;

	virtual void handleQuery(IQueryServer &server, const QueryIdPtr &id, const RequestType& request) {
		std::unique_lock<std::mutex> scoped_lock(mutex);
		RequestEntry entry;
		entry.server = &server;
		entry.id = id;
		entry.request = request;
		request_list.push_back(entry);
		scoped_lock.unlock();

		handler_cond_var.notify_all();
	}

protected:
	std::atomic<bool> signalled_to_stop;
	void signal_to_stop() {
		signalled_to_stop = true;
		handler_cond_var.notify_all();
	}
	int process_fifo_queue()
	{
		while(!signalled_to_stop) {
			std::unique_lock<std::mutex> scoped_lock(mutex);
			if(request_list.empty()) {
				handler_cond_var.wait(scoped_lock);
				if(signalled_to_stop)
					return 0;
			}
			RequestEntry entry = request_list.front();
			request_list.pop_front();
			scoped_lock.unlock();

			if(inner_handler_ptr) {
				inner_handler_ptr->handleQuery(*entry.server, entry.id, entry.request);
			}
		}
		return 0;
	}
};

} /* namespace Smart */

#endif /* SMARTIPROCESSINGPATTERNS_T_H_ */
