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

#ifndef SMARTSOFTTESTS_EVENTPATTERNTESTS_H_
#define SMARTSOFTTESTS_EVENTPATTERNTESTS_H_

#include "SmartSoftTests/TestingEnvironmentBase.h"

namespace Smart {

class DemoEventTestHandler: public IEventTestHandlerBase {
public:
	virtual ~DemoEventTestHandler() = default;

	virtual bool testEvent(ActivationType& param, EventType& event, const EventType& status) override {
		if(status.trajectory.size() > 0 && status.trajectory.front().x > param.x) {
			event = status;
			return true;
		}
		return false;
	}
	virtual void onActivation(const ActivationType& param) override {
//		std::cout << "onActivation: " << param << std::endl;
	};
};

class EventPatternTests : public ::testing::Test {
protected:
	Smart::StatusCode status;

	std::string componentName;
	std::string serviceName;

	Smart::EventIdPtr id;
	ActivationType activation;
	EventType update;
	EventType event;

	IEventClientPtrType eventClient;
	IEventTestHandlerPtrType handler;
	IEventServerPtrType eventServer;

	void SetUp() override {
		status = Smart::StatusCode::SMART_ERROR;

		componentName = TEST_ENVIRONMENT()->getComponentName();
		serviceName = "EventTest";

		update.description = CommTestObjects::CommText { "Hello" };
		update.trajectory.push_back(CommTestObjects::Comm3dPose{1,2,3});

		eventClient = TEST_ENVIRONMENT()->createEventClient();
		handler = std::make_shared<DemoEventTestHandler>();
		eventServer = TEST_ENVIRONMENT()->createEventServer(serviceName, handler);
	}

	void TearDown() override {
		eventClient.reset();
		eventServer.reset();
	}
};

TEST_F(EventPatternTests, EventConnectionTest) {
	// initially a client is disconnected
	status = eventClient->getEvent(id,event);
	EXPECT_EQ(status, Smart::StatusCode::SMART_DISCONNECTED);

	status = eventClient->connect(componentName, serviceName);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	status = eventClient->getEvent(id,event);
	EXPECT_EQ(status, Smart::StatusCode::SMART_WRONGID);

	status = eventClient->disconnect();
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	status = eventClient->getEvent(id,event);
	EXPECT_EQ(status, Smart::StatusCode::SMART_DISCONNECTED);

	// here we connect the client again and delete the server to trigger
	// a Server Initiated Disconnect (SID) which triggers the client to
	// automatically disconnect
	status = eventClient->connect(componentName, serviceName);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
	// release shared pointer to delete the server instance
	eventServer.reset();

	// sleep for 100 ms to ensure that the client actually gets disconnected
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// the client internal status must be now disconnected (after the SID)
	status = eventClient->getEvent(id,event);
	EXPECT_EQ(status, Smart::StatusCode::SMART_DISCONNECTED);

	status = eventClient->connect(componentName, serviceName);
	EXPECT_EQ(status, Smart::StatusCode::SMART_SERVICEUNAVAILABLE);
}

TEST_F(EventPatternTests, EventAPITest) {
	status = eventClient->connect(componentName, serviceName);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	// no event activated yet, all these calls should return wrong-id
	status = eventClient->tryEvent(id);
	EXPECT_EQ(status, Smart::StatusCode::SMART_WRONGID);
	status = eventClient->getEvent(id,event);
	EXPECT_EQ(status, Smart::StatusCode::SMART_WRONGID);
	status = eventClient->getNextEvent(id,event);
	EXPECT_EQ(status, Smart::StatusCode::SMART_WRONGID);

	status = eventClient->activate(Smart::continuous, activation, id);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	// event has been activated but not yet fired
	status = eventClient->tryEvent(id);
	EXPECT_EQ(status, Smart::StatusCode::SMART_ACTIVE);

	Smart::EventIdPtr id_single;
	status = eventClient->activate(Smart::single, activation, id_single);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	// the single event has also not fired yet
	status = eventClient->tryEvent(id_single);
	EXPECT_EQ(status, Smart::StatusCode::SMART_ACTIVE);

	Smart::EventIdPtr id3;
	ActivationType p3;
	p3.x = 3; // event will fire after the 3rd put for the first time
	status = eventClient->activate(Smart::continuous, p3, id3);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	// put an event update (will trigger the DemoEventTestHandler->testEvent(...) method)
	status = eventServer->put(update);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
//	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	//  we can consume the event (this call will block if the event has not yet been received,
	// or return immediately with the new event othervise)
	status = eventClient->getEvent(id, event);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);
	ASSERT_EQ(event.trajectory.size(), update.trajectory.size());
	EXPECT_EQ(event.trajectory.front().x, update.trajectory.front().x);

	// continuous event has been consumed, it is not "active" for the next event to occur
	status = eventClient->tryEvent(id);
	EXPECT_EQ(status, Smart::StatusCode::SMART_ACTIVE);

