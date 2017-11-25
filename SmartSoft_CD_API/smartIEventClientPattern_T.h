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

#ifndef SMARTSOFT_INTERFACES_SMARTIEVENTCLIENTPATTERN_T_H_
#define SMARTSOFT_INTERFACES_SMARTIEVENTCLIENTPATTERN_T_H_

#include "smartIClientPattern.h"
#include "smartIInputHandler_T.h"

#include <chrono>

namespace Smart {

/// struct used by IEventHandler and IEventClientPattern internally
template<class EventType, class EvenIdType>
struct EventInputType {
	EventType event;
	EvenIdType event_id;
};


// forward declaration
template<class ActivationType, class EventType, class EventIdType>
class IEventClientPattern;

/** Handler class to process fired events asynchronously at the client.
 *
 *  Register it with the constructor of the event client.
 *
 *  Template parameters:
 *    - <b>E</b>: Event answer class (Communication Object)
 *
 *  Demonstrated in <a href="/drupal/?q=node/51#fifth-example">fifth example</a>
 */
template<class EventType, class EventIdType>
class IEventHandler : public IInputHandler< EventInputType<EventType,EventIdType> > {
protected:
	/** implements IInputHandler
	 *
	 *  This handler method delegates the call to the handleSend() method.
	 *
	 *  @param input the input data-object
	 */
	virtual void handle_input(const EventInputType<EventType,EventIdType>& input) {
		this->handleEvent(input.event_id, input.event);
	}
public:
	IEventHandler(InputSubject< EventInputType<EventType,EventIdType> > *subject)
	:	IInputHandler< EventInputType<EventType,EventIdType> >(subject)
	{ }

  // User interface
   virtual ~IEventHandler() { }

