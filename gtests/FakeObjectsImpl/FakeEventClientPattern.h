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

#ifndef FAKEEVENTCLIENTPATTERN_H_
#define FAKEEVENTCLIENTPATTERN_H_

#include <map>
#include <mutex>
#include <condition_variable>

#include <smartNumericCorrelationId.h>
#include <smartIEventClientPattern_T.h>
#include "FakeClientBase.h"
#include "FakeComponent.h"
#include "FakeEventResult.h"

namespace Fake {

template<class ActivationType, class EventType>
class EventClientPattern
:	public Smart::IEventClientPattern<ActivationType,EventType>
,	public FakeClientBase // this base simulates a basic middleware API with a typical asynchronous communication behavior
{
private:
	std::recursive_mutex client_mutex;
	bool connected;

	Smart::NumericCorrelationId next_id;

	std::map<Smart::NumericCorrelationId, std::shared_ptr<EventResult<EventType>>> received_events;

	virtual void on_sid() override
	{
		this->disconnect();
	}

	virtual void on_update(const std::vector<std::string> &data) override
	{
		std::unique_lock<std::recursive_mutex> client_lock(client_mutex);
		Smart::NumericCorrelationId correlation_id = std::stoi(data.back());
		// create event input object
		Smart::EventInputType<EventType> input;
		input.event_id = std::make_shared<Smart::NumericCorrelationId>(correlation_id);
		// here we call an externally specified conversion (i.e. deserialization) method
		::convert(data, input.event);

		auto event_result = received_events.find(correlation_id);
		if(event_result != received_events.end()) {
			event_result->second->setNewEvent(input.event);
		}

		// now we notify all potentially registered input handlers
		this->notify_input(input);
	}

    Smart::StatusCode checkEventResultStatus(std::shared_ptr<EventResult<EventType>> &event_result)
    {
    	std::unique_lock<std::recursive_mutex> client_lock(client_mutex);

    	if(event_result->hasNewEvent()) {
    		// a new (not yet consumed) event is available
    		return Smart::StatusCode::SMART_OK;
    	} else {
    		if(event_result->isSingleMode() && event_result->isConsumed()) {
    			// event has been consumed in the single mode
    			return Smart::StatusCode::SMART_PASSIVE;
    		}
    		// the event has not yet been received or the last event has been consumed
    		return Smart::StatusCode::SMART_ACTIVE;
    	}

    	return Smart::SMART_ERROR;
    }

    Smart::StatusCode getEventImpl(const Smart::EventIdPtr id, EventType& event, const bool &await_next_event, const Smart::Duration &timeout)
    {
    	std::unique_lock<std::recursive_mutex> client_lock(client_mutex);

    	if(!connected) {
    		return Smart::SMART_DISCONNECTED;
    	}

    	auto cid_ptr = std::dynamic_pointer_cast<Smart::NumericCorrelationId>(id);
    	auto found_event = received_events.find(*cid_ptr);
    	if(found_event == received_events.end()) {
    		return Smart::SMART_WRONGID;
    	}

    	// get a copy of the internal shared pointer (prevents unintended destruction of the
    	// event result entry while this method blocks awaiting the result)
    	auto event_result = found_event->second;

    	// first check if we can consume the event directly or if the event is valid
    	auto status = checkEventResultStatus(event_result);
    	if(status == Smart::StatusCode::SMART_OK) {
    		// we can consume the event right away and don't need to blocking wait
    		event = event_result->consumeEvent();
    		return Smart::SMART_OK;
    	} else if(status != Smart::StatusCode::SMART_ACTIVE) {
    		// we are not ready to receive the event for some reason (see tryEvent possible status codes)
    		return status;
    	}

    	// blocking wait for the next event
    	bool has_new_event = event_result->waitForEvent(client_lock, await_next_event, timeout);

    	if(has_new_event) {
    		event = event_result->consumeEvent();
    		return Smart::SMART_OK;
    	} else if(this->is_blocking == false) {
    		return Smart::SMART_CANCELLED;
    	} else if(event_result->isDeactivated()) {
    		return Smart::SMART_NOTACTIVATED;
    	}

		return Smart::SMART_ERROR;
    }

public:
	EventClientPattern(FakeComponent* component)
	:	Smart::IEventClientPattern<ActivationType,EventType>(component)
	,	FakeClientBase(component)
	,	connected(false)
	,	next_id(0)
	{  }
	EventClientPattern(FakeComponent* component, const std::string& server, const std::string& service)
	:	Smart::IEventClientPattern<ActivationType,EventType>(component, server, service)
	,	FakeClientBase(component)
	,	connected(false)
	,	next_id(0)
	{
		connect(server, service);
	}
	virtual ~EventClientPattern() {
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
    		FakePatternTypeEnum::EVENT_PATTERN, service,
			{ActivationType::identifier(), EventType::identifier()}
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

    	connected = false;

    	for(auto & event: received_events) {
    		event.second->deactivateEvent();
    		std::vector<std::string> serialized_data(1, event.first.to_string());
    		this->send_data(serialized_data);
    	}
    	received_events.clear();

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
    	for(auto & event: received_events) {
    		event.second->signalEvent();
    	}
		return Smart::SMART_OK;
	}

    /** Activate an event with the provided parameters in either "single" or "continuous" mode.
     *
     *  @param mode        "single" or "continuous" mode
     *  @param parameter   activation parameter class (Communication Object)
     *  @param id          is set to the unique id of the event activation
     *
     *  @return status code
     *    - SMART_OK                  : everything is ok, event is activated and <I>id</I> contains
     *                                  a valid activation identifier.
     *    - SMART_DISCONNECTED        : activation not possible since not connected to a server. No
     *                                  valid activation identifier <I>id</I> returned.
     *    - SMART_ERROR_COMMUNICATION : communication problems, event not activated, <I>id</I> is not
     *                                  a valid activation identifier.
     *    - SMART_ERROR               : something went wrong, event not activated, <I>id</I> is not
     *                                  a valid activation identifier.
     */
    virtual Smart::StatusCode activate(const Smart::EventMode &mode, const ActivationType& parameter, Smart::EventIdPtr& id) override
    {
    	std::unique_lock<std::recursive_mutex> client_lock(client_mutex);
    	if(!connected) {
    		return Smart::SMART_DISCONNECTED;
    	}

    	// create id
    	id = std::make_shared<Smart::NumericCorrelationId>(next_id);

    	// serialize activation data
    	std::vector<std::string> serialized_data = ::serialize(parameter);
    	serialized_data.push_back( id->to_string() );
    	serialized_data.push_back( std::to_string(mode) );

		// send activation data to server
		if(FakeClientBase::send_data(serialized_data) != 0) {
			return Smart::SMART_ERROR_COMMUNICATION;
		}

		// create a new entry to later store the event result
		received_events[next_id] = std::make_shared<EventResult<EventType>>(mode);

    	// increment id for next activations
    	next_id++;

    	return Smart::SMART_OK;
    }

    /** Deactivate the event with the specified identifier.
     *
     *  An event must always be deactivated, even if it has already
     *  fired in single mode. This is just necessary for cleanup
     *  procedures and provides a uniform user API independently of the
     *  event mode. Calling deactivate() while there are blocking calls
     *  aborts them with the appropriate status code.
     *
     *  @param id of event to be disabled
     *
     *  @return status code
     *    - SMART_OK                  : everything is ok and event is deactivated
     *    - SMART_WRONGID             : there is no active event available with this id
     *    - SMART_ERROR_COMMUNICATION : communication problems, event not deactivated
     *    - SMART_ERROR               : something went wrong, event not deactivated
     *
     * (Hint: can not return SMART_DISCONNECTED since then each event is for sure also
     *        deactivated resulting in SMART_WRONGID)
     */
    virtual Smart::StatusCode deactivate(const Smart::EventIdPtr id) override
    {
    	std::unique_lock<std::recursive_mutex> client_lock(client_mutex);

    	if(!connected) {
    		return Smart::SMART_WRONGID;
    	}

    	auto cid_ptr = std::dynamic_pointer_cast<Smart::NumericCorrelationId>(id);
    	auto found_event = received_events.find(*cid_ptr);
    	if(found_event == received_events.end()) {
    		return Smart::SMART_WRONGID;
    	}

    	std::vector<std::string> serialized_data(1, id->to_string());

		// send deactivation command to server
		if(FakeClientBase::send_data(serialized_data) != 0) {
			return Smart::SMART_ERROR_COMMUNICATION;
		}

		// remove the event entry
		received_events.erase(found_event);

    	return Smart::SMART_OK;
    }

    /** Check whether event has already fired and return immediately
     *  with status information.
     *
     *  This method does not consume an available event.
     *
     *  @param id of the event activation to be checked
     *
     *  @return status code
     *    - single mode:
     *      - SMART_OK                : event fired already, is still available and can be consumed by
     *                                  calling getEvent(),
     *      - SMART_ACTIVE            : event has not yet fired
     *      - SMART_PASSIVE           : event fired already and is already consumed.
     *      - SMART_WRONGID           : there is no activation available with this <I>id</I>
     *    - continuous mode:
     *      - SMART_OK                : unconsumed event is available. Since events are
     *                                  overwritten this means that at least one new
     *                                  event has been received since the last event
     *                                  consumption.
     *      - SMART_ACTIVE            : currently there is no unconsumed event available.
     *      - SMART_WRONGID           : there is no activation available with this <I>id</I>
     */
    virtual Smart::StatusCode tryEvent(const Smart::EventIdPtr id) override
    {
    	std::unique_lock<std::recursive_mutex> client_lock(client_mutex);

    	if(!connected) {
    		return Smart::SMART_DISCONNECTED;
    	}

    	auto cid_ptr = std::dynamic_pointer_cast<Smart::NumericCorrelationId>(id);
    	auto found_event = received_events.find(*cid_ptr);
    	if(found_event == received_events.end())
    		return Smart::StatusCode::SMART_WRONGID;

    	return checkEventResultStatus(found_event->second);
    }

    /** Blocking call which waits for the event to fire and then consumes the event.
     *
     *  This method consumes an event. Returns immediately if an unconsumed event is
     *  available. Blocks otherwise till event becomes available. If method is called
     *  concurrently from several threads with the same <I>id</I> and method is blocking,
     *  then every call returns with the same <I>event</I> once the event fired. If there is
     *  however already an unconsumed event available, then only one out of the concurrent
     *  calls consumes the event and the other calls return with appropriate status codes.
     *
     *  @param id of the event activation
     *  @param event is set to the returned event if fired (Communication Object)
     *  @param timeout is the timeout time to block the method maximally (default value zero block infinitelly)
     *
     *  - <b>single mode</b>:
     *      <p>
     *      Since an event in single mode fires only once, return immediately
     *      if the event is already consumed. Also return immediately with an
     *      available and unconsumed event and consume it. Otherwise wait until
     *      the event fires.
     *      </p>
     *      <p>
     *      <b>Returns status code</b>:
     *      </p>
     *      <p>
     *        - SMART_OK            : event fired and event is consumed and returned.
     *        - SMART_PASSIVE       : event fired and got consumed already. Returns immediately without
     *                                valid event since it can not fire again in single mode.
     *        - SMART_CANCELLED     : waiting for the event to fire has been aborted or blocking is not
     *                                not allowed anymore. Therefore no valid <I>event</I> is returned.
     *        - SMART_DISCONNECTED  : client is disconnected or got disconnected while waiting and
     *                                therefore no valid <I>event</I> is returned and the activation
     *                                identifier <I>id</I> is also not valid any longer due to
     *                                automatic deactivation.
     *        - SMART_NOTACTIVATED  : got deactivated while waiting and therefore <I>event</I> not valid and
     *                                also <I>id</I> not valid any longer.
     *        - SMART_WRONGID       : there is no activation available with this <I>id</I> and therefore
     *                                <I>event</I> not valid.
     *      </p>
     *
     *  - <b>continuous mode</b>:
     *     <p>
     *     Returns immediately if an unconsumed event is
     *     available. Returns the newest unconsumed event since
     *     activation. Otherwise waits until the event fires again.
     *     </p>
     *     <p>
     *     <b>Returns status code</b>:
     *     </p>
     *     <p>
     *        - SMART_OK            : unconsumed event is available and event is consumed and returned.
     *                                Due to the overwriting behavior, only the latest event is available.
     *        - SMART_CANCELLED     : blocking is not allowed anymore therefore blocking call has been aborted and
     *                                <I>event</I> is not valid.
     *        - SMART_DISCONNECTED  : got disconnected while waiting and therefore <I>event</I> not valid and
     *                                also <I>id</I> not valid any longer due to automatic deactivation.
     *        - SMART_NOTACTIVATED  : got deactivated while waiting and therefore <I>event</I> not valid and
     *                                also <I>id</I> not valid any longer.
     *        - SMART_WRONGID       : there is no activation available with this <I>id</I> and therefore
     *                                <I>event</I> not valid.
     *     </p>
     */
    virtual Smart::StatusCode getEvent(const Smart::EventIdPtr id, EventType& event, const Smart::Duration &timeout=Smart::Duration::max()) override
    {
    	bool await_next_event = false;
    	return getEventImpl(id, event, await_next_event, timeout);
    }

    /** Blocking call which waits for the next event.
     *
     *  This methods waits for the <I>next</I> arriving event to make sure that only events arriving
     *  after entering the method are considered. Method consumes event. An old event that has been
     *  fired is ignored (in contrary to getEvent()). If method is called concurrently from several
     *  threads with the same <I>id</I>, then every call returns with the same <I>event</I> once the
     *  event fired.
     *
     *  @param id of the event activation
     *  @param event is set to the returned event if fired (Communication Object)
     *  @param timeout is the timeout time to block the method maximally (default value zero block infinitelly)
     *
     *  - <b>single mode</b>:
     *    <p>
     *    In single mode one misses the event if it fired before entering this member function.
     *    </p>
     *    <p>
     *    <b>Returns status code</b>:
     *    </p>
     *    <p>
     *      - SMART_OK            : event fired while waiting for the event and event is consumed
     *                              and returned
     *      - SMART_PASSIVE       : event already fired between activation and calling this member
     *                              function and is therefore missed or event has already been
     *                              consumed and can not fire again in single mode. Does not block
     *                              indefinitely and returns no valid <I>event</I>.
     *      - SMART_CANCELLED     : event not yet fired and waiting for the event has been aborted or
     *                              blocking is not allowed anymore. No valid <I>event</I> is returned.
     *      - SMART_DISCONNECTED  : got disconnected while waiting and therefore <I>event</I> not valid
     *                              and also <I>id</I> not valid any longer due to automatic deactivation.
     *      - SMART_NOTACTIVATED  : got deactivated while waiting and therefore <I>event</I> not valid and
     *                              also <I>id</I> not valid any longer.
     *      - SMART_WRONGID       : there is no activation available with this <I>id</I> and therefore
     *                              <I>event</I> not valid.
     *    </p>
     *
     *  - <b>continuous mode</b>:
     *    <p>
     *    Makes sure that only events fired after entering this member function are considered.
     *    </p>
     *    <p>
     *    <b>Returns status code</b>:
     *    </p>
     *    <p>
     *      - SMART_OK            : event fired while waiting for the event and event is consumed
     *                              and returned
     *      - SMART_CANCELLED     : waiting for the next event has been aborted or blocking is not
     *                              allowed anymore. No valid <I>event</I> is returned.
     *      - SMART_DISCONNECTED  : got disconnected while waiting and therefore <I>event</I> not valid
     *                              and also <I>id</I> not valid any longer due to automatic deactivation.
     *      - SMART_NOTACTIVATED  : got deactivated while waiting and therefore <I>event</I> not valid and
     *                              also <I>id</I> not valid any longer.
     *      - SMART_WRONGID       : there is no activation available with this <I>id</I> and therefore
     *                              <I>event</I> not valid.
     *    </p>
     */
    virtual Smart::StatusCode getNextEvent(const Smart::EventIdPtr id, EventType& event, const Smart::Duration &timeout=Smart::Duration::max()) override
    {
    	bool await_next_event = true;
    	return getEventImpl(id, event, await_next_event, timeout);
    }
};

} /* namespace Fake */

#endif /* FAKEEVENTCLIENTPATTERN_H_ */
