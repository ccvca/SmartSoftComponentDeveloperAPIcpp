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

#ifndef SMARTSOFTTESTS_TESTINGENVIRONMENTBASE_H_
#define SMARTSOFTTESTS_TESTINGENVIRONMENTBASE_H_

#include <memory>
#include <future>

#include <gtest/gtest.h>

// this is a middleware-independent specification of an example communication-object that is used for testing
#include "SmartSoftTests/CommTestObjects/CommTrajectory.h"
using DataType = CommTestObjects::CommTrajectory;

// here come the abstract API includes and Alias definitions for the tests
#include <smartIComponent.h>

using IComponentPtrType = std::shared_ptr<Smart::IComponent>;

#include <smartIPushServerPattern_T.h>
#include <smartIPushClientPattern_T.h>

using IPushServerPtrType = std::shared_ptr<Smart::IPushServerPattern<DataType>>;
using IPushClientPtrType = std::shared_ptr<Smart::IPushClientPattern<DataType>>;

#include <smartISendServerPattern_T.h>
#include <smartISendClientPattern_T.h>

using ISendServerHandlerBase = Smart::ISendServerHandler<DataType>;
using ISendServerHandlerPtrType = std::shared_ptr<ISendServerHandlerBase>;
using ISendServerPtrType = std::shared_ptr<Smart::ISendServerPattern<DataType>>;
using ISendClientPtrType = std::shared_ptr<Smart::ISendClientPattern<DataType>>;

#include <smartIQueryServerPattern_T.h>
#include <smartIQueryClientPattern_T.h>

using RequestType = CommTestObjects::CommTrajectory;
using AnswerType = CommTestObjects::CommTrajectory;
using IQueryServerHandlerBase = Smart::IQueryServerHandler<RequestType,AnswerType>;
using IQueryServerHandlerPtrType = std::shared_ptr<IQueryServerHandlerBase>;
using IQueryServerPtrType = std::shared_ptr<Smart::IQueryServerPattern<RequestType,AnswerType>>;
using IQueryClientPtrType = std::shared_ptr<Smart::IQueryClientPattern<RequestType,AnswerType>>;

#include <smartIEventServerPattern_T.h>
#include <smartIEventClientPattern_T.h>

using ActivationType = CommTestObjects::Comm3dPose;
using EventType = CommTestObjects::CommTrajectory;
using IEventTestHandlerBase = Smart::IEventTestHandler<ActivationType,EventType,EventType>;
using IEventTestHandlerPtrType = std::shared_ptr<IEventTestHandlerBase>;
using IEventServerPtrType = std::shared_ptr<Smart::IEventServerPattern<ActivationType,EventType,EventType>>;
using IEventClientPtrType = std::shared_ptr<Smart::IEventClientPattern<ActivationType,EventType>>;

// Call this macro once in any of your source-files (e.g. in the source-file of your TestingEnvironment implementation)
// to register your middleware-specific implementation of the TestingEnvironmentBase interface.
// For example, if your class is named "MyTestingEnvironment" then the call looks as follows:
//   DEFINE_TESTING_ENVIRONMENT(MyTestingEnvironment);
#define DEFINE_TESTING_ENVIRONMENT(EnvironmentImplType) \
	Smart::TestingEnvironmentBase * ENVIRONMENT_IMPL_PTR = nullptr; \
	Smart::TestingEnvironmentBase * Smart::TEST_ENVIRONMENT() { \
		if(ENVIRONMENT_IMPL_PTR == nullptr) { \
			ENVIRONMENT_IMPL_PTR = new EnvironmentImplType(); \
		} \
		return ENVIRONMENT_IMPL_PTR; \
	} \

namespace Smart {

/** Abstract base class that can be used to implement a middleware-specific testing environment
 *
 *  If you want to test your implementation of the SmartSoft CD API, then subclass this abstract class
 *  and implement all the abstract methods such that they internally map to your specific implementations
 *  of the related interfaces. It is recommended to use std::make_shared for implementing the methods.
 *  You also might want to create one instance of the IComponent class (using your implementation) within the
 *  SetUp() method and use this instance to create all the pattern instances.
 */
class TestingEnvironmentBase : public ::testing::Environment {
protected:
	IComponentPtrType component;
	// use std::async to execute component->run() between SetUp and TearDown
	std::future<void> async_component_runner;
public:
	virtual ~TestingEnvironmentBase() = default;

	inline std::string getComponentName() const {
		return "TestComponent";
	}

	virtual void SetUp() override {
		std::cout << "create new component instance" << std::endl;
		component = createComponent(getComponentName());
		std::cout << "execute component->run() asynchronously..." << std::endl;
		async_component_runner = std::async(std::launch::async, [&]{ component->run(); });
	}

	virtual void TearDown() override {
		std::cout << "signal component to shut-down" << std::endl;
		component->signal_shutdown();
		std::cout << "wait till component runner returns..." << std::endl;
		async_component_runner.wait();
		std::cout << "delete component instance..." << std::endl;
		component.reset();
		std::cout << "TestingEnvironment::TearDown() finished!" << std::endl;
	}

	virtual IComponentPtrType createComponent(const std::string &name) = 0;

	virtual IPushClientPtrType createPushClient() = 0;
	virtual IPushServerPtrType createPushServer(const std::string &name) = 0;

	virtual ISendClientPtrType createSendClient() = 0;
	virtual ISendServerPtrType createSendServer(const std::string &name, ISendServerHandlerPtrType handler) = 0;

	virtual IQueryClientPtrType createQueryClient() = 0;
	virtual IQueryServerPtrType createQueryServer(const std::string &name, IQueryServerHandlerPtrType handler) = 0;

	virtual IEventClientPtrType createEventClient() = 0;
	virtual IEventServerPtrType createEventServer(const std::string &name, IEventTestHandlerPtrType handler) = 0;
};

// This global method can be used whenever you require access to the TestingEnvironment instance.
// The implementation of this method is defined in the macro DEFINE_TESTING_ENVIRONMENT (see above).
extern TestingEnvironmentBase * TEST_ENVIRONMENT();

} /* namespace Smart */

#endif /* SMARTSOFTTESTS_TESTINGENVIRONMENTBASE_H_ */
