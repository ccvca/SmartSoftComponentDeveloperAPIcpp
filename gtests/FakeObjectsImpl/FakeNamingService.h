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

#ifndef FAKENAMINGSERVICE_H_
#define FAKENAMINGSERVICE_H_

#include <list>
#include <vector>
#include <string>
#include <memory>
#include <mutex>

#include "FakeComponent.h"
#include "FakePatternTypeEnum.h"
#include "FakeServerBase.h"

namespace Fake {

// forward declarations
class FakeComponent;
class FakeServerBase;

class FakeNamingService {
private:
	std::mutex fm_mutex;
	FakeComponent *component;

	struct ServerEntry {
		FakePatternTypeEnum pattern_type;
		std::string service_name;
		std::vector<std::string> object_type_names;
		FakeServerBase* server_ptr;
	};

	std::list<ServerEntry> registered_servers;
public:
	FakeNamingService(FakeComponent *component);

	void register_server(
			FakeServerBase* server_ptr,
			const FakePatternTypeEnum &type,
			const std::string &service_name,
			const std::vector<std::string> &object_type_names);

	void unregister_server(FakeServerBase* server_ptr);

	std::shared_ptr<FakeServerBase> find_server(
			const FakePatternTypeEnum &type,
			const std::string &service_name,
			const std::vector<std::string> &object_type_names);

	bool has_similar_servers(const std::string &service_name);
};

} /* namespace Fake */

#endif /* FAKENAMINGSERVICE_H_ */
