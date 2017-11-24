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

#ifndef SMARTSOFT_INTERFACES_SMARTISENDCLIENTPATTERN_T_H_
#define SMARTSOFT_INTERFACES_SMARTISENDCLIENTPATTERN_T_H_

#include "smartIClientPattern.h"

namespace Smart {

/** Client part of the one-way, n->1 <b>Send</b> communication pattern.
 *
 *  Clients can send a specified DataType object to a server. If the
 *  send() method succeeds, this means that the sent object actually
 *  arrived at the server, but the server has the freedom to process
 *  this object later or throw it away altogether, depending on the
 *  implemented server semantics.
 *
 *  Template parameters
 *    - <b>DataType</b>: Communicated DataType (Communication Object)
 */
template <class DataType>
class ISendClientPattern : public IClientPattern {
public:
    /** Constructor (not wired with a service provider).
     *  connect() / disconnect() can always be used to change
     *  the status of the connection.
     *
     *  @param component  the management class of the component
     */
	ISendClientPattern(IComponent* component)
	:	IClientPattern(component)
	{  }

    /** Connection Constructor (implicitly wiring with specified service provider).
     *  Connects to the denoted service and blocks until the connection
     *  has been established. Blocks infinitely if denoted service becomes
     *  unavailable since constructor performs retries. Blocking is useful to
     *  simplify startup of components which have mutual dependencies.
     *  connect() / disconnect() can always be used to change
     *  the status of the instance.
     *
     * @param component  the management class of the component
     * @param server     name of the server (i.e. the component-name to connect to)
     * @param service    name of the service (i.e. the port-name of the component to connect to)
     */
	ISendClientPattern(IComponent* component, const std::string& server, const std::string& service)
	:	IClientPattern(component, server, service)
	{  }

    /** Destructor.
     *
     *  The destructor calls disconnect().
     */
	virtual ~ISendClientPattern()
	{  }

    /** Perform a one-way communication. Appropriate status codes make
     *  sure that the information has been transferred.
     *
     *  @param data the object to be sent (Communication Object)
     *
     *  @return status code:
     *    - SMART_OK                  : everything is ok and communication object sent to server
     *    - SMART_DISCONNECTED        : the client is disconnected and no send can be made
     *    - SMART_ERROR_COMMUNICATION : communication problems, data not transmitted
     *    - SMART_ERROR               : something went wrong, data not transmitted
     */
    StatusCode send(const DataType& data) = 0;
};

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTISENDCLIENTPATTERN_T_H_ */
