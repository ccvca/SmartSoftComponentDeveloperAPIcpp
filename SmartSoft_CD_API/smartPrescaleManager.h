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

#ifndef SMARTPRESCALEMANAGER_H_
#define SMARTPRESCALEMANAGER_H_

namespace Smart {

/** This helper class allows dividing an update frequency by a given factor.
 *
 * Often data arrives at input ports at a certain periodic update frequency.
 * Clients however often are not interested in all updates but in a subset
 * of updates. The lower rate can be specified in the component model and
 * this class helps in realizing this subdivided update frequency.
 *
 */
class PrescaleManager
{
private:
	// internal copy of the prescale factor
	unsigned int prescaleFactor;
	// internal update counter
	unsigned int updateCounter;

public:
	/** default initialization constructor
	 * @param prescaleFactor divides the update frequency by this value
	 */
	PrescaleManager(const unsigned int &prescaleFactor = 1)
	:	prescaleFactor(prescaleFactor)
	,	updateCounter(1)
	{  }

	PrescaleManager& operator=(const unsigned int &prescaleFactor) {
		this->prescaleFactor = prescaleFactor;
		this->updateCounter = 1;
		return *this;
	}

	// method increments the internal update-counter and checks whether the next update is due.
	inline bool isUpdateDue() {
		if(updateCounter == prescaleFactor) {
			updateCounter = 1;
			return true;
		} else {
			updateCounter++;
			return false;
		}
	}
};

} /* namespace Smart */

#endif /* SMARTPRESCALEMANAGER_H_ */
