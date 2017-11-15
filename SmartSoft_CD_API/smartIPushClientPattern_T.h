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

#ifndef SMARTSOFT_INTERFACES_SMARTIPUSHCLIENTPATTERN_T_H_
#define SMARTSOFT_INTERFACES_SMARTIPUSHCLIENTPATTERN_T_H_

#include "smartIClientPattern.h"

namespace Smart {

/** Client part of the Push pattern to provide a flexible push
 *  service. Clients can subscribe to regularly get every n-th
 *  update. This class inherits the API from  IClientPattern.
 *
 *  Template parameters
 *    - <b>DataType</b>: Pushed value class (Communication Object)
 *
 */
template <class DataType>
class IPushClientPattern : public IClientPattern {
public:
    /** Constructor (not wired with service provider and not exposed as port).
     *  connect() / disconnect() can always be used to change
     *  the status of the instance. Instance is not connected to a service provider
     *  and is not exposed as port wireable from outside the component.
     *
     * @param component  the management class of the component
     */
	IPushClientPattern(IComponent* component)
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
	IPushClientPattern(IComponent* component, const std::string& server, const std::string& service)
	:	IClientPattern(component, server, service)
	{  }

    /** Destructor.
     *  The destructor calls disconnect() and therefore properly cleans up
     *  every pending data reception and removes the instance from the set of wireable ports.
     */
    virtual ~IPushClientPattern()
    {  }

    /** Subscribe at the server to periodically get every n-th update. A
     *  newly subscribed client gets the next available new data and is
     *  then updated with regard to its individual prescale-factor.
     *  If the prescale factor is omitted then every update will be received.
     *
     *  @param prescale  whole-numbered value to set the update rate to
     *                   every n-th value (must be greater than 0)
     *
     *  @return status code
     *    - SMART_OK                  : everything is ok and client is subscribed
     *    - SMART_DISCONNECTED        : client is not connected to a server and can therefore
     *                                  not subscribe for updates, not subscribed
     *    - SMART_ERROR_COMMUNICATION : communication problems, not subscribed
     *    - SMART_ERROR               : something went wrong, not subscribed
     */
    virtual StatusCode subscribe(const int &prescale = 1) = 0;

    /** Unsubscribe to get no more updates. All blocking calls are aborted with the appropriate
     *  status and yet received and still buffered data is deleted to avoid returning old data.
     *
     *  @return status code
     *    - SMART_OK                  : everything is ok and client is now unsubscribed or
     *                                  has already been unsubscribed
     *    - SMART_ERROR_COMMUNICATION : communication problems, not unsubscribed
     *    - SMART_ERROR               : something went wrong, not unsubscribed
     *
     * (can not return SMART_DISCONNECTED since then client is for sure also unsubscribed
     *  which results in SMART_OK)
     */
    virtual StatusCode unsubscribe() = 0;

    /** Non-blocking call to immediately return the latest available
     *  data buffered at the client side from the most recent update.
     *
     *  No data is returned as long as no update is received since
     *  subscription. To avoid returning old data, no data is
     *  returned after the client is unsubscribed or when the
     *  server is not active.
     *
     * @param d is set to the newest currently available data
     *
     * @return status code
     *   - SMART_OK                  : everything ok and latest data since client got subscribed
     *                                 is returned.
     *   - SMART_NODATA              : client has not yet received an update since subscription and
     *                                 therefore no data is available and no data is returned.
     *   - SMART_NOTACTIVATED        : the server is currently not active and does therefore not
     *                                 provide updates at the expected rate. No valid data returned.
     *   - SMART_UNSUBSCRIBED        : no data available since client is not subscribed and can
     *                                 therefore not receive updates. Method does not return old data from
     *                                 last subscription since these might be based on too old parameter
     *                                 settings. To get data one has to be subscribed.
     *   - SMART_DISCONNECTED        : no data returned since client is even not connected to a server.
     *   - SMART_ERROR               : something went wrong
     */
    virtual StatusCode getUpdate(DataType& d) = 0;

    /** Blocking call which waits until the next update is received.
     *
     *  Blocking is aborted with the appropriate status if either the
     *  server gets deactivated, the client gets unsubscribed or disconnected
     *  or if blocking is not allowed any more at the client.
     *
     *  @param d is set to the newest currently available data
     *
     *  @return status code
     *   - SMART_OK                  : everything is ok and just received data is returned.
     *   - SMART_CANCELLED           : blocking is not allowed or is not allowed anymore. Waiting for the
     *                                 next update is aborted and no valid data is returned.
     *   - SMART_NOTACTIVATED        : the server is currently not active and does therefore not provide updates
     *                                 at the expected rate. No valid data returned.
     *   - SMART_UNSUBSCRIBED        : returns immediately without data if the client is not subscribed.
     *   - SMART_DISCONNECTED        : returns immediately without data since client is even not connected
     *                                 to a server.
     *   - SMART_ERROR               : something went completely wrong and no valid data returned.
     */
    virtual  StatusCode getUpdateWait(DataType& d) = 0;
};

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTIPUSHCLIENTPATTERN_T_H_ */
