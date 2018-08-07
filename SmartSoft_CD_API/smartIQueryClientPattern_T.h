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

#ifndef SMARTSOFT_INTERFACES_SMARTIQUERYCLIENTPATTERN_T_H_
#define SMARTSOFT_INTERFACES_SMARTIQUERYCLIENTPATTERN_T_H_

#include "smartIClientPattern.h"

namespace Smart {

/** The client part of Query pattern to perform two-way (request-response) communication.
 *
 *  Template parameters
 *    - <b>RequestType</b>: request class (Communication Object)
 *    - <b>AnswerType</b>: answer (reply) class (Communication Object)
 *    - <b>QIDType</b>: the QueryId type that encapsulates the middleware-specific unique IDs
 */
template<class RequestType, class AnswerType, class QIDType>
class IQueryClientPattern : public IClientPattern {
public:
    /** Constructor (not wired with any service provider).
     *
     *  connect() / disconnect() can always be used to change the connection
     *  status of the instance. Instance is not connected to a service provider.
     *
     * @param component  the management class of the component
     */
	IQueryClientPattern(IComponent* component)
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
	IQueryClientPattern(IComponent* component, const std::string& server, const std::string& service)
	:	IClientPattern(component, server, service)
	{  }

    /** Destructor.
     *  The destructor calls disconnect() and therefore properly cleans up
     *  every pending query and removes the instance from the set of wireable ports.
     */
    virtual ~IQueryClientPattern() {  }

    /** Blocking Query.
     *
     *  Perform a blocking query and return only when the query answer
     *  is available. Member function is thread safe and thread reentrant.
     *
     *  @param request send this request to the server (Communication Object)
     *  @param answer  returned answer from the server (Communication Object)
     *
     *  @return status code:
     *    - SMART_OK                  : everything is ok and <I>answer</I> contains answer
     *    - SMART_CANCELLED           : blocking is not allowed or is not allowed anymore and therefore
     *                                  pending query is aborted, answer is lost and <I>answer</I>
     *                                  contains no valid answer.
     *    - SMART_DISCONNECTED        : the client is either disconnected and no query
     *                                  can be made or it got disconnected and a pending
     *                                  query is aborted without answer. In both cases,
     *                                  <I>answer</I> is not valid.
     *    - SMART_ERROR_COMMUNICATION : communication problems, <I>answer</I> is not valid.
     *    - SMART_ERROR               : something went wrong, <I>answer</I> is not valid.
     */
    virtual StatusCode query(const RequestType& request, AnswerType& answer) = 0;

    /** Asynchronous Query.
     *
     *  Perform a query and receive the answer later, returns immediately.
     *  Member function is thread safe and reentrant.
     *
     *  @param request send this request to the server (Communication Object)
     *  @param id      is set to the identifier which is later used to receive
     *                 the reply to this request
     *
     *  @return status code:
     *    - SMART_OK                  : everything is ok and <I>id</I> contains query identifier
     *                                  used to either fetch or discard the answer.
     *    - SMART_DISCONNECTED        : request is rejected since client is not connected to a server
     *                                  and therefore <I>id</I> is not a valid identifier.
     *    - SMART_ERROR_COMMUNICATION : communication problems, <I>id</I> is not valid.
     *    - SMART_ERROR               : something went wrong, <I>id</I> is not valid.
     */
    virtual StatusCode queryRequest(const RequestType& request, QIDType& id) = 0;

    /** Check if answer is available.
     *
     *  Non-blocking call to fetch the answer belonging to the given identifier.
     *  Returns immediately. Member function is thread safe and reentrant.
     *
     *  @warning
     *    It is not allowed to call queryReceive(), queryReceiveWait() or queryDiscard() concurrently
     *    with the <I>same</I> query id (which is not a restriction since it makes no sense !)
     *
     *  @param id      provides the identifier of the query
     *  @param answer  is set to the answer returned from the server if it was available
     *
     *  @return status code:
     *    - SMART_OK           : everything is ok and <I>answer</I> contains the answer
     *    - SMART_WRONGID      : no pending query with this identifier available, therefore no valid
     *                           <I>answer</I> returned.
     *    - SMART_NODATA       : answer not yet available, therefore try again later. The identifier <I>id</I>
     *                           keeps valid, but <I>answer</I> contains no valid answer.
     *    - SMART_DISCONNECTED : the answer belonging to the <I>id</I> can not be received
     *                           anymore since the client got disconnected. <I>id</I> is
     *                           not valid any longer and <I>answer</I> contains no valid answer.
     *    - SMART_ERROR        : something went wrong, <I>answer</I> contains no answer and <I>id</I> is
     *                           not valid any longer.
     *
     */
    virtual StatusCode queryReceive(const QIDType& id, AnswerType& answer) = 0;

    /** Wait for reply.
     *
     *  Blocking call to fetch the answer belonging to the given identifier. Waits until
     *  the answer is received.
     *
     *  @warning
     *    It is not allowed to call queryReceive(), queryReceiveWait() or queryDiscard() concurrently
     *    with the <I>same</I> query id (which is not a restriction since it makes no sense !)
     *
     *  @param id       provides the identifier of the query
     *  @param answer   is set to the answer returned from the server if it was available
     *
     *  @return status code:
     *    - SMART_OK           : everything is ok and <I>answer</I> contains the answer
     *    - SMART_WRONGID      : no pending query with this identifier available, therefore no
     *                           valid <I>answer</I> returned.
     *    - SMART_CANCELLED    : blocking call is not allowed or is not allowed anymore and therefore
     *                           blocking call is aborted and no valid <I>answer</I> is returned. The
     *                           query identifier <I>id</I> keeps valid and one can either again call
     *                           queryReceive(), queryReceiveWait() or discard the answer by calling
     *                           queryDiscard().
     *    - SMART_DISCONNECTED : blocking call is aborted and the answer belonging to <I>id</I> can not
     *                           be received anymore since client got disconnected. <I>id</I> is not valid
     *                           any longer and <I>answer</I> contains no valid answer.
     *    - SMART_ERROR        : something went wrong, <I>answer</I> contains no answer and <I>id</I> is
     *                           not valid any longer.
     *
     */
    virtual StatusCode queryReceiveWait(const QIDType& id, AnswerType& answer, const std::chrono::steady_clock::duration &timeout=std::chrono::steady_clock::duration::zero()) = 0;

    /** Discard the pending answer with the identifier <I>id</I>
     *
     *  Call this member function if you do not want to get the answer of a request anymore which
     *  was invoked by queryRequest(). This member function invalidates the identifier <I>id</I>.
     *
     *  @warning
     *    This member function does NOT abort blocking calls ! This is done by the blocking() member
     *    function. It has to be called if you have not yet received an answer and the identifier is
     *    still valid, for example due to a CANCELLED return value, and you don't want to get the
     *    answer anymore.
     *
     *  @warning
     *    It is not allowed to call queryReceive(), queryReceiveWait() or queryDiscard() concurrently
     *    with the <I>same</I> query id (which is not a restriction since it makes no sense !)
     *
     *  @param id  provides the identifier of the query
     *
     *  @return status code:
     *    - SMART_OK           : everything is ok and query with the identifier <I>id</I> discarded.
     *    - SMART_WRONGID      : no pending query with this identifier.
     *    - SMART_ERROR        : something went wrong, <I>id</I> not valid any longer.
     *
     */
    virtual StatusCode queryDiscard(const QIDType& id) = 0;
};

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTIQUERYCLIENTPATTERN_T_H_ */
