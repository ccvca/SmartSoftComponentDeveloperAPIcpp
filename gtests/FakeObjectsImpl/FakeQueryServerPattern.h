//===================================================================================
//
//  Copyright (C) 2019 Alex Lotz
//
//        lotz@hs-ulm.de
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

#ifndef FAKEQUERYSERVERPATTERN_H_
#define FAKEQUERYSERVERPATTERN_H_

#include <map>
#include <mutex>

#include <smartNumericCorrelationId.h>
#include <smartIQueryServerPattern_T.h>
#include "FakeComponent.h"
#include "FakeServerBase.h"

namespace Fake {

template<class RequestType, class AnswerType>
class QueryServerPattern
:	public Smart::IQueryServerPattern<RequestType,AnswerType>
,	public FakeServerBase
{
public:
	using IQueryServerBase = Smart::IQueryServerPattern<RequestType,AnswerType>;
	using typename IQueryServerBase::IQueryServerHandlerPtr;

	QueryServerPattern(FakeComponent* component, const std::string& serviceName, IQueryServerHandlerPtr query_handler = nullptr)
	:	IQueryServerBase(component, serviceName, query_handler)
	,	FakeServerBase(component)
	{
		FakeServerBase::register_self_as(
			FakePatternTypeEnum::QUERY_PATTERN,	serviceName,
			{RequestType::identifier(), AnswerType::identifier()}
		);
	}
	virtual ~QueryServerPattern() {
		FakeServerBase::unregister_self();
		this->serverInitiatedDisconnect();
	}

    /** Provide answer to be sent back to the requestor.
     *
     *  Member function is thread safe and thread reentrant.
     *
     *  @param id identifies the request to which the answer belongs
     *  @param answer is the reply itself.
     *
     *  @return status code:
     *    - SMART_OK                  : everything is ok and answer sent to requesting client
     *    - SMART_WRONGID             : no pending query with that <I>id</I> known
     *    - SMART_DISCONNECTED        : answer not needed anymore since client
     *                                  got disconnected meanwhile
     *    - SMART_ERROR_COMMUNICATION : communication problems
     *    - SMART_ERROR               : something went wrong
     */
    virtual Smart::StatusCode answer(const Smart::QueryIdPtr id, const AnswerType& answer) override
    {
    	std::unique_lock<std::recursive_mutex> server_lock(FakeServerBase::server_mutex);
    	auto cid_ptr = std::dynamic_pointer_cast<Smart::NumericCorrelationId>(id);
    	auto found_request_it = pending_requests.find(*cid_ptr);
    	if(found_request_it == pending_requests.end()) {
    		return Smart::SMART_WRONGID;
    	}

    	auto found_client_it = FakeServerBase::connected_clients.find(found_request_it->second);
    	if(found_client_it == FakeServerBase::connected_clients.end()) {
    		return Smart::SMART_DISCONNECTED;
    	}

    	// create serialized answer vector
    	std::vector<std::string> serialized_answer = ::serialize(answer);
    	// the id is pushed back in addition
    	serialized_answer.push_back(id->to_string());

    	// send answer to client
    	FakeServerBase::send_data_to(*found_client_it, serialized_answer);

    	// consume the answered request
    	pending_requests.erase(found_request_it);

    	return Smart::SMART_OK;
    }

private:
	std::map<Smart::NumericCorrelationId, FakeClientBase*> pending_requests;

	virtual void on_input_from(FakeClientBase *client, const std::vector<std::string> &data) override
	{
		std::unique_lock<std::recursive_mutex> server_lock(FakeServerBase::server_mutex);
		RequestType request;
		Smart::NumericCorrelationId correlation_id;
		// here we call an externally specified conversion (i.e. deserialization) method
		::convert(data, request);
		correlation_id = std::stoi(data.back());

		pending_requests[correlation_id] = client;

		auto query_id = std::make_shared<Smart::NumericCorrelationId>(correlation_id);

		server_lock.unlock();

		IQueryServerBase::handleQuery(query_id, request);
	}

	virtual void serverInitiatedDisconnect() override
	{
		FakeServerBase::disconnect_all_clients();
	}
};

} /* namespace Fake */

#endif /* FAKEQUERYSERVERPATTERN_H_ */
