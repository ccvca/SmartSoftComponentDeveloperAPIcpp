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

#ifndef SMARTSOFT_INTERFACES_SMARTISENDSERVERPATTERN_T_H_
#define SMARTSOFT_INTERFACES_SMARTISENDSERVERPATTERN_T_H_

#include "smartIInputHandler_T.h"
#include "smartIServerPattern.h"

namespace Smart {

// forward declaration
template <class DataType>
class ISendServerPattern;

/** Handler Class for SendServer for incoming commands.
 *
 *  Used by the SendServer to handle incoming commands.
 *  The user should provide the handleSend() method by
 *  subclassing and register an instance of this handler
 *  class with the SendServer.
 */
template <class DataType>
class ISendServerHandler : public IInputHandler<DataType> {
protected:
	/** implements IInputHandler
	 *
	 *  This handler method delegates the call to the handleSend() method.
	 *
	 *  @param input the input data-object
	 */
	virtual void handle_input(const DataType& input) {
		this->handleSend(input);
	}

public:
  /** Default constructor.
   *
   *  The constructor requires the server pointer to register itself
   *  for handling incoming send commands.
   *
   *  @param server the pointer to the ISendServerPattern instance
   *
   */
  ISendServerHandler(ISendServerPattern<DataType> *server)
  : IInputHandler<DataType>(server)
  { }

  /// Default destructor.
  virtual ~ISendServerHandler()
  { }

  /** Handler method for an incoming command.
   *
   *  This method is called by the ISendServerPattern every time
   *  a new data is received. It must be provided by the component
   *  developer to handle incoming data. Since the method is
   *  executed by the communication thread, it must be very fast
   *  and non-blocking.
   *
   *  Usually the data will be inserted into a queue and another
   *  working thread processes the command. The IActiveQueueInputHandlerDecorator
   *  provides such a processing pattern.
   *
   *  @param data communicated DataType object (Communication Object)
   */
  virtual void handleSend(const DataType& data) = 0;
};


/** Server part of the one-way, n->1 <b>Send</b> communication pattern.
 *
 *  Clients can send a specified DataType object to a server. The server
 *  propagates handling of the input object to a registered ISendServerHandler
 *
 *  If the
 *  send() method succeeds, this means that the sent object actually
 *  arrived at the server, but the server has the freedom to process
 *  this object later or throw it away altogether, depending on the
 *  implemented server semantics.
 *
 *  Template parameters
 *    - <b>DataType</b>: Communicated DataType (Communication Object)
 */
/** Server part of one-way communication pattern.
 *
 *  Template parameters:
 *    - <b>C</b>: command class (Communication Object)
 *
 *  Demonstrated in <a href="/drupal/?q=node/51#sixth-example">sixth example</a>
 */
template <class DataType>
class ISendServerPattern
:	public IServerPattern
,	public InputSubject<DataType>
{
public:
    /** Default constructor.
     *
     *  Note that a handler has to be supplied. Without a handler, the
     *  SendServer could not accept a send command.
     *
     *  @param component management class of the component
     *  @param service   name of the service
     */
	ISendServerPattern(IComponent* component, const std::string& service)
	:	IServerPattern(component, service)
	{ }

	/** Destructor.
	 *  Properly disconnects all service requestors in case of destruction
	 *  such that all pending sends are handled correctly at client side
	 *  even when the service provider disappears during pending sends.
	 */
	virtual ~ISendServerPattern()
	{ }
};

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTISENDSERVERPATTERN_T_H_ */
