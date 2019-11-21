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

#include "FakeTestingEnvironment.h"

DEFINE_TESTING_ENVIRONMENT(Fake::TestingEnvironment);

// these are the "middleware-specific" communication-object serialization implementations
// these includes must be placed before the pattern includes (see below)
#include "CommTestObjectsFake/CommTrajectoryFake.h"

// these are the includes of the Fake pattern implementations
#include "FakeComponent.h"
#include "FakeEventClientPattern.h"
#include "FakeEventServerPattern.h"
#include "FakePushClientPattern.h"
#include "FakePushServerPattern.h"
#include "FakeQueryClientPattern.h"
#include "FakeQueryServerPattern.h"
#include "FakeSendClientPattern.h"
#include "FakeSendServerPattern.h"

namespace Fake {

IComponentPtrType TestingEnvironment::createComponent(const std::string &name) {
	return std::make_shared<FakeComponent>(name);
}

IPushClientPtrType TestingEnvironment::createPushClient() {
	auto fake_component = std::dynamic_pointer_cast<FakeComponent>(component);
	return std::make_shared<Fake::PushClientPattern<DataType>>(fake_component.get());
}
IPushServerPtrType TestingEnvironment::createPushServer(const std::string &name) {
	auto fake_component = std::dynamic_pointer_cast<FakeComponent>(component);
	return std::make_shared<Fake::PushServerPattern<DataType>>(fake_component.get(), name);
}

ISendClientPtrType TestingEnvironment::createSendClient() {
	auto fake_component = std::dynamic_pointer_cast<FakeComponent>(component);
	return std::make_shared<Fake::SendClientPattern<DataType>>(fake_component.get());
}
ISendServerPtrType TestingEnvironment::createSendServer(const std::string &name, ISendServerHandlerPtrType handler) {
	auto fake_component = std::dynamic_pointer_cast<FakeComponent>(component);
	return std::make_shared<Fake::SendServerPattern<DataType>>(fake_component.get(), name, handler);
}

IQueryClientPtrType TestingEnvironment::createQueryClient() {
	auto fake_component = std::dynamic_pointer_cast<FakeComponent>(component);
	return std::make_shared<Fake::QueryClientPattern<RequestType,AnswerType>>(fake_component.get());
}
IQueryServerPtrType TestingEnvironment::createQueryServer(const std::string &name, IQueryServerHandlerPtrType handler) {
	auto fake_component = std::dynamic_pointer_cast<FakeComponent>(component);
	return std::make_shared<Fake::QueryServerPattern<RequestType,AnswerType>>(fake_component.get(), name, handler);
}

IEventClientPtrType TestingEnvironment::createEventClient() {
	auto fake_component = std::dynamic_pointer_cast<FakeComponent>(component);
	return std::make_shared<Fake::EventClientPattern<ActivationType,EventType>>(fake_component.get());
}
IEventServerPtrType TestingEnvironment::createEventServer(const std::string &name, IEventTestHandlerPtrType handler) {
	auto fake_component = std::dynamic_pointer_cast<FakeComponent>(component);
	return std::make_shared<Fake::EventServerPattern<ActivationType,EventType>>(fake_component.get(), name, handler);
}

} /* namespace Fake */
