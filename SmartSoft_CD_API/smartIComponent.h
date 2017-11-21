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

#ifndef SMARTSOFT_INTERFACES_SMARTICOMPONENT_H_
#define SMARTSOFT_INTERFACES_SMARTICOMPONENT_H_

#include <string>

// SmartSoft includes
#include "smartIStatusCode.h"
#include "smartIShutdownObserver.h"
#include "smartITimerManager.h"

namespace Smart {

/** Component management class
 *
 *  Every software-component must provide exactly one instance of an IComponent.
 *  This class provides the base infrastructure for a software-component.
 *  This base-infrastructure is shared among all component's subentities such as
 *  the component's client- and server-ports as well as the user-tasks.
 *
 *  The main thread of a component is used by this class to run() the
 *  SmartSoft framework. Therefore all user activities must be started
 *  before in separate threads. Further details can be found in the
 *  examples.
 *
 */
class IComponent : public IShutdownSubject {
protected:
	/// the internal blocking flag
	bool is_blocking;
	/// the internal component-name
	std::string componentName;

public:
	/** Default constructor initializes the component with a given component-name.
	 *
	 *   @param componentName  unique name of the whole component, which is used by the clients to
	 *                         address this server
	 */
	IComponent(const std::string &componentName)
	:	componentName(componentName)
	,	is_blocking(true)
	{  }

	/** Destructor.
	 *
	 */
	virtual ~IComponent()
	{  }

	/** Runs the SmartSoft framework within a component which includes handling
	 *  intercomponent communication etc. This method is called in the main()-routine
	 *  of a component after all initializations including activation of user threads
	 *  are finished. Thypically the last code-line of the man function
	 *  looks like this: "return component.run();".
	 *
	 *  @return status code
	 *    - SMART_ERROR_UNKNOWN: unknown error (probably a corba problem)
	 *    - SMART_OK: gracefully terminated
	 */
	virtual StatusCode run(void) = 0;

	/** Allow or abort and reject blocking calls in communication patterns of this component.
	 *
	 *  If blocking is set to false all blocking calls of all communication patterns
	 *  of this component return with SMART_CANCELLED. This can be used to abort blocking
	 *  calls of ALL communication patterns inside a component.
	 *
	 *  @param b (blocking)  true/false
	 *
	 *  @return status code
	 *    - SMART_OK    : new mode set
	 *    - SMART_ERROR : something went wrong
	 */
	virtual StatusCode blocking(const bool b) = 0;

	/** Returns the component name
	 *
	 * The component name is a read-only value that is set once at component
	 * startup. This name serves as a parent namespace for all component's
	 * communication ports.
	 *
	 * @return the component name
	 */
	inline std::string getName() const {
		return componentName;
	}

	/** get timer-manager for registering timer-handlers
	 *
	 *  An instance of an ITimerManager is instantiated by an IComponent.
	 *  An ITimerManager allows activation of ITimerHandler instances
	 *  that are triggered (once or repeatedly) after a given time period.
	 *
	 *  @return a pointer to the ITimerManager
	 */
	virtual ITimerManager* getTimerManager() = 0;
};

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTICOMPONENT_H_ */
