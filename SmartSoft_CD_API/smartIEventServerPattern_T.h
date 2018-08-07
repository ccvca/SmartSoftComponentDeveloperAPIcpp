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

#ifndef SMARTSOFT_INTERFACES_SMARTIEVENTSERVERPATTERN_T_H_
#define SMARTSOFT_INTERFACES_SMARTIEVENTSERVERPATTERN_T_H_

#include "smartIInputHandler_T.h"
#include "smartIServerPattern.h"

namespace Smart {

/// struct used by IEventHandler and IEventClientPattern internally
template<class ActivationType, class EventType, class UpdateType>
struct TestEventType {
	ActivationType *param;
	EventType *event;
	UpdateType status;
};

/** Condition Test Handler (decides at server whether event fires or not).
 *
 */
template<class ActivationType, class EventType, class UpdateType>
class IEventTestHandler
{
public:
  virtual ~IEventTestHandler() {  }

  /** This is the test method which decides whether the event fires or
   *  not.
   *
   *  As soon as the EventServer::put() is called, the pattern calls
   *  EventTestHandler::testEvent() to decide for each event
   *  activation parameter set individually whether this parameter
   *  set requires the event to fire.
   *
   *  The parameters p are provided by the client individually with
   *  every event activation, the current state s is provided via
   *  the EventServer::put() method when the server wants the event
   *  conditions to be checked.
   *
   *  The test method has to be provided by the user. In case the
   *  event fires, one can return data in the event answer object
   *  e.
   *
   * <em>Attention:</em> this function will be called within the same
   *                 context as the EventServer::put()-method.
   *                 Therefore pay attention to blocking calls etc.
   *
   *  @param p activation parameter set to be checked (Communication Object).
   *           Can be modified in the test event handler to store state
   *           information in the parameter object (for example needed to
   *           easily implement an event which only fires with state changes)
   *  @param e event answer object (Communication Object) which returns data
   *           in case of firing to the client which is responsible for this event
   *           activation
   *  @param s current information against which the testEvent() checks
   *           the parameters p
   *
   *  @return status code
   *     - true: fire event (predicate true)
   *     - false: do not fire event (predicate false) */
   virtual bool testEvent(ActivationType& p, EventType& e, const UpdateType& s) = 0;

   /** This is a hook which is called whenever an event gets activated.
    *
    * Each time a client activates an event, this hook is called with the corresponding
    * parameter. The overloading of this hook is optional. Blocking calls within this
    * hook should be avoided.
    *
    * @param p event activation parameter set
    */
   virtual void onActivation(const ActivationType& p) {
      // do nothing by default
   };
};



template<class ActivationType, class EventType, class UpdateType, class EventIdType>
class IEventServerPattern
:	public IServerPattern
{
protected:
    /// handler object that contains the test function
	IEventTestHandler<ActivationType,EventType,UpdateType> *testHandler;
public:
    /** Default constructor.
     *
     *  Note that a handler has to be supplied. Without a handler, the
     *  QueryServer could not accept a query.
     *
     *  @param component management class of the component
     *  @param service   name of the service
     *  @param testHandler is the pointer to an EventTestHandler
     */
	IEventServerPattern(IComponent* component, const std::string& service, IEventTestHandler<ActivationType,EventType,UpdateType> *testHandler)
	:	IServerPattern(component, service)
	,	testHandler(testHandler)
	{  }

    /** Destructor.
     *  Properly disconnects all service requestors in case of destruction
     *  such that all pending queries are handled correctly at client side
     *  even when the service provider disappears during pending queries.
     */
	virtual ~IEventServerPattern() {  }

    /** Initiate testing the event conditions for the activations.
     *
     *  @param state contains the current information checked in testEvent()
     *         against the individual activation parameters.
     *
     *  @return status code
     *   - SMART_OK                  : everything is ok
     *   - SMART_ERROR_COMMUNICATION : communication problems
     *   - SMART_ERROR               : something went wrong
     *
     */
    virtual StatusCode put(const UpdateType& state) = 0;
};

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTIEVENTSERVERPATTERN_T_H_ */
