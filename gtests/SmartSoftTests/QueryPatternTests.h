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

#ifndef SMARTSOFTTESTS_QUERYPATTERNTESTS_H_
#define SMARTSOFTTESTS_QUERYPATTERNTESTS_H_

#include <atomic>

#include "SmartSoftTests/TestingEnvironmentBase.h"

namespace Smart {

class EchoQueryHandler: public IQueryServerHandlerBase {
private:
	std::atomic<Smart::StatusCode> last_answer_status;
	Smart::Duration work_time;
public:
	EchoQueryHandler()
	:	work_time(std::chrono::milliseconds(100))
	,	last_answer_status(Smart::StatusCode::SMART_ERROR)
	{  }
	virtual ~EchoQueryHandler() = default;

	void setWorkloadTime(const Smart::Duration &time) {
		work_time = time;
	}

	virtual void handleQuery(IQueryServer &server, const Smart::QueryIdPtr &id, const RequestType& request) override
	{
		// simulate some work-load by simply sleeping for a specified amount of time
		std::this_thread::sleep_for(work_time);
		// now we just echo the request as an answer
		auto answer = request;
		last_answer_status = server.answer(id, answer);
	}

	Smart::StatusCode getLastAnswerStatus() const {
		return last_answer_status;
	}
};

class QueryPatternTests : public ::testing::Test {
protected:
	Smart::StatusCode status;

	std::string componentName;
	std::string serviceName;

	Smart::QueryIdPtr id;
	RequestType request;
	AnswerType answer;

	IQueryClientPtrType queryClient;
	IQueryServerPtrType queryServer;
	std::shared_ptr<EchoQueryHandler> queryHandler;

	void SetUp() override {
		status = Smart::StatusCode::SMART_ERROR;

		componentName = TEST_ENVIRONMENT()->getComponentName();
		serviceName = "TestQuery";

		request.description = CommTestObjects::CommText { "Hello" };
		request.trajectory.push_back(CommTestObjects::Comm3dPose{1,2,3});

		queryClient = TEST_ENVIRONMENT()->createQueryClient();
		queryHandler = std::make_shared<EchoQueryHandler>();
		queryServer = TEST_ENVIRONMENT()->createQueryServer(serviceName, queryHandler);
	}

