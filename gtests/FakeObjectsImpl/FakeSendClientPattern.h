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

#ifndef FAKESENDCLIENTPATTERN_H_
#define FAKESENDCLIENTPATTERN_H_

#include <mutex>

#include <smartISendClientPattern_T.h>
#include "FakeClientBase.h"
#include "FakeComponent.h"

namespace Fake {

template <typename DataType>
class SendClientPattern
:	public Smart::ISendClientPattern<DataType>
,	public FakeClientBase // this base simulates a basic middleware API with a typical asynchronous communication behavior
{
private:
	std::recursive_mutex client_mutex;
	bool connected;

	virtual void on_sid() override
	{
		this->disconnect();
	}

	virtual void on_update(const std::vector<std::string> &data) override
	{
		// no updates expected for this pattern
	}
public:
	SendClientPattern(FakeComponent* component)
	:	Smart::ISendClientPattern<DataType>(component)
	,	FakeClientBase(component)
	,	connected(false)
	{  }
	SendClientPattern(FakeComponent* component, const std::string& server, const std::string& service)
	:	Smart::ISendClientPattern<DataType>(component, server, service)
	,	FakeClientBase(component)
	,	connected(false)
	{
		connect(server, service);
	}
	virtual ~SendClientPattern() {
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
    		FakePatternTypeEnum::SEND_PATTERN,
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
		return Smart::SMART_OK;
	}

    /** Perform a one-way communication. Appropriate status codes make
     *  sure that the information has been transferred.
     *
     *  @param data the object to be sent (Communication Object)
     *
     *  @return status code:
     *    - SMART_OK                  : everything is ok and communication object sent to server
     *    - SMART_DISCONNECTED        : the client is disconnected and no send can be made
     *    - SMART_ERROR_COMMUNICATION : communication problems, data not transmitted
     *    - SMART_ERROR               : something went wrong, data not transmitted
     */
    virtual Smart::StatusCode send(const DataType& data) override
    {
    	std::unique_lock<std::recursive_mutex> client_lock(client_mutex);
    	if(!connected) {
    		return Smart::SMART_DISCONNECTED;
    	}
    	// send data to server
		if(FakeClientBase::send_data(::serialize(data)) != 0) {
			return Smart::SMART_ERROR_COMMUNICATION;
		}
    	return Smart::SMART_OK;
    }
};

} /* namespace Fake */

#endif /* FAKESENDCLIENTPATTERN_H_ */
