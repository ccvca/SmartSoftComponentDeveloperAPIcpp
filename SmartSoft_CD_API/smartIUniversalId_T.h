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

#ifndef SMARTSOFT_INTERFACES_SMARTIUNIVERSALID_T_H_
#define SMARTSOFT_INTERFACES_SMARTIUNIVERSALID_T_H_

#include <string>

namespace Smart {

/** A template class providing an interface for a universal identifier.
 *
 *  Universal identifiers are typically platform-specific and need to be
 *  implemented for each middleware individually. This template class
 *  defines the required interface for a wrapper class of an unique identifier.
 *
 *   *  Template parameters
 *    - <b>IdType</b>: The middleware (or platform) specific identifier representation
 */
template <class IdType>
class IUniversalId {
protected:
	/// internal (platform-specific) identifier
	IdType _id;

	/** generate a new identifier
	 *
	 *  Provide a platform-specific implementation of generating a new identifier.
	 *  Typical implementations are incrementing an identifier or generating a
	 *  random number. This method is only used internally.
	 */
	virtual IdType generateNewId() = 0;

	/** convert the plaform-specific identifier into a std::string
	 *
	 *  Provide a platform-specific implementation of converting
	 *  the identifier into a string. This conversion method
	 *  should not change the internal value of this class.
	 *
	 *  @param id the platform-specific id representation
	 *
	 *  @returns the string representation of the id
	 */
	virtual std::string to_string(const IdType& id) const = 0;

	/** convert an std::string into the platform-specific id representation.
	 *
	 *  Provide a platform-specific implementation of converting
	 *  string into the platform-specific id representation.
	 *  This conversion method should not change the internal value
	 *  of this class.
	 *
	 *  @param s_id the platform-specific id representation
	 *
	 *  @returns the string representation of the id
	 */
	virtual IdType from_string(const std::string& s_id) const = 0;

public:
	/** Default constructor
	 *
	 * 	In your derived constructor, please call <b>_id = generateNewId();</b>.
	 * 	This ensures that each new instance of IUniversalId gets a new id.
	 */
	IUniversalId()
	{
		// generate new id in derived classes
		//_id = generateNewId();
	}

	/** String constructor.
	 *
	 *  This constructor used the string representation to create a new id.
	 *  Please call <b>_id = from_string(s_id);</b> in your derived constructor
	 *  to convert the string into the id.
	 *
	 *  @param s_id string representation of the id
	 */
	IUniversalId(const std::string& s_id)
	{
		// convert from string in derived classes
		//_id = from_string(s_id);
	}

	/// Default destructor
	virtual ~IUniversalId() = default;

	/// conversion operator to std::string
	virtual operator std::string() const {
		return to_string(_id);
	}
	/// assignment operator from std::string
	virtual IUniversalId& operator =(const std::string& id) {
		_id = from_string(id);
		return *this;
	}

	/// equality comparison operator
	virtual bool operator ==(const IUniversalId& id) const {
		return _id == id._id;
	}
	/// inequality comparison operator
	virtual bool operator !=(const IUniversalId& id) const {
		return _id != id._id;
	}
	/// smaller comparison operator
	virtual bool operator <(const IUniversalId& id) const {
		return _id < id._id;
	}
	/// greater comparison operator
	virtual bool operator >(const IUniversalId& id) const {
		return _id > id._id;
	}
	/// smaller-or-equals comparison operator
	virtual bool operator <=(const IUniversalId& id) const {
		return _id <= id._id;
	}
	/// greater-or-equals comparison operator
	virtual bool operator >=(const IUniversalId& id) const {
		return _id >= id._id;
	}
};

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTIUNIVERSALID_T_H_ */
