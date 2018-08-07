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

#ifndef SMARTSOFT_INTERFACES_SMARTSTATUSCODE_H_
#define SMARTSOFT_INTERFACES_SMARTSTATUSCODE_H_

#include <string>

namespace Smart {

/** Status Code
 *
 *  SmartSoft so far doesn't use exceptions generally, only in constructors.
 *  It uses return values for status & error handling of methods (see
 *  communication pattern classes for more details).
 *
 */
enum StatusCode {
	/// Operation completed successfully
	SMART_OK              = 0,
	/// no new data is available (yet) e.g. in PushClient
	SMART_NODATA,
	/// some blocking wait has been cancelled
	SMART_CANCELLED,
	/// e.g. used for Push subscription state
	SMART_UNSUBSCRIBED,
	/// e.g. used for QueryId(s)
	SMART_WRONGID,
	/// e.g. used by StatePattern
	SMART_UNKNOWNSTATE,
	/// indicates deactivated service
	SMART_NOTACTIVATED,
	/// indicates activated service
	SMART_ACTIVATED,
	/// generic not-allowed status code (with context-specific meaning)
	SMART_NOTALLOWED,

	SMART_ACTIVE,

	SMART_PASSIVE,
	/// used to indicate client disconnected state
	SMART_DISCONNECTED,
	/// used to indicate incompatible service during client connection
	SMART_INCOMPATIBLESERVICE,
	/// used to indicate duplicate port usage (with the same server+service name)
	SMART_PORTALREADYUSED,
	/// used to indicate unknown ports
	SMART_UNKNOWNPORT,
	/// used to indicate unknown service-names
	SMART_SERVICEUNAVAILABLE,
	/// used to indicate unknown component names
	SMART_UNKNOWNCOMPONENT,
	/// generic timeout status code
	SMART_TIMEOUT,
	///value=256 (all enumeration values <= SMART_STATUS indicate regular status codes)
	SMART_STATUS          = 256,

	///value=512 (all enumeration values >= SMART_ERROR indicate errors)
	SMART_ERROR           = 512,
	/// generic communication error code
	SMART_ERROR_COMMUNICATION,
	/// generic error code for unspecified errors
	SMART_ERROR_UNKNOWN,
	/// something went badly wrong
	SMART_ERROR_FATAL
};

/** global function used to convert StatusCode into ASCII representation.
 *
 *  @param code StatusCode
 */
inline std::string StatusCodeConversion(const StatusCode &code)
{
	  std::string r;
	  if (code <= SMART_STATUS) {
	    switch (code) {
	      case SMART_OK:
	        r = "STATUS: OK";
	        break;
	      case SMART_NODATA:
	        r = "STATUS: NODATA";
	        break;
	      case SMART_CANCELLED:
	        r = "STATUS: CANCELLED";
	        break;
	      case SMART_UNSUBSCRIBED:
	        r = "STATUS: UNSUBSCRIBED";
	        break;
	      case SMART_WRONGID:
	        r = "STATUS: WRONGID";
	        break;
	      case SMART_UNKNOWNSTATE:
	        r = "STATUS: UNKNOWNSTATE";
	        break;
	      case SMART_NOTACTIVATED:
	        r = "STATUS: NOTACTIVATED";
	        break;
	      case SMART_ACTIVATED:
	        r = "STATUS: ACTIVATED";
	        break;
	      case SMART_NOTALLOWED:
	        r = "STATUS: NOTALLOWED";
	        break;
	      case SMART_ACTIVE:
	        r = "STATUS: ACTIVE";
	        break;
	      case SMART_PASSIVE:
	        r = "STATUS: PASSIVE";
	        break;
	      case SMART_DISCONNECTED:
	        r = "STATUS: DISCONNECTED";
	        break;
	      case SMART_INCOMPATIBLESERVICE:
	        r = "STATUS: INCOMPATIBLESERVICE";
	        break;
	      case SMART_PORTALREADYUSED:
	        r = "STATUS: PORTALREADYUSED";
	        break;
	      case SMART_UNKNOWNPORT:
	        r = "STATUS: UNKNOWNPORT";
	        break;
	      case SMART_SERVICEUNAVAILABLE:
	        r = "STATUS: SERVICEUNAVAILABLE";
	        break;
	      case SMART_UNKNOWNCOMPONENT:
	        r = "STATUS: UNKNOWNCOMPONENT";
	        break;
	      case SMART_TIMEOUT:
	        r = "STATUS: TIMEOUT";
	        break;
	      default:
	        r = "STATUS: unknown status code";
	        break;
	    }
	  } else if (code >= SMART_ERROR) {
	    switch (code) {
	      case SMART_ERROR_COMMUNICATION:
	        r = "ERROR: COMMUNICATION";
	        break;
	      case SMART_ERROR_UNKNOWN:
	        r = "ERROR: UNKNOWN";
	        break;
	      case SMART_ERROR_FATAL:
	        r = "ERROR: FATAL";
	        break;
	      default:
	        r = "ERROR: unknown error code";
	        break;
	    }
	  } else {
	    r = "unknown status / error code";
	  }

	  return r;
}

/** ostream operator for StatusCode
 *
 * This operator prints a string representation of the StatusCode to ostream.
 *
 */
inline std::ostream &operator<<(std::ostream &os, const StatusCode &status)
{
	os << StatusCodeConversion(status);
	return os;
}

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTSTATUSCODE_H_ */
