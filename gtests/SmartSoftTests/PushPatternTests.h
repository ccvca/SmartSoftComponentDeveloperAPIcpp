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

#ifndef SMARTSOFTTESTS_PUSHPATTERNTESTS_H_
#define SMARTSOFTTESTS_PUSHPATTERNTESTS_H_

#include "SmartSoftTests/TestingEnvironmentBase.h"

namespace Smart {

class PushPatternTests : public ::testing::Test {
protected:
	Smart::StatusCode status;

	std::string componentName;
	std::string serviceName;

	DataType data;

	IPushClientPtrType pushClient;
	IPushServerPtrType pushServer;

	void SetUp() override {
		status = Smart::StatusCode::SMART_ERROR;

		componentName = TEST_ENVIRONMENT()->getComponentName();
		serviceName = "PushTest";

		data.description = CommTestObjects::CommText { "Hello" };
		data.trajectory.push_back(CommTestObjects::Comm3dPose{1,2,3});

		pushClient = TEST_ENVIRONMENT()->createPushClient();
		pushServer = TEST_ENVIRONMENT()->createPushServer(serviceName);
	}

	void TearDown() override {
		pushClient.reset();
		pushServer.reset();
	}
};

TEST_F(PushPatternTests, ConnectionTest) {
	// initially a client is disconnected
	status = pushClient->getUpdate(data);
	EXPECT_EQ(status, Smart::StatusCode::SMART_DISCONNECTED);

	status = pushClient->connect(componentName, serviceName);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	status = pushClient->getUpdate(data);
	EXPECT_EQ(status, Smart::StatusCode::SMART_UNSUBSCRIBED);

	status = pushClient->subscribe();
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	status = pushClient->getUpdate(data);
	EXPECT_EQ(status, Smart::StatusCode::SMART_NODATA);

	status = pushClient->unsubscribe();
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	status = pushClient->getUpdate(data);
	EXPECT_EQ(status, Smart::StatusCode::SMART_UNSUBSCRIBED);

	status = pushClient->disconnect();
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	status = pushClient->getUpdate(data);
	EXPECT_EQ(status, Smart::StatusCode::SMART_DISCONNECTED);

	// here we connect the client again and delete the server to trigger
	// a Server Initiated Disconnect (SID) which triggers the client to
	// automatically disconnect
	status = pushClient->connect(componentName, serviceName);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
	status = pushClient->subscribe();
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	// release shared pointer to delete the push-server instance
	pushServer.reset();

	// sleep for 100 ms to ensure that the client actually gets disconnected
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// the client internal status must be now disconnected (after the SID)
	status = pushClient->getUpdate(data);
	EXPECT_EQ(status, Smart::StatusCode::SMART_DISCONNECTED);

	status = pushClient->connect(componentName, serviceName);
	EXPECT_EQ(status, Smart::StatusCode::SMART_SERVICEUNAVAILABLE);
}

TEST_F(PushPatternTests, APITest) {
	status = pushClient->connect(componentName, serviceName);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	status = pushClient->subscribe();
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	DataType new_data;
	status = pushClient->getUpdate(new_data);
	EXPECT_EQ(status, Smart::StatusCode::SMART_NODATA);

	status = pushServer->put(data);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	status = pushClient->getUpdate(new_data);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);
	EXPECT_EQ(data.description.text, new_data.description.text);
	ASSERT_EQ(data.trajectory.size(), new_data.trajectory.size());
	EXPECT_EQ(data.trajectory.front().x, new_data.trajectory.front().x);

	status = pushClient->unsubscribe();
	status = pushClient->subscribe(2);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	status = pushClient->getUpdate(new_data);
	EXPECT_EQ(status, Smart::StatusCode::SMART_NODATA);