	void TearDown() override {
		id.reset();
		queryClient.reset();
		queryServer.reset();
		queryHandler.reset();
	}
};

TEST_F(QueryPatternTests, ConnectionTest) {
	// deactivate blocking wait as we are not interested in delays in this test
	queryHandler->setWorkloadTime(Smart::Duration::zero());

	// initially a client is disconnected
	status = queryClient->query(request, answer);
	EXPECT_EQ(status, Smart::StatusCode::SMART_DISCONNECTED);

	status = queryClient->connect(componentName, serviceName);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	// now the query should succeed
	status = queryClient->query(request, answer);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	// reconnecting a client is allowed (it will internally call discinnect automatically)
	status = queryClient->connect(componentName, serviceName);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	// now the query should succeed
	status = queryClient->query(request, answer);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	status = queryClient->disconnect();
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	status = queryClient->query(request, answer);
	EXPECT_EQ(status, Smart::StatusCode::SMART_DISCONNECTED);

	// here we connect the client again and delete the server to trigger
	// a Server Initiated Disconnect (SID) which triggers the client to
	// automatically disconnect
	status = queryClient->connect(componentName, serviceName);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	// release shared pointer to delete the query-server instance
	queryServer.reset();

	// sleep for 100 ms to ensure that the client actually gets disconnected
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// the client internal status must be now disconnected (after the SID)
	status = queryClient->query(request, answer);
	EXPECT_EQ(status, Smart::StatusCode::SMART_DISCONNECTED);

	status = queryClient->connect(componentName, serviceName);
	EXPECT_EQ(status, Smart::StatusCode::SMART_SERVICEUNAVAILABLE);
}

TEST_F(QueryPatternTests, ClientAPITest) {
	status = queryClient->connect(componentName, serviceName);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	queryHandler->setWorkloadTime(std::chrono::milliseconds(500));

	// this is the synchronous (blocking) query call
	status = queryClient->query(request, answer);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	Smart::QueryIdPtr id1, id2, id3;
	RequestType request1, request2, request3;
	request1.trajectory.push_back(CommTestObjects::Comm3dPose{1,1,1});
	request2.trajectory.push_back(CommTestObjects::Comm3dPose{2,2,2});
	request3.trajectory.push_back(CommTestObjects::Comm3dPose{3,3,3});

	status = queryClient->queryRequest(request1, id1);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	status = queryClient->queryRequest(request2, id2);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	status = queryClient->queryRequest(request3, id3);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	// all IDs must be different
	ASSERT_NE(id1, id2);
	ASSERT_NE(id2, id3);
	ASSERT_NE(id3, id1);

	// the server should not yet have replied
	status = queryClient->queryReceive(id1, answer);
	EXPECT_EQ(status, Smart::StatusCode::SMART_NODATA);

	// now we get the second answer first, before the first one
	status = queryClient->queryReceiveWait(id2, answer);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
	ASSERT_EQ(request2.trajectory.size(), answer.trajectory.size());
	EXPECT_EQ(request2.trajectory.front().x, answer.trajectory.front().x);

	status = queryClient->queryReceive(id1, answer);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
	ASSERT_EQ(request1.trajectory.size(), answer.trajectory.size());
	EXPECT_EQ(request1.trajectory.front().x, answer.trajectory.front().x);

	// the answer has been consumed and is not available anymore
	status = queryClient->queryReceive(id1, answer);
	EXPECT_EQ(status, Smart::StatusCode::SMART_WRONGID);

	// now receive the last answer, the order of or receiving answer is
	// entirely arbitrary and all combinations must be possible
	status = queryClient->queryReceiveWait(id3, answer);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
	ASSERT_EQ(request3.trajectory.size(), answer.trajectory.size());
	EXPECT_EQ(request3.trajectory.front().x, answer.trajectory.front().x);

	queryHandler->setWorkloadTime(Smart::Duration::zero());

	request1.trajectory.front().x = 4;
	request2.trajectory.front().x = 5;

	// now we initiate another two asynchronous requests while performing a synchronous query in between
	status = queryClient->queryRequest(request1, id1);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	// execute a synchronous query in between the two asynchronous request calls
	status = queryClient->query(request, answer);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	status = queryClient->queryRequest(request2, id2);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	// we discard the second request
	status = queryClient->queryDiscard(id2);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	// trying to receive a discarded request results in wrong ID
	status = queryClient->queryReceiveWait(id2, answer);
	EXPECT_EQ(status, Smart::StatusCode::SMART_WRONGID);

	// now we consume the answer for the first request
	status = queryClient->queryReceiveWait(id1, answer);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
	ASSERT_EQ(request1.trajectory.size(), answer.trajectory.size());
	EXPECT_EQ(request1.trajectory.front().x, answer.trajectory.front().x);

	// trying to discard an already consumed request results in wrong ID
	status = queryClient->queryDiscard(id1);
	EXPECT_EQ(status, Smart::StatusCode::SMART_WRONGID);
}

TEST_F(QueryPatternTests, ServerAPITest) {
	status = queryClient->connect(componentName, serviceName);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	Smart::QueryIdPtr invalid_id;
	status = queryServer->answer(invalid_id, answer);
	EXPECT_EQ(status, Smart::StatusCode::SMART_WRONGID);

	queryHandler->setWorkloadTime(Smart::Duration::zero());

	status = queryClient->queryRequest(request, id);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	// give the server a few milliseconds to internally process the request
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	EXPECT_EQ(queryHandler->getLastAnswerStatus(), Smart::StatusCode::SMART_OK);

	// trying to call answer multiple times with the same ID results in SMART_WRONGID
	status = queryServer->answer(id, answer);
	EXPECT_EQ(status, Smart::StatusCode::SMART_WRONGID);

	// also the client should now receive the answer
	status = queryClient->queryReceiveWait(id, answer);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	// here we create and connect a second QueryClient
	auto queryClient2 = TEST_ENVIRONMENT()->createQueryClient();
	status = queryClient2->connect(componentName, serviceName);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	Smart::QueryIdPtr id1, id2, id3;
	RequestType request1, request2, request3;
	request1.trajectory.push_back(CommTestObjects::Comm3dPose{1,1,1});
	request2.trajectory.push_back(CommTestObjects::Comm3dPose{2,2,2});
	request3.trajectory.push_back(CommTestObjects::Comm3dPose{3,3,3});

	// now we initiate two query requests, each from a different client
	status = queryClient->queryRequest(request1, id1);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	status = queryClient2->queryRequest(request2, id2);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	status = queryClient2->queryRequest(request3, id3);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	// only IDs from the same client must be different
	// IDs from different clients must not necessarily be related
	ASSERT_NE(id2, id3);

	// we receive the answers in reverse order (any order should be possible)
	status = queryClient2->queryReceiveWait(id2, answer);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);
	ASSERT_EQ(request2.trajectory.size(), answer.trajectory.size());
	EXPECT_EQ(request2.trajectory.front().x, answer.trajectory.front().x);