	// now we can consume the single event
	status = eventClient->getEvent(id_single, event);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);
	ASSERT_EQ(event.trajectory.size(), update.trajectory.size());
	EXPECT_EQ(event.trajectory.front().x, update.trajectory.front().x);

	// single event has been consumed
	status = eventClient->tryEvent(id_single);
	EXPECT_EQ(status, Smart::StatusCode::SMART_PASSIVE);

	// event 3 should not yet has fired
	status = eventClient->tryEvent(id);
	EXPECT_EQ(status, Smart::StatusCode::SMART_ACTIVE);

	// now we put another three event updates with x values 2, 3 and 4
	for(auto i=2; i<5; ++i) {
		update.trajectory.front().x = i;
		status = eventServer->put(update);
		ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// we now should have a new continuous event update that we consume
	status = eventClient->getEvent(id, event);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);
	// only the latest event-update is visible
	ASSERT_EQ(event.trajectory.size(), 1);
	EXPECT_EQ(event.trajectory.front().x, 4);

	// continuous event has been consumed
	status = eventClient->tryEvent(id);
	EXPECT_EQ(status, Smart::StatusCode::SMART_ACTIVE);

	// we already consumed the only single event, so it remains passive until it gets deactivated
	status = eventClient->tryEvent(id_single);
	EXPECT_EQ(status, Smart::StatusCode::SMART_PASSIVE);
	status = eventClient->getEvent(id_single, event);
	EXPECT_EQ(status, Smart::StatusCode::SMART_PASSIVE);
	status = eventClient->getNextEvent(id_single, event);
	EXPECT_EQ(status, Smart::StatusCode::SMART_PASSIVE);

	// event 3 should now has been fired
	status = eventClient->tryEvent(id3);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);
	if(status == Smart::StatusCode::SMART_OK) {
		// now we can consume the event
		status = eventClient->getEvent(id3, event);
		EXPECT_EQ(status, Smart::StatusCode::SMART_OK);
		// only the latest event-update is visible
		ASSERT_EQ(event.trajectory.size(), 1);
		EXPECT_EQ(event.trajectory.front().x, 4);

		// continuous event has been consumed
		status = eventClient->tryEvent(id3);
		EXPECT_EQ(status, Smart::StatusCode::SMART_ACTIVE);
	}

	// now we deactivate the first two event activations (only leaving event 3 activated)
	status = eventClient->deactivate(id);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);
	status = eventClient->deactivate(id_single);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	// now we put another event update with value 5
	update.trajectory.front().x = 5;
	status = eventServer->put(update);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	status = eventClient->tryEvent(id);
	EXPECT_EQ(status, Smart::StatusCode::SMART_WRONGID);
	status = eventClient->tryEvent(id_single);
	EXPECT_EQ(status, Smart::StatusCode::SMART_WRONGID);

	// event 3 should now has been fired
	status = eventClient->tryEvent(id3);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);
	if(status == Smart::StatusCode::SMART_OK) {
		// now we can consume the event
		status = eventClient->getEvent(id3, event);
		EXPECT_EQ(status, Smart::StatusCode::SMART_OK);
		// only the latest event-update is visible
		ASSERT_EQ(event.trajectory.size(), 1);
		EXPECT_EQ(event.trajectory.front().x, 5);

		// continuous event has been consumed
		status = eventClient->tryEvent(id3);
		EXPECT_EQ(status, Smart::StatusCode::SMART_ACTIVE);
	}
}

//Smart::StatusCode get_event(IEventClientType* eventClient) {
//	DataType data;
//	return eventClient->getUpdateWait(data);
//}
//
//TEST_F(EventPatternTest, EventMultithreadingTest) {
//	status = eventClient->connect(componentName, serviceName);
//	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
//
//	status = eventClient->subscribe();
//	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
//
//	// initiate two getUpdateWait calls
//	auto p1 = std::async(std::launch::async, execute_client, eventClient);
//	auto p2 = std::async(std::launch::async, execute_client, eventClient);
//
//	// make sure the calls are ready to receive data
//	std::this_thread::sleep_for(std::chrono::milliseconds(100));
//
//	// send data to both waiting calls
//	status = eventServer->put(data);
//	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
//
//	p1.wait();
//	EXPECT_EQ(p1.get(), Smart::StatusCode::SMART_OK);
//	p2.wait();
//	EXPECT_EQ(p2.get(), Smart::StatusCode::SMART_OK);
//
//	auto p3 = std::async(std::launch::async, execute_client, eventClient);
//	std::this_thread::sleep_for(std::chrono::milliseconds(100));
//	eventClient->disconnect();
//
//	// wait until the asynchronous query call returns
//	p3.wait();
//	EXPECT_EQ(p3.get(), Smart::StatusCode::SMART_DISCONNECTED);
//}

} /* namespace Smart */

#endif /* SMARTSOFTTESTS_EVENTPATTERNTESTS_H_ */
