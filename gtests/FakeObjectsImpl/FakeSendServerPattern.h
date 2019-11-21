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

#ifndef FAKESENDSERVERPATTERN_H_
#define FAKESENDSERVERPATTERN_H_

#include <mutex>

#include <smartISendServerPattern_T.h>
#include "FakeComponent.h"
#include "FakeServerBase.h"

namespace Fake {

template <typename DataType>
class SendServerPattern
:	public Smart::ISendServerPattern<DataType>
,	public FakeServerBase
{
public:
	using ISendServerBase = Smart::ISendServerPattern<DataType>;
	using typename ISendServerBase::ISendServerHandlerPtr;

	SendServerPattern(FakeComponent* component, const std::string& serviceName, ISendServerHandlerPtr handler = nullptr)
	:	ISendServerBase(component, serviceName, handler)
	,	FakeServerBase(component)
	{
		FakeServerBase::register_self_as(
			FakePatternTypeEnum::SEND_PATTERN, serviceName,
			std::vector<std::string>(1, DataType::identifier())
		);
	}
	virtual ~SendServerPattern()
	{
		FakeServerBase::unregister_self();
		this->serverInitiatedDisconnect();
	}

private:
	virtual void on_input_from(FakeClientBase *client, const std::vector<std::string> &data) override
	{
		std::unique_lock<std::recursive_mutex> server_lock(FakeServerBase::server_mutex);
		DataType update;
		// here we call an externally specified conversion (i.e. deserialization) method
		::convert(data, update);
		server_lock.unlock();

		ISendServerBase::handleSend(update);
	}

	virtual void serverInitiatedDisconnect() override
	{
		FakeServerBase::disconnect_all_clients();
	}
};

} /* namespace Fake */

#endif /* FAKESENDSERVERPATTERN_H_ */
