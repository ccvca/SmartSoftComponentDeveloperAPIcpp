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

#ifndef SMARTSOFTTESTS_SENDPATTERNTESTS_H_
#define SMARTSOFTTESTS_SENDPATTERNTESTS_H_

#include <mutex>

#include "SmartSoftTests/TestingEnvironmentBase.h"

namespace Smart {

class SimpleSendHandler : public ISendServerHandlerBase {
private:
	mutable std::mutex data_mutex;
	DataType last_update;
public:
	virtual ~SimpleSendHandler() = default;

	virtual void handleSend(const DataType& data) override {
		std::unique_lock<std::mutex> data_lock(data_mutex);
		last_update = data;
	}

	DataType getLastUpdate() const {
		std::unique_lock<std::mutex> data_lock(data_mutex);
		return last_update;
	}
};

class SendPatternTests : public ::testing::Test {
protected:
	Smart::StatusCode status;

	std::string componentName;
	std::string serviceName;

	DataType data;

	ISendClientPtrType sendClient;
	std::shared_ptr<SimpleSendHandler> sendHandler;
	ISendServerPtrType sendServer;


	void SetUp() override {
		status = Smart::StatusCode::SMART_ERROR;

		componentName = TEST_ENVIRONMENT()->getComponentName();
		serviceName = "SendTest";

		data.description = CommTestObjects::CommText { "Hello" };
		data.trajectory.push_back(CommTestObjects::Comm3dPose{1,2,3});

		sendClient = TEST_ENVIRONMENT()->createSendClient();
		sendHandler = std::make_shared<SimpleSendHandler>();
		sendServer = TEST_ENVIRONMENT()->createSendServer(serviceName, sendHandler);
	}

	void TearDown() override {
		sendClient.reset();
		sendHandler.reset();
		sendServer.reset();
	}
};

TEST_F(SendPatternTests, SendConnectionTest) {
	// initially a client is disconnected
	status = sendClient->send(data);
	EXPECT_EQ(status, Smart::StatusCode::SMART_DISCONNECTED);

	status = sendClient->connect(componentName, serviceName);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	status = sendClient->send(data);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	status = sendClient->disconnect();
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	status = sendClient->send(data);
	EXPECT_EQ(status, Smart::StatusCode::SMART_DISCONNECTED);

	// here we connect the client again and delete the server to trigger
	// a Server Initiated Disconnect (SID) which triggers the client to
	// automatically disconnect
	status = sendClient->connect(componentName, serviceName);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	// release shared pointer to delete the push-server instance
	sendServer.reset();

	// sleep for 100 ms to ensure that the client actually gets disconnected
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// the client internal status must be now disconnected (after the SID)
	status = sendClient->send(data);
	EXPECT_EQ(status, Smart::StatusCode::SMART_DISCONNECTED);

	status = sendClient->connect(componentName, serviceName);
	EXPECT_EQ(status, Smart::StatusCode::SMART_SERVICEUNAVAILABLE);
}

TEST_F(SendPatternTests, SendAPITest) {
	status = sendClient->connect(componentName, serviceName);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	status = sendClient->send(data);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	// give the server some time to actually receive the data
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	auto received_data = sendHandler->getLastUpdate();
	ASSERT_EQ(data.trajectory.size(), received_data.trajectory.size());
	EXPECT_EQ(data.trajectory.front().x, received_data.trajectory.front().x);
}

} /* namespace Smart */

#endif /* SMARTSOFTTESTS_SENDPATTERNTESTS_H_ */
