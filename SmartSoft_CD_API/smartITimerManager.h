//===================================================================================
//
//  Copyright (C) 2019 Alex Lotz, Dennis Stampfer, Matthias Lutz, Christian Schlegel
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

#ifndef SMARTSOFT_INTERFACES_SMARTITIMERMANAGER_H_
#define SMARTSOFT_INTERFACES_SMARTITIMERMANAGER_H_

#include "smartChronoAliases.h"
#include "smartITimerHandler.h"

namespace Smart {

class ITimerManager {
public:
	using TimerId = long int;

	virtual ~ITimerManager() = default;

	/** Schedule a timer.
	 *
	 *  @param  handler       The handler that will be called when the timer
	 *                        expires.
	 *  @param  act           a value that will be passed to the handler (see Asynchronous Completion Token (ACT), POSA2)
	 *  @param  oneshot_time  relative time for the first timer expiration
	 *  @param  interval      Interval for periodic timers. A single shot timer
	 *                        is scheduled by default.
	 *
	 * @return timer_id: -1 on failure. Unique time id else. This id
	 *                      can be used to cancel a timer before it
	 *                      expires with cancelTimer() and to change
	 *                      the the interval of a timer with
	 *                      resetTimerInterval().
	 */
	virtual TimerId scheduleTimer(ITimerHandler *handler,
			const void *act, // Asynchronous Completion Token (ACT), see POSA2
			const Duration &oneshot_time, const Duration &interval = Duration::zero()) = 0;

	/** Cancel a single timer.
	 *
	 *  @param  timer_id   to cancel
	 *  @param  act        a pointer (to a pointer) to retrieve the act that was given on
	 *                     scheduleTimer(). Can be used to release resources
	 *                     (see Asynchronous Completion Token (ACT), POSA2).
	 *                     owned by act. If act == nullptr, nothing is retrieved.
	 *  @return 0 on success
	 *  @return -1 on error
	 */
	virtual int cancelTimer(const TimerId& timer_id, const void **act = nullptr) = 0;

	/** Resets the interval of a timer.
	 *
	 *  @param timer_id     to change
	 *  @param interval     new timer interval (relative to the current time)
	 *  @return 0 on success
	 *  @return -1 on error
	 */
	virtual int resetTimerInterval(const TimerId& timer_id, const Duration &interval) = 0;

	/** Cancel all timers associated with a handler
	 *
	 *  @param handler     cancel timers associated with this handler
	 *
	 *  @return number of timers canceled.
	 */
	virtual int cancelTimersOf(ITimerHandler *handler) = 0;

	/** Delete all currently scheduled timers.
	 */
	virtual void deleteAllTimers() = 0;
};

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTITIMERMANAGER_H_ */
