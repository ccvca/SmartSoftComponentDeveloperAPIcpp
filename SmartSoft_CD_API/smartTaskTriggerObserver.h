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

#ifndef SMARTSOFT_INTERFACES_SMARTTASKTRIGGEROBSERVER_H_
#define SMARTSOFT_INTERFACES_SMARTTASKTRIGGEROBSERVER_H_

#include <smartStatusCode.h>
#include <smartPrescaleManager.h>

#include <map>

// C++11 interface
#include <chrono>
#include <mutex>
#include <condition_variable>

namespace Smart {

// forward declaration
class TaskTriggerSubject;


class TaskTriggerObserver {
	friend class TaskTriggerSubject;
private:
	bool trigger_cancelled;
	bool signalled;
	std::mutex observer_mutex;
	std::condition_variable_any trigger_cond_var;

protected:
	TaskTriggerSubject *subject;

	virtual void signalTrigger() {
		std::unique_lock<std::mutex> lock(observer_mutex);
		signalled = true;
		trigger_cond_var.notify_all();
	}

	virtual void cancelTrigger() {
		std::unique_lock<std::mutex> lock(observer_mutex);
		trigger_cancelled = true;
		trigger_cond_var.notify_all();
	}

public:
	TaskTriggerObserver(TaskTriggerSubject *subject, const unsigned int &prescaleFactor=1);
	virtual ~TaskTriggerObserver();

	virtual StatusCode waitOnTrigger() {
		std::unique_lock<std::mutex> lock(observer_mutex);
		if(subject == 0) return SMART_NOTACTIVATED;
		if(trigger_cancelled == true) {
			return SMART_CANCELLED;
		} else {
			if(signalled == false) {
				trigger_cond_var.wait(lock);
			}
			signalled = false;
			return SMART_OK;
		}
	}

	virtual StatusCode waitOnTrigger(const std::chrono::steady_clock::duration &timeout) {
		std::unique_lock<std::mutex> lock(observer_mutex);
		if(subject == 0) return SMART_NOTACTIVATED;
		if(trigger_cancelled == true) {
			return SMART_CANCELLED;
		} else {
			if(signalled == false) {
				if(trigger_cond_var.wait_for(lock, timeout)==std::cv_status::timeout) {
					return SMART_TIMEOUT;
				}
			}
			signalled = false;
			return SMART_OK;
		}
	}
};


class TaskTriggerSubject {
	friend class TaskTriggerObserver;
private:
	std::mutex subject_mutex;
	std::map<TaskTriggerObserver*, PrescaleManager> observers;

protected:
	void trigger_all_tasks() {
		std::unique_lock<std::mutex> lock(subject_mutex);
		for(auto it=observers.begin(); it!=observers.end(); it++) {
			if(it->second.isUpdateDue() == true) {
				it->first->signalTrigger();
			}
		}
	}

public:
	TaskTriggerSubject()
	{ }
	virtual ~TaskTriggerSubject()
	{ }

	void attach(TaskTriggerObserver *observer, const unsigned int &prescaleFactor=1) {
		std::unique_lock<std::mutex> lock(subject_mutex);
		observers[observer] = prescaleFactor;
	}
	void detach(TaskTriggerObserver *observer) {
		std::unique_lock<std::mutex> lock(subject_mutex);
		observer->cancelTrigger();
		observers.erase(observer);
	}
};



inline TaskTriggerObserver::TaskTriggerObserver(TaskTriggerSubject *subject, const unsigned int &prescaleFactor)
:	subject(subject)
,	trigger_cancelled(false)
,	signalled(false)
{
	if(subject != 0) {
		this->subject->attach(this, prescaleFactor);
	}
}
inline TaskTriggerObserver::~TaskTriggerObserver()
{
	if(subject != 0) {
		this->subject->detach(this);
	}
}

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTTASKTRIGGEROBSERVER_H_ */
