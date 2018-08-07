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

#ifndef SMARTSOFT_INTERFACES_SMARTIINPUTHANDLER_H_
#define SMARTSOFT_INTERFACES_SMARTIINPUTHANDLER_H_

#include <map>

// C++11 mutex
#include <mutex>

#include "smartPrescaleManager.h"

namespace Smart {

// forward declaration
template <class InputType>
class InputSubject;

// forward declaration
template <class InputType>
class IActiveQueueInputHandlerDecorator;

/** This template class implements the <b>Observer</b> part of the Observer design pattern for
 *  implementing a generic data-input-handler (i.e. input-data-upcall handler).
 *
 *  This class implements the <b>Observer</b> part of the Observer design pattern.
 *  All the communication-patterns that receive input-data can implement the counterpart
 *  IInputSubject which can be used by this handler for receiving the input-data.
 *  Therefore the abstract method handle_input() needs to be implemented in derived classes.
 */
template <class InputType>
class IInputHandler {
	/// allows acessing protected members
	template <class InnerType>
	friend class IActiveQueueInputHandlerDecorator;
protected:
	/// this is the subject-pointer (can be used in derived classes)
	InputSubject<InputType> *subject;

	/** calls subject->attach(this);
	 *
	 *  This method encapsulates the <b>attachment</b> of itself to the
	 *  IInputSubject. This is useful as this method can be called
	 *  from within the IActiveQueueInputHandlerDecorator.
	 *
	 *  @param prescale optionally divides the input-update frequency by this factor
	 */
	void attach_self(const unsigned int &prescale=1);

	/** calls subject->detach(this);
	 *
	 *  This method encapsulates the <b>detachment</b> of itself from the
	 *  IInputSubject. This is useful as this method can be called
	 *  from within the IActiveQueueInputHandlerDecorator.
	 */
	void detach_self();

public:
	/** The default constructor.
	 *
	 * This constructor will call <b>subject->attach(this)</b> to start observing the given subject.
	 *
	 * @param subject the subject (also called model) that this handler is going to observe
	 * @param prescaleFactor optionally divides the input-update frequency by this factor
	 */
	IInputHandler(InputSubject<InputType> *subject, const unsigned int &prescaleFactor=1)
	:	subject(subject)
	{
		this->attach_self(prescaleFactor);
	}

	/** The default destructor.
	 *
	 * This destructor will call <b>subject->detach(this)</b> to stop observing the given subject.
	 */
	virtual ~IInputHandler()
	{
		this->detach_self();
	}

	/** This is the main input-handler method that will be automatically called from the given subject
	 *  each time the subject receives input-data.
	 *
	 *  This method should be implemented in derived classes to provide a data-handling procedure.
	 *
	 *  @param input the input-data reference
	 */
	virtual void handle_input(const InputType& input) = 0;
};


/** This template class implements the <b>Subject</b> (also called Model) part of the Observer design pattern for
 * implementing delegation of handling incoming data.
 *
 * All Communication-Patterns that internally receive input data (e.g. IPushClient, or IQueryServer) can implement
 * this interface to allow the definition of upcall-handlers for handling input-data.
 */
template <class InputType>
class InputSubject {
	/// allows calling protected attach() and detach() methods
	friend class IInputHandler<InputType>;
private:
	std::mutex observers_mutex;

	// map of observers with individual prescale management
	std::map<IInputHandler<InputType>*, PrescaleManager> observers;
protected:
	/** Attach an IInputHandler<InputType> instance.
	 *
	 * This method should be called from within the constructor
	 * of an IInputHandler instance. This is possible because
	 * the IInputHandler is defined as a <i>friend class</i>
	 * of IInputSubject.
	 *
	 * @param handler the InputHandler pointer
	 * @param prescaleFactor divides the input-update frequency by this factor
	 */
	virtual void attach(IInputHandler<InputType> *handler, const unsigned int &prescaleFactor=1)
	{
		std::unique_lock<std::mutex> lock (observers_mutex);
		observers[handler] =  prescaleFactor;
	}

	/** Detach an IInputHandler<InputType> instance.
	 *
	 * This method should be called from within the destructor
	 * of an IInputHandler instance. This is possible because
	 * the IInputHandler is defined as a <i>friend class</i>
	 * of IInputSubject.
	 *
	 * @param handler the InputHandler pointer
	 */
	virtual void detach(IInputHandler<InputType> *handler)
	{
		std::unique_lock<std::mutex> lock (observers_mutex);
		observers.erase(handler);
	}

	/** Notifies all attached IInputHandler instances about incoming data.
	 *
	 *  An instance of IInputSubject should call this method each time new
	 *  data arrives. This method then delegates the input-data-handling
	 *  to all attached IInputHandler instances.
	 *
	 *  @param input the input-data reference
	 */
	virtual bool notify_input(const InputType& input)
	{
		std::unique_lock<std::mutex> lock (observers_mutex);
		for(auto it=observers.begin(); it!=observers.end(); it++) {
			if(it->second.isUpdateDue() == true) {
				it->first->handle_input(input);
			}
		}
		return !observers.empty();
	}

public:
	/** Default constructor
	 */
	InputSubject()
	{  }
	/** Default destructor
	 */
	virtual ~InputSubject()
	{  }
};


///////////////////////////////////////////////////////////
// default implementation of IInputHandler attachment
// and detachment methods
//////////////////////////////////////////////////////////
template <class InputType>
inline void IInputHandler<InputType>::attach_self(const unsigned int &prescale)
{
	subject->attach(this, prescale);
}

template <class InputType>
inline void IInputHandler<InputType>::detach_self()
{
	subject->detach(this);
}

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTIINPUTHANDLER_H_ */
