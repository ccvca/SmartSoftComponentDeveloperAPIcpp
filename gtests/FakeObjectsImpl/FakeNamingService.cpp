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

#include "FakeNamingService.h"

namespace Fake {

FakeNamingService::FakeNamingService(FakeComponent *component)
:	component(component)
{  }

void FakeNamingService::register_server(
		FakeServerBase* server_ptr,
		const FakePatternTypeEnum &type,
		const std::string &service_name,
		const std::vector<std::string> &object_type_names)
{
	std::unique_lock<std::mutex> lock(fm_mutex);

	ServerEntry entry;
	entry.pattern_type = type;
	entry.service_name = service_name;
	entry.object_type_names = object_type_names;
	entry.server_ptr = server_ptr;

	registered_servers.push_back(entry);
}

void FakeNamingService::unregister_server(FakeServerBase* server_ptr)
{
	std::unique_lock<std::mutex> lock(fm_mutex);
	registered_servers.remove_if(
		[server_ptr](const ServerEntry &entry) {
			if(entry.server_ptr == server_ptr) {
				return true;
			}
			return false;
		}
	);
}

std::shared_ptr<FakeServerBase>
FakeNamingService::find_server(
		const FakePatternTypeEnum &type,
		const std::string &service_name,
		const std::vector<std::string> &object_type_names)
{
	std::unique_lock<std::mutex> lock(fm_mutex);
	for(auto &entry: registered_servers) {
		if(entry.pattern_type == type && entry.service_name == service_name) {
			if(entry.object_type_names.size() == object_type_names.size()) {
				bool all_same = true;
				for(size_t i=0; i<entry.object_type_names.size(); ++i) {
					if(entry.object_type_names[i] != object_type_names[i]) {
						all_same = false;
						break;
					}
					if(all_same == true && entry.server_ptr != nullptr) {
						return entry.server_ptr->shared_from_this();
					}
				}
			}
		}
	}
	return nullptr;
}

bool FakeNamingService::has_similar_servers(const std::string &service_name)
{
	for(auto &entry: registered_servers) {
		if(entry.service_name == service_name) {
			return true;
		}
	}
	return false;
}

} /* namespace Fake */