	data.trajectory.front().x = 1;
	status = pushServer->put(data);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	// sleep for 100 ms to ensure that the client actually receives the data
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// we should get the first element right away
	status = pushClient->getUpdate(new_data);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);
	ASSERT_EQ(data.trajectory.size(), new_data.trajectory.size());
	EXPECT_EQ(data.trajectory.front().x, new_data.trajectory.front().x);

	data.trajectory.front().x = 2;
	status = pushServer->put(data);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	// sleep for 100 ms to ensure that the client actually receives the data
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// now we should get the first update, which should have the x value of 2
	status = pushClient->getUpdate(new_data);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);
	ASSERT_EQ(new_data.trajectory.size(), 1);
	EXPECT_EQ(new_data.trajectory.front().x, 1);

	data.trajectory.front().x = 3;
	status = pushServer->put(data);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	// sleep for 100 ms to ensure that the client actually receives the data
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// we now should have the value 3
	status = pushClient->getUpdate(new_data);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
	ASSERT_EQ(new_data.trajectory.size(), 1);
	EXPECT_EQ(new_data.trajectory.front().x, 3);

	data.trajectory.front().x = 4;
	status = pushServer->put(data);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	// sleep for 100 ms to ensure that the client actually receives the data
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// now we get the next update
	status = pushClient->getUpdate(new_data);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
	ASSERT_EQ(new_data.trajectory.size(), 1);
	EXPECT_EQ(new_data.trajectory.front().x, 3);
}

Smart::StatusCode execute_client(IPushClientPtrType pushClient) {
	DataType data;
	auto timeout = std::chrono::seconds(1);
	return pushClient->getUpdateWait(data, timeout);
}

TEST_F(PushPatternTests, MultithreadingTest) {
	status = pushClient->connect(componentName, serviceName);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	status = pushClient->subscribe();
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	// initiate two getUpdateWait calls
	auto p1 = std::async(std::launch::async, execute_client, pushClient);
	auto p2 = std::async(std::launch::async, execute_client, pushClient);

	// make sure the calls are ready to receive data
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// send data to both waiting calls
	status = pushServer->put(data);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	p1.wait();
	EXPECT_EQ(p1.get(), Smart::StatusCode::SMART_OK);
	p2.wait();
	EXPECT_EQ(p2.get(), Smart::StatusCode::SMART_OK);

	auto p3 = std::async(std::launch::async, execute_client, pushClient);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	pushClient->disconnect();

	// wait until the asynchronous query call returns
	p3.wait();
	EXPECT_EQ(p3.get(), Smart::StatusCode::SMART_DISCONNECTED);
}

TEST_F(PushPatternTests, ScalabilityTest) {
	auto serviceName2 = "PushTest2";
	auto pushClient2 = TEST_ENVIRONMENT()->createPushClient();
	auto pushClient3 = TEST_ENVIRONMENT()->createPushClient();
	auto pushServer2 = TEST_ENVIRONMENT()->createPushServer(serviceName2);

	// connect both clients
	status = pushClient->connect(componentName, serviceName);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
	status = pushClient2->connect(componentName, serviceName2);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
	status = pushClient3->connect(componentName, serviceName2);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	// subscribe both clients
	status = pushClient->subscribe();
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
	status = pushClient2->subscribe(2);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
	status = pushClient3->subscribe();
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	DataType data1, data2, data3;
	data1.trajectory.push_back(CommTestObjects::Comm3dPose{1,1,1});
	data2.trajectory.push_back(CommTestObjects::Comm3dPose{2,2,2});
	data3.trajectory.push_back(CommTestObjects::Comm3dPose{3,3,3});

	status = pushServer->put(data1);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	status = pushServer2->put(data2);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	status = pushServer2->put(data3);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	DataType new_data1, new_data2, new_data3;
	status = pushClient->getUpdate(new_data1);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
	ASSERT_EQ(data1.trajectory.size(), new_data1.trajectory.size());
	EXPECT_EQ(data1.trajectory.front().x, new_data1.trajectory.front().x);

	status = pushClient2->getUpdate(new_data2);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
	ASSERT_EQ(data2.trajectory.size(), new_data2.trajectory.size());
	EXPECT_EQ(data2.trajectory.front().x, new_data2.trajectory.front().x);

	status = pushClient3->getUpdate(new_data3);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
	ASSERT_EQ(data3.trajectory.size(), new_data3.trajectory.size());
	EXPECT_EQ(data3.trajectory.front().x, new_data3.trajectory.front().x);
}

} /* namespace Smart */

#endif /* SMARTSOFTTESTS_PUSHPATTERNTESTS_H_ */
