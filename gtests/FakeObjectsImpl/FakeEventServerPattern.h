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

#ifndef FAKEEVENTSERVERPATTERN_H_
#define FAKEEVENTSERVERPATTERN_H_

#include <list>
#include <mutex>

#include <smartNumericCorrelationId.h>
#include <smartIEventServerPattern_T.h>
#include "FakeComponent.h"
#include "FakeEventActivation.h"
#include "FakeServerBase.h"

namespace Fake {

template<class ActivationType, class EventType, class UpdateType = EventType>
class EventServerPattern
:	public Smart::IEventServerPattern<ActivationType,EventType,UpdateType>
,	public FakeServerBase
{
public:
	// creating typed aliases, see: https://isocpp.org/wiki/faq/templates#nondependent-name-lookup-types
	using IEventServerBase = Smart::IEventServerPattern<ActivationType,EventType,UpdateType>;
	using typename IEventServerBase::IEventTestHandlerPtr;

	EventServerPattern(FakeComponent* component, const std::string& serviceName, IEventTestHandlerPtr testHandler)
	:	IEventServerBase(component, serviceName, testHandler)
	,	FakeServerBase(component)
	{
		FakeServerBase::register_self_as(
				FakePatternTypeEnum::EVENT_PATTERN, serviceName,
				{ ActivationType::identifier(),	EventType::identifier() }
		);
	}
	virtual ~EventServerPattern() {
		FakeServerBase::unregister_self();
		this->serverInitiatedDisconnect();
	}

	/** Initiate testing the event conditions for the activations.
	 *
	 *  @param state contains the current information checked in testEvent()
	 *         against the individual activation parameters.
	 *
	 *  @return status code
	 *   - SMART_OK                  : everything is ok
	 *   - SMART_ERROR_COMMUNICATION : communication problems
	 *   - SMART_ERROR               : something went wrong
	 *
	 */
	virtual Smart::StatusCode put(const UpdateType& state) override
	{
		std::unique_lock<std::recursive_mutex> server_lock(
				FakeServerBase::server_mutex);

		// we access the elements by reference (so we can modify them in the loop)
		for (auto & event_activation : event_activations) {
			if (event_activation.isContinuous()
					|| !event_activation.hasFiredOnce()) {
				EventType event;
				if (IEventServerBase::testEvent(event_activation.getEventParametersRef(),
						event, state)) {
					event_activation.fireEvent();
					std::vector<std::string> serialized_data = ::serialize(
							event);
					serialized_data.push_back(event_activation.getEventId().to_string());

					FakeServerBase::send_data_to(
							event_activation.getClientPtr(), serialized_data);
				}
			}
		}

		return Smart::StatusCode::SMART_OK;
	}

private:
	std::list<EventActivation<ActivationType>> event_activations;

	virtual void on_input_from(FakeClientBase *client,
			const std::vector<std::string> &data) override
			{
		std::unique_lock<std::recursive_mutex> server_lock(
				FakeServerBase::server_mutex);
		if (data.size() == 1) {
			// this means event deactivation for a certain ID
			Smart::NumericCorrelationId id = std::stoi(data[0]);
			event_activations.remove_if(
					[&](const EventActivation<ActivationType> &activation) {
						return activation.getClientPtr() == client && activation.getEventId() == id;
					});
		} else if (data.size() > 1) {
			EventActivation<ActivationType> activation(client);
			::convert(data, activation.getEventParametersRef());
			activation.setEventId(std::stoi(data[data.size() - 2]));
			auto mode = static_cast<Smart::EventMode>(std::stoi(data.back()));
			activation.setActivationMode(mode);

			event_activations.push_back(activation);

			IEventServerBase::onActivation(activation.getEventParametersRef());
		}
	}

	virtual void serverInitiatedDisconnect() override
	{
		FakeServerBase::disconnect_all_clients();
	}
};

} /* namespace Fake */

#endif /* FAKEEVENTSERVERPATTERN_H_ */
