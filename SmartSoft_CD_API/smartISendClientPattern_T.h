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

#ifndef SMARTISENDCLIENTPATTERN_T_H_
#define SMARTISENDCLIENTPATTERN_T_H_

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
	ISendClientPattern();
	virtual ~ISendClientPattern();

    /** Connect this service requestor to the denoted service provider. An
     *  already established connection is first disconnected.
     *
     *  It is no problem to change the connection to a service provider at any
     *  point of time irrespective of any calls to send().
     *
     *  @param server   name of the server
     *  @param service  name of the service
     *
     *  @return status code
     *   - SMART_OK                  : everything is OK and connected to the specified service.
     *   - SMART_SERVICEUNAVAILABLE  : the specified service is currently not available and the
     *                                 requested connection can not be established. Service
     *                                 requestor is now not connected to any service provider.
     *   - SMART_INCOMPATIBLESERVICE : the specified service provider is not compatible (wrong communication
     *                                 pattern or wrong communication objects) to this service requestor and
     *                                 can therefore not be connected. Service requestor is now not connected
     *                                 to any service provider.
     *   - SMART_ERROR_COMMUNICATION : communication problems, service requestor is now not connected to any
     *                                 service provider.
     *   - SMART_ERROR               : something went wrong, service requestor is now not connected to any
     *                                 service provider.
     */
    StatusCode connect(const std::string& server, const std::string& service);

    /** Disconnect the service requestor from the service provider.
     *
     *  It is no problem to change the connection to a service provider at any
     *  point of time irrespective of any calls to send().
     *
     *  @return status code
     *   - SMART_OK                  : everything is OK and service requestor is disconnected from
     *                                 the service provider.
     *   - SMART_ERROR_COMMUNICATION : something went wrong at the level of the intercomponent
     *                                 communication. At least the service requestor is in the
     *                                 disconnected state irrespective of the service provider
     *                                 side clean up procedures.
     *   - SMART_ERROR               : something went wrong. Again at least the service requestor
     *                                 is in the disconnected state.
     */
    StatusCode disconnect();

    /** Allow or abort and reject blocking calls.
     *
     *  If blocking is set to false all blocking calls return with SMART_CANCELLED. This can be
     *  used to abort blocking calls. Since this pattern currently does not have any blocking
     *  member functions, currently without effect.
     *
     *  @param b (blocking)  true/false
     *
     *  @return status code
     *   - SMART_OK                  : new mode set
     *   - SMART_ERROR               : something went wrong
     */
    StatusCode blocking(const bool b);


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
    StatusCode send(const DataType& data);
};

} /* namespace Smart */

#endif /* SMARTISENDCLIENTPATTERN_T_H_ */
