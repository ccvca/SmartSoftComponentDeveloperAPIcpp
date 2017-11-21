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

#ifndef SMARTSOFT_INTERFACES_SMARTICLIENTPATTERN_H_
#define SMARTSOFT_INTERFACES_SMARTICLIENTPATTERN_H_

#include "smartStatusCode.h"
#include "smartICommunicationPattern.h"

#include <string>

namespace Smart {

/** This is the base class for all client-patterns implementing a connection-oriented communication.
 *
 * Each ClientPattern needs to implement the connection-oriented
 * user API (i.e. the methods <b>connect()</b> / <b>disconnect()</b> ).
 * In addition all clients need to implement the method <b>blocking()</b>
 * that allows to globally manage all blocking user API calls.
 * This provides a thread-save implementation of blocking-call
 * management.
 */
class IClientPattern : public ICommunicationPattern {
protected:
	/// the server-name used for the last connection (can be used in derived classes)
	std::string connectionServerName;
	/// the service-name used for the last connection (can be used in derived classes)
	std::string connectionServiceName;
	/// the flag indicating the current blocking state of this client (can be used in derived classes)
	bool is_blocking;

	/** implements individual shutdown strategy
	 * The default behavior for each client during component shutdown
	 * is to call disconnect() which automatically disconnects
	 * the current client instance.
	 */
	virtual void on_shutdown() {
		// default behavior is to disconnect
		// all clients connected to this server
		this->disconnect();
	}

public:
    /** Default Constructor (not yet connecting with any service provider).
     *  connect() / disconnect() can always be used to change the connection
     *  status of the instance.
     *
     * @param component  the management class of the component
     */
	IClientPattern(IComponent *component)
	:	ICommunicationPattern(component)
	,	connectionServerName("")
	,	connectionServiceName("")
	,	is_blocking(true)
	{  }

    /** Connection Constructor (implicitly connecting with specified service provider).
     *  Connects to the denoted service and blocks until the connection
     *  has been established. Blocks infinitely if denoted service becomes
     *  unavailable since constructor performs retries. Blocking is useful to
     *  simplify startup of components which have mutual dependencies.
     *  connect() / disconnect() can always be used to change
     *  the connection-status of the instance.
     *
     * @param component  the management class of the component
     * @param server     name of the server (i.e. the component-name to connect to)
     * @param service    name of the service (i.e. the port-name of the component to connect to)
     */
	IClientPattern(IComponent *component, const std::string& server, const std::string& service)
	:	ICommunicationPattern(component)
	,	connectionServerName(server)
	,	connectionServiceName(service)
	,	is_blocking(true)
	{
		// perform autoconnect in derived classes
	}

	/** Default destructor.
	 */
	virtual ~IClientPattern()
	{
		// perform disconnect() in derived classes
	}

    /** Connect this service requestor to the denoted service provider. An
     *  already established connection is first disconnected. See disconnect()
     *
     *  It is no problem to change the connection to a service provider at any
     *  point in time irrespective of any other calls.
     *
     * @param server     name of the server (i.e. the component-name to connect to)
     * @param service    name of the service (i.e. the port-name of the component to connect to)
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
    virtual StatusCode connect(const std::string& server, const std::string& service) = 0;

    /** Disconnect the service requestor from the service provider.
     *
     *  It is no problem to change the connection to a service provider at any
     *  point in time irrespective of any other calls.
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
    virtual StatusCode disconnect() = 0;

    /** Allow or abort and reject blocking calls.
     *
     *  If blocking is set to false all blocking calls return with SMART_CANCELLED. This can be
     *  used to abort blocking calls.
     *
     *  @param blocking  true/false
     *
     *  @return status code
     *   - SMART_OK                  : new mode set
     *   - SMART_ERROR               : something went wrong
     */
    virtual StatusCode blocking(const bool blocking) = 0;
};

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTICLIENTPATTERN_H_ */
