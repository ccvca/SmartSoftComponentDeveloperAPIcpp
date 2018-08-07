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

#ifndef SMARTSOFT_INTERFACES_SMARTIPUSHSERVERPATTERN_T_H_
#define SMARTSOFT_INTERFACES_SMARTIPUSHSERVERPATTERN_T_H_

#include "smartStatusCode.h"
#include "smartIServerPattern.h"

namespace Smart {

/** Server part of the Push pattern to provide a push service.
 *  New data is provided to the server by the put() method.
 *  It immediately distributes the new data to all at that point
 *  subscribed clients taking into account their
 *  individual prescale factors (see IPushClientPattern).
 *
 *  Template parameters
 *    - <b>DataType</b>: Pushed value class (Communication Object)
 */
template <class DataType>
class IPushServerPattern : public IServerPattern {
public:
    /** Default Constructor.
     *
     *  The default constructor should initialize all internal
     *  server resources such that the put() method can be used.
     *
     * @param component   management class of the component
     * @param serviceName name of the service
     *
     */
	IPushServerPattern(IComponent* component, const std::string& serviceName)
	:	IServerPattern(component, serviceName)
	{  }

    /** Destructor.
     *  Properly disconnects all service requestors in case of destruction
     *  such that all connected and subscribed clients are unsubscribed and
     *  disconnected properly.
     */
	virtual ~IPushServerPattern()
	{  }

    /** Provide new data which is sent to all subscribed clients
     *  taking into account their individual prescale factors.
     *  Prescale factors are always whole-numbered multiples of the server
     *  update intervals.
     *
     *  (Individual update interval counters are incremented each
     *   time this member function is called irrespectively of the
     *   elapsed time. One should use the time triggered handler to
     *   call the put() member function with the appropriate timing.)
     *
     *  @param d contains the newly acquired data to be sent as
     *           update.
     *
     *  @return status code
     *    - SMART_OK                  : everything is ok
     *    - SMART_ERROR_COMMUNICATION : communication problems caused by at least
     *                                  one client. The other clients are updated
     *                                  correctly.
     *    - SMART_ERROR               : something went completely wrong with at least one
     *                                  client. Some clients might still been
     *                                  updated correctly.
     */
    virtual StatusCode put(const DataType& d) = 0;
};

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTIPUSHSERVERPATTERN_T_H_ */
