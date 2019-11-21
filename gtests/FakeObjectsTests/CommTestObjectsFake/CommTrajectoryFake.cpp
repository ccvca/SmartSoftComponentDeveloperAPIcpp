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

#include "CommTrajectoryFake.h"

std::vector<std::string> serialize(const CommTestObjects::CommTrajectory &object)
{
	std::vector<std::string> data = serialize(object.description);

	for(const auto& item: object.trajectory) {
		auto pose3d = serialize(item);
		data.insert(data.end(), pose3d.begin(), pose3d.end());
	}

	return data;
}

bool convert(const std::vector<std::string> &data, CommTestObjects::CommTrajectory &object)
{
	if(data.size() > 1) {
		std::vector<std::string> front_elem(1, data[0]);
		convert(front_elem, object.description);

		object.trajectory.clear();
		size_t counter = 1;
		while( (counter*3) < data.size()) {
			CommTestObjects::Comm3dPose pose3d;
			auto end_offset = counter * 3 + 1;
			auto start_offset = end_offset - 3;
			std::vector<std::string> current(data.begin()+start_offset, data.begin()+end_offset);
			convert(current, pose3d);
			object.trajectory.push_back(pose3d);
			counter++;
		}
		return true;
	}
	return false;
}
