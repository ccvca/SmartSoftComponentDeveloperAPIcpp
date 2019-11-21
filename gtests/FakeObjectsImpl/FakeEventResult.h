/*
 * FakeEventResult.h
 *
 *  Created on: Jul 22, 2019
 *      Author: alexej
 */

#ifndef FAKEEVENTRESULT_H_
#define FAKEEVENTRESULT_H_

#include <list>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include <smartIEventClientPattern_T.h>

namespace Fake {

template <typename EventType>
class EventResult {
private:
	EventType event;
	const Smart::EventMode mode;
	std::atomic<bool> has_new_event;
	std::atomic<bool> is_consumed;
	std::atomic<bool> is_deactivated;
	std::condition_variable_any new_event_cond_var;

public:
	EventResult(const Smart::EventMode &mode)
	:	event()
	,	mode(mode)
	,	has_new_event(false)
	,	is_consumed(false)
	,	is_deactivated(false)
	{  }

	inline bool isSingleMode() const {
		return mode == Smart::EventMode::single;
	}

	inline bool hasNewEvent() const {
		return has_new_event;
	}

	inline bool isConsumed() const {
		return is_consumed;
	}

	inline bool isDeactivated() const {
		return is_deactivated;
	}

	inline void setNewEvent(const EventType &event) {
		if(is_deactivated == true) {
			// no need to update the event if it has been deactivated
			return;
		}

		// update event only in continuous mode or if the single event has not yet been consumed
		if(mode == Smart::EventMode::continuous || is_consumed==false) {
			this->event = event;
			has_new_event = true;
			is_consumed = false;
			new_event_cond_var.notify_all();
		}
	}

	inline bool waitForEvent(std::unique_lock<std::recursive_mutex> &event_lock, const bool &await_next_event, const Smart::Duration &timeout) {
		if(is_deactivated) {
			return false;
		} else if(await_next_event || !has_new_event) {
			if(timeout != Smart::Duration::max()) {
				auto result = new_event_cond_var.wait_for(event_lock, timeout);
				if(result == std::cv_status::timeout) {
					return false;
				}
			} else {
				new_event_cond_var.wait(event_lock);
			}
		}
		return has_new_event;
	}

	inline EventType consumeEvent() {
		has_new_event = false;
		is_consumed = true;
		return event;
	}

	inline void deactivateEvent() {
		has_new_event = false;
		is_deactivated = true;
		new_event_cond_var.notify_all();
	}
	inline void signalEvent() {
		new_event_cond_var.notify_all();
	}
};

} /* namespace Fake */

#endif /* FAKEEVENTRESULT_H_ */
