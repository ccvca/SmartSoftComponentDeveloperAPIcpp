//===================================================================================
//
//  Copyright (C) 2019 Alex Lotz, Dennis Stampfer, Matthias Lutz, Christian Schlegel
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

#ifndef SMARTICORRELATIONID_H_
#define SMARTICORRELATIONID_H_

#include <string>
#include <ostream>
#include <memory>

namespace Smart {

/**
 * This pure virtual interface can be implemented using middleware-specific features
 * to correlate e.g. answers to related requests in a request-response communication
 * (e.g. for the Query Pattern).
 */
class ICorrelationId {
public:
	virtual ~ICorrelationId() = default;

	// comparison operators delegate to the implementation provided in derived classes
	bool operator<(const ICorrelationId &other) const {
		return less_than(&other);
	}
	bool operator==(const ICorrelationId &other) const {
		return equals_to(&other);
	}

	// this is the only method that has to be implemented in derived implementations
	virtual std::string to_string() const = 0;

protected:
	// use dynamic_cast<const YourDerivedType*>(other)
	// to implement these comparison functions in derived classes
	virtual bool less_than(const ICorrelationId *other) const = 0;
	virtual bool equals_to(const ICorrelationId *other) const = 0;
};

// this pointer type can be used as the implementation-independent shared pointer
using CorrelationIdPtr = std::shared_ptr<ICorrelationId>;

} /* namespace Smart */

// the pointer content can be printed out to ostream
inline std::ostream& operator<<(std::ostream& out, const Smart::CorrelationIdPtr& cid)
{
	if(cid) {
		out << cid->to_string();
	}
    return out;
}

#endif /* SMARTICORRELATIONID_H_ */
