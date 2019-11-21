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

#include "FakeServerBase.h"

namespace Fake {

FakeServerBase::FakeServerBase(FakeComponent* fake_component)
:	cancelled(false)
,	fake_component(fake_component)
{
	middleware_thread = std::async(std::launch::async, &FakeServerBase::middleware_thread_runner, this);
}
FakeServerBase::~FakeServerBase()
{
	// this signals the internal thread to stop
	cancelled = true;
	send_buffer_cond_var.notify_all();
}

void FakeServerBase::register_self_as(
		const FakePatternTypeEnum &type,
		const std::string &service_name,
		const std::vector<std::string> &object_type_names)
{
	fake_component->getNamingServicePtr()->register_server(this, type, service_name, object_type_names);
}

void FakeServerBase::FakeServerBase::unregister_self()
{
	fake_component->getNamingServicePtr()->unregister_server(this);
}

void FakeServerBase::middleware_thread_runner()
{
	while(!cancelled) {
		std::unique_lock<std::recursive_mutex> server_lock(server_mutex);
		if(client_send_buffer.empty()) {
			// don't block forever, as this might cause a deadlock
			auto result = send_buffer_cond_var.wait_for(server_lock, std::chrono::milliseconds(500));
			if(result == std::cv_status::timeout) {
				continue;
			}
			if(cancelled) break;
			if(client_send_buffer.empty()) {
				continue;
			}
		}

		// get a copy of the client entry
		auto entry = client_send_buffer.front();
		client_send_buffer.pop_front();

		// we unlock the mutex in order for the client class to prevent deadlocks
		server_lock.unlock();

		entry.client->on_update(entry.data);
	}
}

void FakeServerBase::send_data_to(FakeClientBase *client, const std::vector<std::string> &data)
{
	std::unique_lock<std::recursive_mutex> server_lock(server_mutex);
	client_send_buffer.push_back(ClientDataEntry{client, data});
	send_buffer_cond_var.notify_one();
}

void FakeServerBase::disconnect_all_clients() {
	std::unique_lock<std::recursive_mutex> server_lock(server_mutex);

	// release the current client buffer as we are going to disconnect all clients anyways
	client_send_buffer.clear();
	send_buffer_cond_var.notify_one();

	// we make a local copy of the clients list as the original
	// list will be modified by clients in the process...
	auto clients_copy = connected_clients;

	server_lock.unlock();

	for(auto& client: clients_copy) {
		client->on_sid();
	}
}

void FakeServerBase::on_connect(FakeClientBase *client) {
	std::unique_lock<std::recursive_mutex> server_lock(server_mutex);
	connected_clients.insert(client);
}
void FakeServerBase::on_disconnect(FakeClientBase *client) {
	std::unique_lock<std::recursive_mutex> server_lock(server_mutex);
	connected_clients.erase(client);
	client_send_buffer.remove_if(
		[client](const ClientDataEntry &entry) {
			return entry.client == client;
		}
	);
}

} /* namespace Fake */
