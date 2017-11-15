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

#ifndef SMARTSOFT_INTERFACES_SMARTISERVERPATTERN_H_
#define SMARTSOFT_INTERFACES_SMARTISERVERPATTERN_H_

#include "smartIShutdownObserver.h"
#include "smartIComponent.h"

namespace Smart {

/** This is the base class for all server-patterns.
 *
 * The main two functionalities that are shared among all server-patterns are:
 * - Constructor with a given serviceName
 * - individual serverInitiatedDisconnect() implementation
 *
 * A server-pattern is supposed to fully initialize its internal
 * server-side infrastructure within its constructor such that after
 * the constructor the server is ready to deliver its service.
 *
 * The individual serverInitiatedDisconnect() is automatically
 * triggered from within the on_shutdown() method and from within
 * the destructor.
 */
class IServerPattern : public IShutdownObserver {
protected:
	/// the internal pointer to a component (can be used in derived classes)
	IComponent* component;
	/// the current serviceName (can be used in derived classes)
	std::string serviceName;

	/** implements individual shutdown strategy.
	 *
	 * The default behavior for each server during component shutdown
	 * is to call serverInitiatedDisconnect() which automatically disconnects
	 * all currently connected clients.
	 */
	virtual void on_shutdown() {
		// default behavior is to disconnect
		// all clients connected to this server
		this->serverInitiatedDisconnect();
	}

	/** implements server-initiated-disconnect (SID)
	 *
	 *	The server-initiated-disconnect is specific to a certain server implementation.
	 *	Each server should be able triggering disconnecting all currently connected
	 *	clients in case e.g. the component of that server is about to shutdown.
	 *	Disconnecting clients before that ensures that the clients remain in a defined
	 *	state (namely disconnected) after the server is gone.
	 */
	virtual void serverInitiatedDisconnect() = 0;
public:
    /** Constructor.
     *
     *  Default constructor for a ServerPattern. Derived classes should entirely initialize
     *  all internal server resources such that after the constructor the server
     *  becomes ready to deliver its service.
     *
     *  @param component   management class of the component
     *  @param serviceName name of the service
     */
	IServerPattern(IComponent* component, const std::string& serviceName)
	:	IShutdownObserver(component)
	,	component(component)
	,	serviceName(serviceName)
	{  }

    /** Destructor.
     *
     *  Properly disconnects all service requestors in case of destruction.
     */
	virtual ~IServerPattern()
	{
		// execute serverInitiatedDisconnect() in derived classes
	}
};

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTISERVERPATTERN_H_ */
