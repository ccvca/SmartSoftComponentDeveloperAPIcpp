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

#ifndef FAKEPUSHCLIENTPATTERN_H_
#define FAKEPUSHCLIENTPATTERN_H_

#include <mutex>

#include <smartIPushClientPattern_T.h>
#include "FakeClientBase.h"
#include "FakeComponent.h"

namespace Fake {

template <typename DataType>
class PushClientPattern
:	public Smart::IPushClientPattern<DataType>
,	public FakeClientBase // this base simulates a basic middleware API with a typical asynchronous communication behavior
{
private:
	std::recursive_mutex client_mutex;
	std::condition_variable_any data_cond_var;
	bool connected;
	bool subscribed;
	bool has_data;
	DataType last_update;

	virtual void on_sid() override
	{
		this->disconnect();
	}

	virtual void on_update(const std::vector<std::string> &data) override
	{
		std::unique_lock<std::recursive_mutex> client_lock(client_mutex);
		// here we call an externally specified conversion (i.e. deserialization) method
		::convert(data, last_update);
		has_data = true;
		client_lock.unlock();
		// signal waiting calls to release
		data_cond_var.notify_all();
		// trigger the notification interface
		this->notify_input(last_update);
	}

public:
	PushClientPattern(FakeComponent* component)
	:	Smart::IPushClientPattern<DataType>(component)
	,	FakeClientBase(component)
	,	connected(false)
	,	subscribed(false)
	,	has_data(false)
	{  }
	PushClientPattern(FakeComponent* component, const std::string& server, const std::string& service)
	:	Smart::IPushClientPattern<DataType>(component, server, service)
	,	FakeClientBase(component)
	,	connected(false)
	,	subscribed(false)
	,	has_data(false)
	{
		connect(server, service);
	}
	virtual ~PushClientPattern() {
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
    		FakePatternTypeEnum::PUSH_PATTERN,
			service, std::vector<std::string>(1, DataType::identifier())
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

    	this->unsubscribe();

    	connected = false;

    	data_cond_var.notify_all();

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
    	data_cond_var.notify_all();
		return Smart::SMART_OK;
	}

    /** Subscribe at the server to periodically get every n-th update. A
     *  newly subscribed client gets the next available new data and is
     *  then updated with regard to its individual prescale-factor.
     *  If the prescale factor is omitted then every update will be received.
     *
     *  @param prescale  whole-numbered value to set the update rate to
     *                   every n-th value (must be greater than 0)
     *
     *  @return status code
     *    - SMART_OK                  : everything is ok and client is subscribed
     *    - SMART_DISCONNECTED        : client is not connected to a server and can therefore
     *                                  not subscribe for updates, not subscribed
     *    - SMART_ERROR_COMMUNICATION : communication problems, not subscribed
     *    - SMART_ERROR               : something went wrong, not subscribed
     */
    virtual Smart::StatusCode subscribe(const unsigned int &prescale = 1) override
    {
    	std::unique_lock<std::recursive_mutex> client_lock(client_mutex);
    	if(!connected) {
    		return Smart::SMART_DISCONNECTED;
    	}

    	if(prescale < 1) {
    		return Smart::SMART_ERROR_COMMUNICATION;
    	}

    	std::vector<std::string> subscription_data(1, std::to_string(prescale));
    	bool wait_for_ack = true;
    	if(FakeClientBase::send_data(subscription_data, wait_for_ack) == 0) {
			subscribed = true;
			return Smart::SMART_OK;
    	}

    	return Smart::SMART_ERROR_COMMUNICATION;
	}

    /** Unsubscribe to get no more updates. All blocking calls are aborted with the appropriate
     *  status and yet received and still buffered data is deleted to avoid returning old data.
     *
     *  @return status code
     *    - SMART_OK                  : everything is ok and client is now unsubscribed or
     *                                  has already been unsubscribed
     *    - SMART_ERROR_COMMUNICATION : communication problems, not unsubscribed
     *    - SMART_ERROR               : something went wrong, not unsubscribed
     *
     * (can not return SMART_DISCONNECTED since then client is for sure also unsubscribed
     *  which results in SMART_OK)
     */
    virtual Smart::StatusCode unsubscribe() override
    {
    	std::unique_lock<std::recursive_mutex> client_lock(client_mutex);

    	if(!connected) {
			return Smart::SMART_OK;
		}

    	subscribed = false;
    	has_data = false;

    	std::vector<std::string> empty_data;
    	bool wait_for_ack = true;
    	if(FakeClientBase::send_data(empty_data, wait_for_ack) == 0) {
			return Smart::SMART_OK;
    	}

		return Smart::SMART_ERROR_COMMUNICATION;
	}

    /** Non-blocking call to immediately return the latest available
     *  data buffered at the client side from the most recent update.
     *
     *  No data is returned as long as no update is received since
     *  subscription. To avoid returning old data, no data is
     *  returned after the client is unsubscribed or when the
     *  server is not active.
     *
     * @param d is set to the newest currently available data
     *
     * @return status code
     *   - SMART_OK                  : everything ok and latest data since client got subscribed
     *                                 is returned.
     *   - SMART_NODATA              : client has not yet received an update since subscription and
     *                                 therefore no data is available and no data is returned.
     *   - SMART_UNSUBSCRIBED        : no data available since client is not subscribed and can
     *                                 therefore not receive updates. Method does not return old data from
     *                                 last subscription since these might be based on too old parameter
     *                                 settings. To get data one has to be subscribed.
     *   - SMART_DISCONNECTED        : no data returned since client is even not connected to a server.
     *   - SMART_ERROR               : something went wrong
     */
    virtual Smart::StatusCode getUpdate(DataType& d) override
    {
    	std::unique_lock<std::recursive_mutex> client_lock(client_mutex);
    	if(!connected) {
    		return Smart::SMART_DISCONNECTED;
    	}
    	if(!subscribed) {
    		return Smart::SMART_UNSUBSCRIBED;
    	}

    	if(has_data == true) {
    		d = last_update;
    		return Smart::SMART_OK;
    	} else {
    		return Smart::SMART_NODATA;
    	}

		return Smart::SMART_ERROR;
	}

    /** Blocking call which waits until the next update is received.
     *
     *  Blocking is aborted with the appropriate status if either the
     *  server gets deactivated, the client gets unsubscribed or disconnected
     *  or if blocking is not allowed any more at the client.
     *
     *  @param d is set to the newest currently available data
     *  @param timeout is the timeout time to block the method maximally (default value max blocks infinitely)
     *
     *  @return status code
     *   - SMART_OK                  : everything is ok and just received data is returned.
     *   - SMART_CANCELLED           : blocking is not allowed or is not allowed anymore. Waiting for the
     *                                 next update is aborted and no valid data is returned.
     *   - SMART_UNSUBSCRIBED        : returns immediately without data if the client is not subscribed.
     *   - SMART_DISCONNECTED        : returns immediately without data since client is even not connected
     *                                 to a server.
     *   - SMART_ERROR               : something went completely wrong and no valid data returned.
     */
    virtual Smart::StatusCode getUpdateWait(DataType& d, const Smart::Duration &timeout = Smart::Duration::max()) override
    {
    	std::unique_lock<std::recursive_mutex> client_lock(client_mutex);
    	if(!this->is_blocking) {
    		return Smart::SMART_CANCELLED;
    	}
    	if(!connected) {
    		return Smart::SMART_DISCONNECTED;
    	}
    	if(!subscribed) {
    		return Smart::SMART_UNSUBSCRIBED;
    	}

    	if(timeout != Smart::Duration::max()) {
			auto result = data_cond_var.wait_for(client_lock, timeout);
			if(result == std::cv_status::timeout) {
				return Smart::SMART_TIMEOUT;
			}
    	} else {
    		// blocking wait until new update becomes available or any other related event occurs
    		data_cond_var.wait(client_lock);
    	}
    	if(!this->is_blocking) {
			return Smart::SMART_CANCELLED;
		}

    	// delegate the actual data retrieval to the nonblocking method
		return getUpdate(d);
	}
};

} /* namespace Fake */

#endif /* FAKEPUSHCLIENTPATTERN_H_ */
