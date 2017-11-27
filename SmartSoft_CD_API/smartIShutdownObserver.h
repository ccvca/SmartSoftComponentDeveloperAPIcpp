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

#ifndef SMARTSOFT_INTERFACES_SMARTISHUTDOWNOBSERVER_H_
#define SMARTSOFT_INTERFACES_SMARTISHUTDOWNOBSERVER_H_

#include <list>
// C++11 mutex
#include <mutex>


namespace Smart {

// forward declaration
class ShutdownSubject;


/** This class implements the <b>Observer</b> part of the Observer design pattern for
 * implementing a uniform shutdown procedure for all component's resources.
 *
 * An IComponent implements the counter-part IShutdownSubject interface that will trigger all
 * attached IShutdownObserver instances just before the IComponent finally shuts down.
 * Each communication pattern (clients and servers) attached to an IComponent should implement
 * IShutdownObserver interface (i.e. the on_shutdown() method) thus providing individual
 * cleanup strategies.
 */
class IShutdownObserver {
private:
	/// the internal pointer to the Subject (or model) of the Observer design pattern
	ShutdownSubject *subject;
protected:

	/** call this method from within the constructor of derived classes
	 *
	 *  The IShutdownObserver interface can be derived pure virtual which means that
	 *  in these cases the default empty constructor will be called.
	 *  For the pure virtual derivation, the constructor of the derived class
	 *  should call this method manually to ensure proper registration of the
	 *  IShutdownObserver.
	 *
	 *  @param subject the subject (i.e. model) of the Observer design pattern
	 */
	void attach_self_to(ShutdownSubject *subject);

	/** this method is called within the destructor
	 *
	 *  This method is called from within the destructor of this class if the
	 *  internal subject pointer has been set-up before using the attach_self_to() method.
	 *
	 *  @param subject the subject (i.e. model) of the Observer design pattern
	 */
	void detach_self_from(ShutdownSubject *subject);
public:
	/** The default constructor.
	 *
	 * This constructor will call <b>subject->attach(this)</b> to start observing the given subject.
	 *
	 * @param subject the subject (also called model) that this Observer is going to observe
	 */
	IShutdownObserver(ShutdownSubject *subject = 0)
	:	subject(subject)
	{
		if(subject != 0) {
			attach_self_to(subject);
		}
	}

	/** The default destructor.
	 *
	 * This destructor will call <b>subject->detach(this)</b> to stop observing the given subject.
	 */
	virtual ~IShutdownObserver()
	{
		if(this->subject != 0) {
			this->detach_self_from(subject);
		}
	}

	/** This is the main update method that will be automatically called from the given subject
	 *  each time the subject undergoes a notable change.
	 *
	 *  This method should be implemented in derived classes to provide an individual shutdown
	 *  (i.e. cleanup) strategy/procedure.
	 */
	virtual void on_shutdown() = 0;
};

/** This class implements the <b>Subject</b> (also called Model) part of the Observer design pattern for
 * implementing a uniform shutdown procedure for all component's resources.
 *
 * An IComponent implements this interface to trigger all attached IShutdownObserver instances
 * just before the IComponent finally shuts down.
 * Each communication pattern (clients and servers) and all user-defined Tasks attached to an IComponent
 * should implement the counter-part IShutdownObserver interface (i.e. the on_shutdown() method) thus
 * providing individual cleanup procedures/strategies.
 */
class ShutdownSubject {
	/// allows calling protected attach() and detach() methods
	friend class IShutdownObserver;
private:
	std::mutex observers_mutex;
	// implement Observer design pattern
	std::list<IShutdownObserver*> observers;
protected:

	/** Attach an IShutdownObserver instance.
	 *
	 * This method should be called from within the constructor
	 * of an IShutdownObserver instance. This is possible because
	 * the IShutdownObserver is defined as a <i>friend class</i>
	 * of IShutdownSubject.
	 */
	virtual void attach(IShutdownObserver* observer){
		std::unique_lock<std::mutex> lock (observers_mutex);
		observers.push_back(observer);
	}

	/** Detach an IShutdownObserver instance.
	 *
	 * This method should be called from within the destructor
	 * of an IShutdownObserver instance. This is possible because
	 * the IShutdownObserver is defined as a <i>friend class</i>
	 * of IShutdownSubject.
	 */
	virtual void detach(IShutdownObserver* observer) {
		std::unique_lock<std::mutex> lock (observers_mutex);
		observers.remove(observer);
	}

	/** Notifies all attached IShutdownObserver instances about an upcoming shutdown.
	 *
	 *  An instance of IComponent should call this method just before the component
	 *  itself cleans-up its own internal resources. In this way, all the attached
	 *  entities (such as client- and server-ports, as well as tasks) can clean-up
	 *  their individual internal resources before the component invalidates its own
	 *  internal resources. This effectively prevents nullpointer exceptions.
	 */
	virtual void notify_sutdown() {
		std::unique_lock<std::mutex> lock (observers_mutex);
		for(auto it=observers.begin(); it!=observers.end(); it++) {
			(*it)->on_shutdown();
		}
	}

public:
	/** Default constructor
	 */
	ShutdownSubject()
	{  }

	/** Default destructor
	 */
	virtual ~ShutdownSubject()
	{  }
};


///////////////////////////////////////////////////////////
// default implementation of IShutdownObserver init
// and fini methods
//////////////////////////////////////////////////////////
inline void IShutdownObserver::attach_self_to(ShutdownSubject *subject)
{
	this->subject = subject;
	this->subject->attach(this);
}
inline void IShutdownObserver::detach_self_from(ShutdownSubject *subject)
{
	this->subject->detach(this);
}

} /* namespace Smart */

#endif /* SMARTSOFT_INTERFACES_SMARTISHUTDOWNOBSERVER_H_ */
