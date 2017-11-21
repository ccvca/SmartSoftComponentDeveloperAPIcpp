//===================================================================================
//
//  Copyright (C) 2017 Alex Lotz, Dennis Stampfer, Matthias Lutz, Christian Schlegel
//
//        lotz@hs-ulm.de
//        stampfer@hs-ulm.de
//        lutz@hs-ulm.de
//        schlegel@hs-ulm.de
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

#ifndef SMARTSOFT_INTERFACES_SMARTTASKINTERACTIONOBSERVER_H_
#define SMARTSOFT_INTERFACES_SMARTTASKINTERACTIONOBSERVER_H_

#include <list>
#include <mutex>

namespace Smart {

// forward declaration
class TaskInteractionSubject;

class ITaskInteractionObserver {
public:
	ITaskInteractionObserver()
	{ }
	virtual ~ITaskInteractionObserver()
	{ }

	virtual void update_from(TaskInteractionSubject *subject) = 0;
};



class TaskInteractionSubject {
	friend class ITaskTriggerObserver;
private:
	std::mutex subject_mutex;
	std::list<ITaskInteractionObserver*> observers;

protected:
	virtual void notify_all_tasks() {
		std::unique_lock<std::mutex> lock(subject_mutex);
		for(auto it=observers.begin(); it!=observers.end(); it++) {
			(*it)->update_from(this);
		}
	}

public:
	TaskInteractionSubject()
	{ }
	virtual ~TaskInteractionSubject()
	{ }

	virtual void attach(ITaskInteractionObserver *observer) {
		std::unique_lock<std::mutex> lock(subject_mutex);
		observers.push_back(observer);
	}
	virtual void detach(ITaskInteractionObserver *observer) {
		std::unique_lock<std::mutex> lock(subject_mutex);
		observers.remove(observer);
	}
};

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTTASKINTERACTIONOBSERVER_H_ */