  /** Handler which is called by the event client pattern with every
   *  fired event.
   *
   *  The handler method has to be provided by the user by subclassing.
   *
   *  @param id contains the activation id to be able to find out which
   *         activation caused the current event
   *  @param event is the event answer class (Communication Object)
   *         received due to the firing of the activation with identifier id.
   */
  virtual void handleEvent(const EventIdType &id, const EventType& event) = 0;
};


/** Handles the event service on client side.
 *
 *  Template parameters:
 *    - <b>P</b>: Activation parameter class (Communication Object)
 *            contains individual parameters of the event activation.
 *    - <b>E</b>: Event answer class (Communication Object)
 *            is returned when an event fires and can contain further
 *            details why and under which circumstances an event fired.
 *
 *  Demonstrated in <a href="/drupal/?q=node/51#fifth-example">fifth example</a>
 */
template<class ActivationType, class EventType, class EventIdType>
class IEventClientPattern
:	public IClientPattern
,	public InputSubject< EventInputType<EventType,EventIdType> >
{
public:
	/// Mode of event
	enum EventMode {
	  /// fire event only once.
	  single,
	  /// event fires whenever the condition is met.
	  continuous
	};

    /** Constructor (not wired with a service provider).
     *  connect() / disconnect() can always be used to change
     *  the status of the connection.
     *
     *  @param component  the management class of the component
     */
	IEventClientPattern(IComponent* component)
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
	IEventClientPattern(IComponent* component, const std::string& server, const std::string& service)
	:	IClientPattern(component, server, service)
	{  }

    /** Destructor.
     *
     *  The destructor calls disconnect().
     */
	virtual ~IEventClientPattern()
	{  }

    /** Activate an event with the provided parameters in either "single" or "continuous" mode.
     *
     *  @param mode        "single" or "continuous" mode
     *  @param parameter   activation parameter class (Communication Object)
     *  @param id          is set to the unique id of the event activation
     *
     *  @return status code
     *    - SMART_OK                  : everything is ok, event is activated and <I>id</I> contains
     *                                  a valid activation identifier.
     *    - SMART_DISCONNECTED        : activation not possible since not connected to a server. No
     *                                  valid activation identifier <I>id</I> returned.
     *    - SMART_ERROR_COMMUNICATION : communication problems, event not activated, <I>id</I> is not
     *                                  a valid activation identifier.
     *    - SMART_ERROR               : something went wrong, event not activated, <I>id</I> is not
     *                                  a valid activation identifier.
     */
    virtual StatusCode activate(const EventMode mode, const ActivationType& parameter, EventIdType& id) = 0;

    /** Deactivate the event with the specified identifier.
     *
     *  An event must always be deactivated, even if it has already
     *  fired in single mode. This is just necessary for cleanup
     *  procedures and provides a uniform user API independently of the
     *  event mode. Calling deactivate() while there are blocking calls
     *  aborts them with the appropriate status code.
     *
     *  @param id of event to be disabled
     *
     *  @return status code
     *    - SMART_OK                  : everything is ok and event is deactivated
     *    - SMART_WRONGID             : there is no active event available with this id
     *    - SMART_ERROR_COMMUNICATION : communication problems, event not deactivated
     *    - SMART_ERROR               : something went wrong, event not deactivated
     *
     * (Hint: can not return SMART_DISCONNECTED since then each event is for sure also
     *        deactivated resulting in SMART_WRONGID)
     */
    virtual StatusCode deactivate(const EventIdType &id) = 0;

    /** Check whether event has already fired and return immediately
     *  with status information.
     *
     *  This method does not consume an available event.
     *
     *  @param id of the event activation to be checked
     *
     *  @return status code
     *    - single mode:
     *      - SMART_OK                : event fired already, is still available and can be consumed by
     *                                  calling getEvent(),
     *      - SMART_ACTIVE            : event has not yet fired
     *      - SMART_PASSIVE           : event fired already and is already consumed.
     *      - SMART_WRONGID           : there is no activation available with this <I>id</I>
     *    - continuous mode:
     *      - SMART_OK                : unconsumed event is available. Since events are
     *                                  overwritten this means that at least one new
     *                                  event has been received since the last event
     *                                  consumption.
     *      - SMART_ACTIVE            : currently there is no unconsumed event available.
     *      - SMART_WRONGID           : there is no activation available with this <I>id</I>
     */
    virtual StatusCode tryEvent(const EventIdType &id) = 0;

    /** Blocking call which waits for the event to fire and then consumes the event.
     *
     *  This method consumes an event. Returns immediately if an unconsumed event is
     *  available. Blocks otherwise till event becomes available. If method is called
     *  concurrently from several threads with the same <I>id</I> and method is blocking,
     *  then every call returns with the same <I>event</I> once the event fired. If there is
     *  however already an unconsumed event available, then only one out of the concurrent
     *  calls consumes the event and the other calls return with appropriate status codes.
     *
     *  @param id of the event activation
     *  @param event is set to the returned event if fired (Communication Object)
     *
     *  - <b>single mode</b>:
     *      <p>
     *      Since an event in single mode fires only once, return immediately
     *      if the event is already consumed. Also return immediately with an
     *      available and unconsumed event and consume it. Otherwise wait until
     *      the event fires.
     *      </p>
     *      <p>
     *      <b>Returns status code</b>:
     *      </p>
     *      <p>
     *        - SMART_OK            : event fired and event is consumed and returned.
     *        - SMART_PASSIVE       : event fired and got consumed already. Returns immediately without
     *                                valid event since it can not fire again in single mode.
     *        - SMART_CANCELLED     : waiting for the event to fire has been aborted or blocking is not
     *                                not allowed anymore. Therefore no valid <I>event</I> is returned.
     *        - SMART_DISCONNECTED  : client is disconnected or got disconnected while waiting and
     *                                therefore no valid <I>event</I> is returned and the activation
     *                                identifier <I>id</I> is also not valid any longer due to
     *                                automatic deactivation.
     *        - SMART_NOTACTIVATED  : got deactivated while waiting and therefore <I>event</I> not valid and
     *                                also <I>id</I> not valid any longer.
     *        - SMART_WRONGID       : there is no activation available with this <I>id</I> and therefore
     *                                <I>event</I> not valid.
     *      </p>
     *
     *  - <b>continuous mode</b>:
     *     <p>
     *     Returns immediately if an unconsumed event is
     *     available. Returns the newest unconsumed event since
     *     activation. Otherwise waits until the event fires again.
     *     </p>
     *     <p>
     *     <b>Returns status code</b>:
     *     </p>
     *     <p>
     *        - SMART_OK            : unconsumed event is available and event is consumed and returned.
     *                                Due to the overwriting behavior, only the latest event is available.
     *        - SMART_CANCELLED     : blocking is not allowed anymore therefore blocking call has been aborted and
     *                                <I>event</I> is not valid.
     *        - SMART_DISCONNECTED  : got disconnected while waiting and therefore <I>event</I> not valid and
     *                                also <I>id</I> not valid any longer due to automatic deactivation.
     *        - SMART_NOTACTIVATED  : got deactivated while waiting and therefore <I>event</I> not valid and
     *                                also <I>id</I> not valid any longer.
     *        - SMART_WRONGID       : there is no activation available with this <I>id</I> and therefore
     *                                <I>event</I> not valid.
     *     </p>
     */
    virtual StatusCode getEvent(const EventIdType &id, EventType& event, const std::chrono::steady_clock::duration &timeout=std::chrono::duration_values::zero()) = 0;

    /** Blocking call which waits for the next event.
     *
     *  This methods waits for the <I>next</I> arriving event to make sure that only events arriving
     *  after entering the method are considered. Method consumes event. An old event that has been
     *  fired is ignored (in contrary to getEvent()). If method is called concurrently from several
     *  threads with the same <I>id</I>, then every call returns with the same <I>event</I> once the
     *  event fired.
     *
     *  @param id of the event activation
     *  @param event is set to the returned event if fired (Communication Object)
     *
     *  - <b>single mode</b>:
     *    <p>
     *    In single mode one misses the event if it fired before entering this member function.
     *    </p>
     *    <p>
     *    <b>Returns status code</b>:
     *    </p>
     *    <p>
     *      - SMART_OK            : event fired while waiting for the event and event is consumed
     *                              and returned
     *      - SMART_PASSIVE       : event already fired between activation and calling this member
     *                              function and is therefore missed or event has already been
     *                              consumed and can not fire again in single mode. Does not block
     *                              indefinitely and returns no valid <I>event</I>.
     *      - SMART_CANCELLED     : event not yet fired and waiting for the event has been aborted or
     *                              blocking is not allowed anymore. No valid <I>event</I> is returned.
     *      - SMART_DISCONNECTED  : got disconnected while waiting and therefore <I>event</I> not valid
     *                              and also <I>id</I> not valid any longer due to automatic deactivation.
     *      - SMART_NOTACTIVATED  : got deactivated while waiting and therefore <I>event</I> not valid and
     *                              also <I>id</I> not valid any longer.
     *      - SMART_WRONGID       : there is no activation available with this <I>id</I> and therefore
     *                              <I>event</I> not valid.
     *    </p>
     *
     *  - <b>continuous mode</b>:
     *    <p>
     *    Makes sure that only events fired after entering this member function are considered.
     *    </p>
     *    <p>
     *    <b>Returns status code</b>:
     *    </p>
     *    <p>
     *      - SMART_OK            : event fired while waiting for the event and event is consumed
     *                              and returned
     *      - SMART_CANCELLED     : waiting for the next event has been aborted or blocking is not
     *                              allowed anymore. No valid <I>event</I> is returned.
     *      - SMART_DISCONNECTED  : got disconnected while waiting and therefore <I>event</I> not valid
     *                              and also <I>id</I> not valid any longer due to automatic deactivation.
     *      - SMART_NOTACTIVATED  : got deactivated while waiting and therefore <I>event</I> not valid and
     *                              also <I>id</I> not valid any longer.
     *      - SMART_WRONGID       : there is no activation available with this <I>id</I> and therefore
     *                              <I>event</I> not valid.
     *    </p>
     */
    virtual StatusCode getNextEvent(const EventIdType &id, EventType& event, const std::chrono::steady_clock::duration &timeout=std::chrono::duration_values::zero()) = 0;
};

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTIEVENTCLIENTPATTERN_T_H_ */
