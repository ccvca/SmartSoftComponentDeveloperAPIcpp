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

#ifndef FAKEPUSHSERVERPATTERN_H_
#define FAKEPUSHSERVERPATTERN_H_

#include <map>
#include <mutex>

#include <smartIPushServerPattern_T.h>
#include "FakeServerBase.h"

namespace Fake {

template <typename DataType>
class PushServerPattern
:	public Smart::IPushServerPattern<DataType>
,	public FakeServerBase
{
private:
	struct SubscriptionData {
		unsigned long long update_counter {0};
		unsigned int prescale_factor {0};
	};
	std::map<FakeClientBase*,SubscriptionData> subscriptions;

	virtual void on_input_from(FakeClientBase *client, const std::vector<std::string> &data) override
	{
		std::unique_lock<std::recursive_mutex> server_lock(FakeServerBase::server_mutex);
		if(data.empty()) {
			// this means unsubscribe
			subscriptions.erase(client);
		} else {
			SubscriptionData entry;
			entry.prescale_factor = std::stoi(data[0]);
			entry.update_counter = 0;
			subscriptions[client] = entry;
		}
	}

	virtual void serverInitiatedDisconnect() override
	{
		FakeServerBase::disconnect_all_clients();
	}
public:
    /** Default Constructor.
     *
     *  The default constructor should initialize all internal
     *  server resources such that the put() method can be used.
     *
     * @param component   management class of the component
     * @param serviceName name of the service
     *
     */
	PushServerPattern(FakeComponent* component, const std::string& serviceName)
	:	Smart::IPushServerPattern<DataType>(component, serviceName)
	,	FakeServerBase(component)
	{
		FakeServerBase::register_self_as(
			FakePatternTypeEnum::PUSH_PATTERN, serviceName,
			std::vector<std::string>(1, DataType::identifier())
		);
	}
	virtual ~PushServerPattern() {
		FakeServerBase::unregister_self();
		this->serverInitiatedDisconnect();
	}

    /** Provide new data which is sent to all subscribed clients
     *  taking into account their individual prescale factors.
     *  Prescale factors are always whole-numbered multiples of the server
     *  update intervals.
     *
     *  (Individual update interval counters are incremented each
     *   time this member function is called irrespectively of the
     *   elapsed time. One should use the time triggered handler to
     *   call the put() member function with the appropriate timing.)
     *
     *  @param d contains the newly acquired data to be sent as
     *           update.
     *
     *  @return status code
     *    - SMART_OK                  : everything is ok
     *    - SMART_ERROR_COMMUNICATION : communication problems caused by at least
     *                                  one client. The other clients are updated
     *                                  correctly.
     *    - SMART_ERROR               : something went completely wrong with at least one
     *                                  client. Some clients might still been
     *                                  updated correctly.
     */
	virtual Smart::StatusCode put(const DataType& d) override {
		std::unique_lock<std::recursive_mutex> server_lock(FakeServerBase::server_mutex);
		// here we use an externally specified serialization method
		auto serialized_data = ::serialize(d);
		for(auto& sub: subscriptions) {
			if(sub.second.update_counter++ % sub.second.prescale_factor == 0) {
				FakeServerBase::send_data_to(sub.first, serialized_data);
			}
		}
		return Smart::StatusCode::SMART_OK;
	}
};

} /* namespace Fake */

#endif /* FAKEPUSHSERVERPATTERN_H_ */
