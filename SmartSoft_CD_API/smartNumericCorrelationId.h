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

#ifndef SMARTNUMERICCORRELATIONID_H_
#define SMARTNUMERICCORRELATIONID_H_

#include "smartICorrelationId.h"

namespace Smart {

/** A simple ID realization using a size_t type internally
 *
 * This is the simplest-possible ID realization based on a simple numeric value.
 * The drawback of this "oversimplified" ID realization is that it is likely not unique in a network
 * as this ID realization does not encode the HOST, Time, Hash, etc. which is typically the
 * case in more advanced ID implementations.
 */
class NumericCorrelationId : public Smart::ICorrelationId {
private:
	size_t id;
public:
	NumericCorrelationId(const size_t &id=0)
	:	id(id)
	{  }
	virtual ~NumericCorrelationId() = default;

	operator size_t() const {
		return id;
	}

	NumericCorrelationId operator++(int) {
		id++;
		return *this;
	}

	virtual std::string to_string() const override final {
		return std::to_string(id);
	}

protected:
	virtual bool less_than(const ICorrelationId *other) const override final {
		auto sother = dynamic_cast<const NumericCorrelationId*>(other);
		return id < sother->id;
	}
	virtual bool equals_to(const ICorrelationId *other) const override final {
		auto sother = dynamic_cast<const NumericCorrelationId*>(other);
		return id == sother->id;
	}
};

} /* namespace Smart */

#endif /* SMARTNUMERICCORRELATIONID_H_ */
