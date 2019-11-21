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

#ifndef FAKECLIENTBASE_H_
#define FAKECLIENTBASE_H_

#include <vector>
#include <string>
#include <memory>

#include <list>
#include <mutex>
#include <atomic>
#include <future>

#include "FakeComponent.h"
#include "FakePatternTypeEnum.h"
#include "FakeServerBase.h"

namespace Fake {

// forward declarations
class FakeComponent;
class FakeServerBase;

class FakeClientBase
//:	public std::enable_shared_from_this<FakeClientBase>
{
private:
	FakeComponent *fake_component;
	std::weak_ptr<FakeServerBase> server_weak_ptr;

	std::mutex middleware_mutex;
	std::condition_variable send_buffer_cond_var;
	std::condition_variable sync_ack_cond_var;
	std::list<std::vector<std::string>> send_buffer;

	std::atomic<bool> cancelled;
	std::future<void> middleware_thread;

	void middleware_thread_runner();

protected:
	int middleware_connect(
			const FakePatternTypeEnum &type,
			const std::string &service_name,
			const std::vector<std::string> &object_type_names);
	int middleware_disconnect();

	// this method does not directly sends data, but pushes it onto an internal send buffer
	// which is drained by an internal thread. This simulates a middleware's asynchronous
	// communication behavior and allows for a more realistic testing.
	int send_data(const std::vector<std::string> &data, const bool &wait_for_ack=false);

public:
	FakeClientBase(FakeComponent *component);
	virtual ~FakeClientBase();

	// this method is called from the server whenever the server is
	// in the process of shutting down; the client should implement
	// this method such that it performs a disconnect
	virtual void on_sid() = 0;

	// this is a generic update method with a variable number
	// of serialized parameters
	virtual void on_update(const std::vector<std::string> &data) = 0;
};

} /* namespace Fake */

#endif /* FAKECLIENTBASE_H_ */
