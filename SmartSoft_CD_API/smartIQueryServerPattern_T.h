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

#ifndef SMARTSOFT_INTERFACES_SMARTIQUERYSERVERPATTERN_T_H_
#define SMARTSOFT_INTERFACES_SMARTIQUERYSERVERPATTERN_T_H_

#include "smartIInputHandler_T.h"
#include "smartIServerPattern.h"
#include "smartQueryStatus.h"

namespace Smart {

// forward declaration
template<class RequestType, class AnswerType, class QIDType>
class IQueryServerPattern;

/// struct used by IQueryServerHandler and IQueryServerPattern internally
template<class RequestType, class QIDType>
struct QueryServerInputType {
	RequestType request;
	QIDType query_id;
};

/** Handler Class for QueryServer for incoming requests.
 *
 *  Used by the QueryServer to handle incoming queries.
 *  The user should implement the handleQuery() method by
 *  subclassing and providing a pointer to an IQueryServerPattern
 *  to this handler.
 */
template<class RequestType, class AnswerType, class QIDType>
class IQueryServerHandler : public IInputHandler< QueryServerInputType<RequestType,QIDType> >
{
protected:
	/// use this pointer in your derived class to call <b>"server->answer(...)"</b>
	IQueryServerPattern<RequestType,AnswerType,QIDType>* server;

	/** implements IInputHandler
	 *
	 *  This handler method delegates the call to the handleQuery handler, thereby
	 *  extracting the input attributes from the composed QueryServerInputType
	 */
	virtual void handle_input(const QueryServerInputType<RequestType,QIDType>& input) {
		this->handleQuery(input.query_id, input.request);
	}

public:
	/** Default constructor
	 *
	 * This constructor automatically registers itself to the denoted
	 * QueryServer such that all incoming query-requests within the QueryServer
	 * automatically will trigger the method handleQuery().
	 *
	 * @param server the pointer to the QueryServer whose queries need to be processed.
	 *
	 */
	IQueryServerHandler(IQueryServerPattern<RequestType,AnswerType,QIDType>* server)
	:	IInputHandler< QueryServerInputType<RequestType,QIDType> >(server)
	,	server(server)
	{  }

	/** Default destructor
	 */
	virtual ~IQueryServerHandler()
	{  }

  /** Handler method for an incoming query request.
   *
   *  This method is called by the query-server every time
   *  a new query request is received. It must be provided by the
   *  component developer to handle incoming requests. Since the
   *  method is executed by the communication thread, it must be
   *  very fast and non-blocking. Within this handler, use the
   *  provided <b>server</b> pointer to provide an answer like
   *  this: <b>"server->answer(...)"</b>.
   *
   *  Usually the request and the id will be inserted into a queue
   *  and another working thread processes the request and provides
   *  the result. The ThreadedQueryHandler decorator provides such
   *  a processing pattern.
   *
   *  @param id       id of new query
   *  @param request the request itself
   */
  virtual void handleQuery(const QIDType &id, const RequestType& request) = 0;
};


/** The server part of the Query pattern to perform two-way (request-response) communication.
 *
 *  Template parameters
 *    - <b>RequestType</b>: request class (Communication Object)
 *    - <b>AnswerType</b>: answer (reply) class (Communication Object)
 *    - <b>QIDType</b>: the QueryId that should be implemented for each middleware by subclassing IQueryId
 */
template<class RequestType, class AnswerType, class QIDType>
class IQueryServerPattern
:	public IServerPattern
,	public InputSubject< QueryServerInputType<RequestType,QIDType> >
{
public:
    /** Default constructor.
     *
     *  Note that a handler has to be supplied. Without a handler, the
     *  QueryServer could not accept a query.
     *
     *  @param component management class of the component
     *  @param service   name of the service
     */
	IQueryServerPattern(IComponent* component, const std::string& service)
	:	IServerPattern(component, service)
	{  }

    /** Destructor.
     *  Properly disconnects all service requestors in case of destruction
     *  such that all pending queries are handled correctly at client side
     *  even when the service provider disappears during pending queries.
     */
	virtual ~IQueryServerPattern() {  }

    /** Provide answer to be sent back to the requestor.
     *
     *  Member function is thread safe and thread reentrant.
     *
     *  @param id identifies the request to which the answer belongs
     *  @param answer is the reply itself.
     *
     *  @return status code:
     *    - SMART_OK                  : everything is ok and answer sent to requesting client
     *    - SMART_WRONGID             : no pending query with that <I>id</I> known
     *    - SMART_DISCONNECTED        : answer not needed anymore since client
     *                                  got disconnected meanwhile
     *    - SMART_ERROR_COMMUNICATION : communication problems
     *    - SMART_ERROR               : something went wrong
     */
    virtual StatusCode answer(const QIDType& id, const AnswerType& answer) = 0;
};

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTIQUERYSERVERPATTERN_T_H_ */
