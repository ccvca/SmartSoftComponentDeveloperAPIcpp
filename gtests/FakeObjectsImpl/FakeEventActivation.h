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

#ifndef FAKEEVENTACTIVATION_H_
#define FAKEEVENTACTIVATION_H_

#include <smartNumericCorrelationId.h>
#include <smartIEventClientPattern_T.h>
#include "FakeClientBase.h"

namespace Fake {

template <class ActivationType>
class EventActivation {
private:
	FakeClientBase *client;
	bool fired_once;
	Smart::NumericCorrelationId id;
	Smart::EventMode mode;
	ActivationType event_parameters;
public:
	EventActivation(FakeClientBase *client)
	:	client(client)
	,	id(0)
	,	fired_once(false)
	,	mode(Smart::EventMode::single)
	,	event_parameters()
	{  }

	bool operator==(const EventActivation &other) const {
		return id == other.id;
	}

	FakeClientBase *getClientPtr() const {
		return client;
	}

	inline void setEventId(const Smart::NumericCorrelationId &id) {
		this->id = id;
	}
	inline Smart::NumericCorrelationId getEventId() const {
		return id;
	}

	inline void setEventParameters(const ActivationType &event_parameters) {
		this->event_parameters = event_parameters;
	}
	inline ActivationType getEventParameters() const {
		return event_parameters;
	}
	inline ActivationType& getEventParametersRef() {
		return event_parameters;
	}

	inline void setActivationMode(const Smart::EventMode &mode) {
		this->mode = mode;
	}

	inline void fireEvent() { fired_once = true; }
	inline bool hasFiredOnce() const { return fired_once; }

	inline bool isContinuous() const {
		return mode == Smart::EventMode::continuous;
	}
};

} /* namespace Fake */

#endif /* FAKEEVENTACTIVATION_H_ */
