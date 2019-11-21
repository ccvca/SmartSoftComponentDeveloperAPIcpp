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

#ifndef FAKESERVERBASE_H_
#define FAKESERVERBASE_H_

#include <set>
#include <vector>
#include <string>
#include <memory>

#include <mutex>
#include <atomic>
#include <future>

#include "FakeClientBase.h"
#include "FakeComponent.h"
#include "FakePatternTypeEnum.h"

namespace Fake {

// forward declarations
class FakeComponent;
class FakeClientBase;

class FakeServerBase : public std::enable_shared_from_this<FakeServerBase>
{
private:
	std::atomic<bool> cancelled;
	std::future<void> middleware_thread;

	FakeComponent* fake_component;

	struct ClientDataEntry {
		FakeClientBase *client;
		std::vector<std::string> data;
	};
	std::list<ClientDataEntry> client_send_buffer;
	std::condition_variable_any send_buffer_cond_var;

	void middleware_thread_runner();

protected:
	// these two helper methods allow registering the derived server types within
	// the FakeNamingService registry so that they can later be used for connection by clients.
	void register_self_as(const FakePatternTypeEnum &type,
			const std::string &service_name,
			const std::vector<std::string> &object_type_names);
	void unregister_self();

	std::recursive_mutex server_mutex;
	std::set<FakeClientBase*> connected_clients;

	// this method helps disconnecting all currently connected clients at once
	// this method should be typically called from the server's serverInitiatedDisconnect handler
	void disconnect_all_clients();

	// use this method to reply data to a specific client asynchronously
	// this method simulates asynchronous middleware communication by
	// executing the actual communication in an extra internal thread
	// this helps simulating a more realistic middleware behavrion which
	// is important for realistic testing
	void send_data_to(FakeClientBase *client, const std::vector<std::string> &data);
public:
	FakeServerBase(FakeComponent* fake_component);
	virtual ~FakeServerBase();

	// this method is called each time a new client wants to connect to this server
	void on_connect(FakeClientBase *client);
	// this method is called each time a client wants to disconnect from this server
	void on_disconnect(FakeClientBase *client);

	// implement this method in derived classes to process incoming data from clients
	virtual void on_input_from(FakeClientBase *client, const std::vector<std::string> &data) = 0;
};

} /* namespace Fake */

#endif /* FAKESERVERBASE_H_ */