	status = queryClient->queryReceiveWait(id1, answer);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);
	ASSERT_EQ(request1.trajectory.size(), answer.trajectory.size());
	EXPECT_EQ(request1.trajectory.front().x, answer.trajectory.front().x);

	status = queryClient2->queryReceiveWait(id3, answer);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);
	ASSERT_EQ(request3.trajectory.size(), answer.trajectory.size());
	EXPECT_EQ(request3.trajectory.front().x, answer.trajectory.front().x);
}

Smart::StatusCode execute_query(IQueryClientPtrType queryClient) {
	RequestType request;
	AnswerType answer;
	return queryClient->query(request, answer);
}

TEST_F(QueryPatternTests, MultithreadingTest) {
	status = queryClient->connect(componentName, serviceName);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	queryHandler->setWorkloadTime(std::chrono::milliseconds(200));

	// initiate two requests from two different threads
	auto q1 = std::async(std::launch::async, execute_query, queryClient);
	auto q2 = std::async(std::launch::async, execute_query, queryClient);

	q1.wait();
	EXPECT_EQ(q1.get(), Smart::StatusCode::SMART_OK);

	q2.wait();
	EXPECT_EQ(q2.get(), Smart::StatusCode::SMART_OK);

	auto q3 = std::async(std::launch::async, execute_query, queryClient);
	// we sleep for a few milliseconds to make sure that the query has been fully initialized
	// before we call disconnect in parallel
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	queryClient->disconnect();

	// wait until the asynchronous query call returns
	q3.wait();

	EXPECT_EQ(q3.get(), Smart::StatusCode::SMART_DISCONNECTED);
	// make sure the handler has finished before exiting
	std::this_thread::sleep_for(std::chrono::seconds(1));
}

TEST_F(QueryPatternTests, ScalabilityTest) {
	auto serviceName2 = "TestQuery2";
	auto queryClient2 = TEST_ENVIRONMENT()->createQueryClient();
	auto queryHandler2 = std::make_shared<EchoQueryHandler>();
	auto queryServer2 = TEST_ENVIRONMENT()->createQueryServer(serviceName2, queryHandler2);

	status = queryClient->connect(componentName, serviceName);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	status = queryClient2->connect(componentName, serviceName2);
	ASSERT_EQ(status, Smart::StatusCode::SMART_OK);

	Smart::QueryIdPtr id1, id2;
	RequestType request1, request2;
	request1.trajectory.push_back(CommTestObjects::Comm3dPose{1,1,1});
	request2.trajectory.push_back(CommTestObjects::Comm3dPose{2,2,2});

	// initiate the first request for the first client/server combination asynchronously
	status = queryClient->queryRequest(request1, id);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);

	// the second request for the second client/server combination is initiated in parallel to the first one
	status = queryClient2->query(request2, answer);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);
	ASSERT_EQ(request2.trajectory.size(), answer.trajectory.size());
	EXPECT_EQ(request2.trajectory.front().x, answer.trajectory.front().x);

	// now receive the answer for the first request
	status = queryClient->queryReceiveWait(id, answer);
	EXPECT_EQ(status, Smart::StatusCode::SMART_OK);
	ASSERT_EQ(request1.trajectory.size(), answer.trajectory.size());
	EXPECT_EQ(request1.trajectory.front().x, answer.trajectory.front().x);
}

} /* namespace Smart */

#endif /* SMARTSOFTTESTS_QUERYPATTERNTESTS_H_ */
