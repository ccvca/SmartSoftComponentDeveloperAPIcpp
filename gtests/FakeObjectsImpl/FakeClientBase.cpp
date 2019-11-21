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

#include "FakeClientBase.h"

#include <iostream>

namespace Fake {

FakeClientBase::FakeClientBase(FakeComponent *component)
:	fake_component(component)
,	cancelled(false)
{
	middleware_thread = std::async(std::launch::async, &FakeClientBase::middleware_thread_runner, this);
}

FakeClientBase::~FakeClientBase()
{
	// this signals the internal thread to stop
	cancelled = true;
	send_buffer_cond_var.notify_all();
}

int FakeClientBase::middleware_connect(
		const FakePatternTypeEnum &type,
		const std::string &service_name,
		const std::vector<std::string> &object_type_names)
{
	std::unique_lock<std::mutex> middleware_lock(middleware_mutex);
	auto server_ptr = fake_component->getNamingServicePtr()->find_server(type, service_name, object_type_names);
	if(server_ptr) {
		server_ptr->on_connect(this);
		// store the weak pointer
		server_weak_ptr = server_ptr;
		return 1;
	} else if(fake_component->getNamingServicePtr()->has_similar_servers(service_name)) {
		return 0;
	}
	return -1;
}
int FakeClientBase::middleware_disconnect()
{
	std::unique_lock<std::mutex> middleware_lock(middleware_mutex);
	if(auto server_ptr = server_weak_ptr.lock()) {
		send_buffer.clear();
		send_buffer_cond_var.notify_one();
		server_ptr->on_disconnect(this);
		return 0;
	}
	return -1;
}

void FakeClientBase::middleware_thread_runner()
{
	while(!cancelled) {
		std::unique_lock<std::mutex> middleware_lock(middleware_mutex);
		if(send_buffer.empty()) {
			sync_ack_cond_var.notify_one();
			// don't block forever, as this might cause a deadlock
			auto result = send_buffer_cond_var.wait_for(middleware_lock, std::chrono::milliseconds(500));
			if(result == std::cv_status::timeout) {
				continue;
			}
			if(cancelled) break;
			if(send_buffer.empty()) {
				continue;
			}
		}

		// get a data copy
		auto data = send_buffer.front();
		send_buffer.pop_front();

		// we unlock the mutex in order for the client class to prevent deadlocks
		middleware_lock.unlock();

		if(auto server_ptr = server_weak_ptr.lock()) {
			server_ptr->on_input_from(this, data);
		}
	}
}

int FakeClientBase::send_data(const std::vector<std::string> &data, const bool &wait_for_ack)
{
	std::unique_lock<std::mutex> middleware_lock(middleware_mutex);
	if(auto server_ptr = server_weak_ptr.lock()) {
		send_buffer.push_back(data);
		send_buffer_cond_var.notify_one();
		while(wait_for_ack == true && !send_buffer.empty()) {
			sync_ack_cond_var.wait(middleware_lock);
		}
		return 0;
	}
	return -1;
}

} /* namespace Fake */
