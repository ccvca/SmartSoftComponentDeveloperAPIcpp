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

#ifndef SMARTSOFT_INTERFACES_SMARTQUERYSTATUS_H_
#define SMARTSOFT_INTERFACES_SMARTQUERYSTATUS_H_

#include <string>

namespace Smart {

	/** QueryStatus
	 *
	 *  QueryStatus used to communicate the current QueryStatus from a QueryServer to a QueryClient
	 *  for each individual query-request. A query-request can be either still <b>pending</b> if no
	 *  answer has been yet calculated. If an answer for a specific query-request has been calculated
	 *  by the QueryServer but this answer has not yet been fetched by the requesting QueryClient
	 *  then this query-request is marked <b>validanswer</b>. A query-request can become invalidated
	 *  if the QueryClient disconnects in the meantime.
	 */
	enum QueryStatus {
		/// this indicates a pending query-request (i.e. not yet answered/consumed query-requests)
		QUERY_PENDING      = 0,
		/// this indicates a calculated answer that has not yet been consumed
		QUERY_VALIDANSWER  = 1,
		/// this indicates a query-request that became invalid due to a closed connection
		QUERY_DISCONNECTED = 2,
		/// this indicates a wrong id of a query-request (i.e. a request that does no longer exists)
		QUERY_WRONGID      = 3
	};

	/** global function used to convert a QueryStatus into ASCII representation.
	 *
	 *  @param qs QueryStatus
	 */
	inline std::string QueryStatusString(QueryStatus qs)
	{
		if(QUERY_PENDING == qs) return "QUERY_PENDING";
		else if(QUERY_VALIDANSWER == qs) return "QUERY_VALIDANSWER";
		else if(QUERY_DISCONNECTED == qs) return "QUERY_DISCONNECTED";
		else if(QUERY_WRONGID == qs) return "QUERY_WRONGID";
		else return "NA";
	}

} // end namespace Smart

#endif /* SMARTSOFT_INTERFACES_SMARTQUERYSTATUS_H_ */
