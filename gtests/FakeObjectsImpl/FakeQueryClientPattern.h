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

#ifndef FAKEQUERYCLIENTPATTERN_H_
#define FAKEQUERYCLIENTPATTERN_H_

#include <map>
#include <mutex>

#include <smartNumericCorrelationId.h>
#include <smartIQueryClientPattern_T.h>
#include "FakeClientBase.h"
#include "FakeComponent.h"

namespace Fake {

template<class RequestType, class AnswerType>
class QueryClientPattern
:	public Smart::IQueryClientPattern<RequestType,AnswerType>
,	public FakeClientBase // this base simulates a basic middleware API with a typical asynchronous communication behavior
{
private:
	std::recursive_mutex client_mutex;
	bool connected;
	Smart::NumericCorrelationId next_id;

	enum class QueryStatus {
		PENDING,
		HAS_ANSWER
	};

	struct PendingQuery {
		AnswerType answer;
		std::condition_variable_any cond_var;
		QueryStatus status = QueryStatus::PENDING;
	};
	std::map<Smart::NumericCorrelationId, std::shared_ptr<PendingQuery>> pending_queries;

	virtual void on_sid() override
	{
		this->disconnect();
	}

	virtual void on_update(const std::vector<std::string> &data) override
	{
		std::unique_lock<std::recursive_mutex> client_lock(client_mutex);

		Smart::NumericCorrelationId id = std::stoi(data.back());

		auto query_it = pending_queries.find(id);
		if(query_it != pending_queries.end()) {
			auto answer_ptr = query_it->second;
			// here we use an externally specified conversion (i.e. deserialization) method
			::convert(data, answer_ptr->answer);
			answer_ptr->status = QueryStatus::HAS_ANSWER;
			answer_ptr->cond_var.notify_all();
		}
	}
public:
	QueryClientPattern(FakeComponent* component)
	:	Smart::IQueryClientPattern<RequestType,AnswerType>(component)
	,	FakeClientBase(component)
	,	connected(false)
	,	next_id(0)
	{  }
	QueryClientPattern(FakeComponent* component, const std::string& server, const std::string& service)
	:	Smart::IQueryClientPattern<RequestType,AnswerType>(component, server, service)
	,	FakeClientBase(component)
	,	connected(false)
	,	next_id(0)
	{
		connect(server, service);
	}
	virtual ~QueryClientPattern()
	{
		this->disconnect();
	}

    /** Connect this service requestor to the denoted service provider. An
     *  already established connection is first disconnected. See disconnect()
     *
     *  It is no problem to change the connection to a service provider at any
     *  point in time irrespective of any other calls.
     *
     * @param server     name of the server (i.e. the component-name to connect to)
     * @param service    name of the service (i.e. the port-name of the component to connect to)
     *
     *  @return status code
     *   - SMART_OK                  : everything is OK and connected to the specified service.
     *   - SMART_SERVICEUNAVAILABLE  : the specified service is currently not available and the
     *                                 requested connection can not be established. Service
     *                                 requestor is now not connected to any service provider.
     *   - SMART_INCOMPATIBLESERVICE : the specified service provider is not compatible (wrong communication
     *                                 pattern or wrong communication objects) to this service requestor and
     *                                 can therefore not be connected. Service requestor is now not connected
     *                                 to any service provider.
     *   - SMART_ERROR_COMMUNICATION : communication problems, service requestor is now not connected to any
     *                                 service provider.
     *   - SMART_ERROR               : something went wrong, service requestor is now not connected to any
     *                                 service provider.
     */
    virtual Smart::StatusCode connect(const std::string& server, const std::string& service) override
    {
    	std::unique_lock<std::recursive_mutex> client_lock(client_mutex);

    	this->disconnect();

    	// this simulates a middleware connection
    	int result = FakeClientBase::middleware_connect(
    		FakePatternTypeEnum::QUERY_PATTERN, service,
			{RequestType::identifier(), AnswerType::identifier()}
    	);

    	if(result == 1) {
    		connected = true;
    		return Smart::SMART_OK;
    	} else if(result == 0) {
    		return Smart::SMART_INCOMPATIBLESERVICE;
    	} else if(result == -1) {
    		return Smart::SMART_SERVICEUNAVAILABLE;
    	}
    	return Smart::SMART_ERROR_COMMUNICATION;
    }

    /** Disconnect the service requestor from the service provider.
     *
     *  It is no problem to change the connection to a service provider at any
     *  point in time irrespective of any other calls.
     *  @return status code
     *   - SMART_OK                  : everything is OK and service requestor is disconnected from
     *                                 the service provider.
     *   - SMART_ERROR_COMMUNICATION : something went wrong at the level of the intercomponent
     *                                 communication. At least the service requestor is in the
     *                                 disconnected state irrespective of the service provider
     *                                 side clean up procedures.
     *   - SMART_ERROR               : something went wrong. Again at least the service requestor
     *                                 is in the disconnected state.
     */
    virtual Smart::StatusCode disconnect() override
    {
    	std::unique_lock<std::recursive_mutex> client_lock(client_mutex);

    	for(auto& entry: pending_queries) {
    		entry.second->cond_var.notify_all();
    	}
    	pending_queries.clear();

    	connected = false;

    	if(FakeClientBase::middleware_disconnect() != 0) {
    		return Smart::SMART_ERROR_COMMUNICATION;
    	}

    	return Smart::SMART_OK;
    }

    /** Allow or abort and reject blocking calls.
     *
     *  If blocking is set to false all blocking calls return with SMART_CANCELLED. This can be
     *  used to abort blocking calls.
     *
     *  @param blocking  true/false
     *
     *  @return status code
     *   - SMART_OK                  : new mode set
     *   - SMART_ERROR               : something went wrong
     */
    virtual Smart::StatusCode blocking(const bool blocking) override
    {
    	std::unique_lock<std::recursive_mutex> client_lock(client_mutex);
    	this->is_blocking = blocking;
    	for(auto& entry: pending_queries) {
			entry.second->cond_var.notify_all();
		}
		return Smart::SMART_OK;
	}

    /** Blocking Query.
     *
     *  Perform a blocking query and return only when the query answer
     *  is available. Member function is thread safe and thread reentrant.
     *
     *  @param request send this request to the server (Communication Object)
     *  @param answer  returned answer from the server (Communication Object)
     *
     *  @return status code:
     *    - SMART_OK                  : everything is ok and <I>answer</I> contains answer
     *    - SMART_CANCELLED           : blocking is not allowed or is not allowed anymore and therefore
     *                                  pending query is aborted, answer is lost and <I>answer</I>
     *                                  contains no valid answer.
     *    - SMART_DISCONNECTED        : the client is either disconnected and no query
     *                                  can be made or it got disconnected and a pending
     *                                  query is aborted without answer. In both cases,
     *                                  <I>answer</I> is not valid.
     *    - SMART_ERROR_COMMUNICATION : communication problems, <I>answer</I> is not valid.
     *    - SMART_ERROR               : something went wrong, <I>answer</I> is not valid.
     */
    using Smart::IQueryClientPattern<RequestType,AnswerType>::query;

    /** Asynchronous Query.
     *
     *  Perform a query and receive the answer later, returns immediately.
     *  Member function is thread safe and reentrant.
     *
     *  @param request send this request to the server (Communication Object)
     *  @param id      is set to the identifier which is later used to receive
     *                 the reply to this request
     *
     *  @return status code:
     *    - SMART_OK                  : everything is ok and <I>id</I> contains query identifier
     *                                  used to either fetch or discard the answer.
     *    - SMART_DISCONNECTED        : request is rejected since client is not connected to a server
     *                                  and therefore <I>id</I> is not a valid identifier.
     *    - SMART_ERROR_COMMUNICATION : communication problems, <I>id</I> is not valid.
     *    - SMART_ERROR               : something went wrong, <I>id</I> is not valid.
     */
    virtual Smart::StatusCode queryRequest(const RequestType& request, Smart::QueryIdPtr& id) override
    {
    	std::unique_lock<std::recursive_mutex> client_lock(client_mutex);
    	if(!connected) {
    		return Smart::SMART_DISCONNECTED;
    	}

    	// save id as a shared-ptr
		id = std::make_shared<Smart::NumericCorrelationId>(next_id);

		// create an empty pending query for the current ID
		pending_queries[next_id] = std::make_shared<PendingQuery>();

    	// increment id for next requests
    	next_id++;

		// collect data to be send
		std::vector<std::string> serialized_request = ::serialize(request);
		serialized_request.push_back( id->to_string() );

		// send request to server
		if(FakeClientBase::send_data(serialized_request) != 0) {
			this->queryDiscard(id);
			return Smart::SMART_ERROR_COMMUNICATION;
		}

		return Smart::SMART_OK;
    }

    /** Check if answer is available.
     *
     *  Non-blocking call to fetch the answer belonging to the given identifier.
     *  Returns immediately. Member function is thread safe and reentrant.
     *
     *  @warning
     *    It is not allowed to call queryReceive(), queryReceiveWait() or queryDiscard() concurrently
     *    with the <I>same</I> query id (which is not a restriction since it makes no sense !)
     *
     *  @param id      provides the identifier of the query
     *  @param answer  is set to the answer returned from the server if it was available
     *
     *  @return status code:
     *    - SMART_OK           : everything is ok and <I>answer</I> contains the answer
     *    - SMART_WRONGID      : no pending query with this identifier available, therefore no valid
     *                           <I>answer</I> returned.
     *    - SMART_NODATA       : answer not yet available, therefore try again later. The identifier <I>id</I>
     *                           keeps valid, but <I>answer</I> contains no valid answer.
     *    - SMART_DISCONNECTED : the answer belonging to the <I>id</I> can not be received
     *                           anymore since the client got disconnected. <I>id</I> is
     *                           not valid any longer and <I>answer</I> contains no valid answer.
     *    - SMART_ERROR        : something went wrong, <I>answer</I> contains no answer and <I>id</I> is
     *                           not valid any longer.
     *
     */
    virtual Smart::StatusCode queryReceive(const Smart::QueryIdPtr id, AnswerType& answer) override
    {
		std::unique_lock<std::recursive_mutex> client_lock(client_mutex);

    	if(!connected) {
    		return Smart::SMART_DISCONNECTED;
    	}

    	auto cid_ptr = std::dynamic_pointer_cast<Smart::NumericCorrelationId>(id);
    	if(!cid_ptr) {
    		return Smart::SMART_WRONGID;
    	}
		auto query_it = pending_queries.find(*cid_ptr);
		if(query_it == pending_queries.end()) {
			return Smart::SMART_WRONGID;
		}

		auto answer_it = query_it->second;
		if(answer_it->status == QueryStatus::PENDING) {
			// query has not yet been answered
			return Smart::SMART_NODATA;
		} else if(answer_it->status == QueryStatus::HAS_ANSWER) {
			// copy answer object
			answer = answer_it->answer;

			// consume the pending query
			pending_queries.erase(query_it);

			return Smart::SMART_OK;
		}

    	return Smart::SMART_ERROR;
    }

    /** Wait for reply.
     *
     *  Blocking call to fetch the answer belonging to the given identifier. Waits until
     *  the answer is received.
     *
     *  @warning
     *    It is not allowed to call queryReceive(), queryReceiveWait() or queryDiscard() concurrently
     *    with the <I>same</I> query id (which is not a restriction since it makes no sense !)
     *
     *  @param id       provides the identifier of the query
     *  @param answer   is set to the answer returned from the server if it was available
     *  @param timeout is the timeout time to block the method maximally (default value max blocks infinitely)
     *
     *  @return status code:
     *    - SMART_OK           : everything is ok and <I>answer</I> contains the answer
     *    - SMART_WRONGID      : no pending query with this identifier available, therefore no
     *                           valid <I>answer</I> returned.
     *    - SMART_CANCELLED    : blocking call is not allowed or is not allowed anymore and therefore
     *                           blocking call is aborted and no valid <I>answer</I> is returned. The
     *                           query identifier <I>id</I> keeps valid and one can either again call
     *                           queryReceive(), queryReceiveWait() or discard the answer by calling
     *                           queryDiscard().
     *    - SMART_DISCONNECTED : blocking call is aborted and the answer belonging to <I>id</I> can not
     *                           be received anymore since client got disconnected. <I>id</I> is not valid
     *                           any longer and <I>answer</I> contains no valid answer.
     *    - SMART_ERROR        : something went wrong, <I>answer</I> contains no answer and <I>id</I> is
     *                           not valid any longer.
     *
     */
    virtual Smart::StatusCode queryReceiveWait(const Smart::QueryIdPtr id, AnswerType& answer, const Smart::Duration &timeout=Smart::Duration::max()) override
    {
		std::unique_lock<std::recursive_mutex> client_lock(client_mutex);

    	if(!this->is_blocking) {
    		return Smart::SMART_CANCELLED;
    	}

    	// try getting the answer directly without waiting (maybe one is already there)
    	auto status = queryReceive(id, answer);
    	if(status == Smart::SMART_OK) {
    		return Smart::SMART_OK;
    	} else if(status != Smart::SMART_NODATA) {
    		// we will only continue if none of the error cases happened
    		// the current query still is pending, otherwise we just return any
    		// error status directly
    		return status;
    	}

    	auto cid_ptr = std::dynamic_pointer_cast<Smart::NumericCorrelationId>(id);
		auto query_it = pending_queries.find(*cid_ptr);
		if(query_it == pending_queries.end()) {
			// this case should not happen as we checked this in queryReceive, but just in case...
			return Smart::SMART_WRONGID;
		}

		// we copy the answer shared pointer to make ensure it is not deleted
		// while we are waiting for the result
		auto answer_it = query_it->second;

		if(timeout != Smart::Duration::max()) {
			// blocking wait until the answer becomes available or any other related event occurs
			auto result = answer_it->cond_var.wait_for(client_lock, timeout);

			if(result == std::cv_status::timeout) {
				return Smart::SMART_TIMEOUT;
			}
		} else {
			// wait infinitely until some event occurs
			answer_it->cond_var.wait(client_lock);
		}

		if(!this->is_blocking) {
			return Smart::SMART_CANCELLED;
		}

		// we handled all the blocking-wait-specific cases so we can delegate the actual
		// answer retrieval to the non-blocking call
		return queryReceive(id, answer);
    }

    /** Discard the pending answer with the identifier <I>id</I>
     *
     *  Call this member function if you do not want to get the answer of a request anymore which
     *  was invoked by queryRequest(). This member function invalidates the identifier <I>id</I>.
     *
     *  @warning
     *    This member function does NOT abort blocking calls ! This is done by the blocking() member
     *    function. It has to be called if you have not yet received an answer and the identifier is
     *    still valid, for example due to a CANCELLED return value, and you don't want to get the
     *    answer anymore.
     *
     *  @warning
     *    It is not allowed to call queryReceive(), queryReceiveWait() or queryDiscard() concurrently
     *    with the <I>same</I> query id (which is not a restriction since it makes no sense !)
     *
     *  @param id  provides the identifier of the query
     *
     *  @return status code:
     *    - SMART_OK           : everything is ok and query with the identifier <I>id</I> discarded.
     *    - SMART_WRONGID      : no pending query with this identifier.
     *    - SMART_ERROR        : something went wrong, <I>id</I> not valid any longer.
     *
     */
    virtual Smart::StatusCode queryDiscard(const Smart::QueryIdPtr id) override
    {
		std::unique_lock<std::recursive_mutex> client_lock(client_mutex);

		auto cid_ptr = std::dynamic_pointer_cast<Smart::NumericCorrelationId>(id);
		auto query_it = pending_queries.find(*cid_ptr);
		if(query_it == pending_queries.end()) {
			return Smart::SMART_WRONGID;
		}

		// notify the related blocking calls to unblock
		query_it->second->cond_var.notify_all();

		// we can safely erase (i.e. consume) the entry as others will have a local
		// copy of the shared pointer
		pending_queries.erase(query_it);

    	return Smart::SMART_OK;
    }
};

} /* namespace Fake */

#endif /* FAKEQUERYCLIENTPATTERN_H_ */
