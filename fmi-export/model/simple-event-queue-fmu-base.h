/* -*- Mode:C++; c-file-style:"bsd"; -*- *//*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation;
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// --------------------------------------------------------------
// Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.
// --------------------------------------------------------------

#ifndef _NS3_FMU_BACKEND_BASE
#define _NS3_FMU_BACKEND_BASE


#include <fstream>
#include <set>


// FMI++ includes.
#include "export/include/BackEndApplicationBase.h"


namespace Ns3FMUBackendEventQueue
{
	typedef fmi2Real TimeStamp;
	typedef fmi2Integer MessageID;
	typedef fmi2Integer* Receiver;

	struct Event {

		TimeStamp time_stamp; // Each event is associated with a timestamp.
		MessageID msg_id; // Each event is associated with a message ID (can be 0).
		bool default_event; // The FMU schedules 'default events' at regular time intervals.
		Receiver receiver; // Each message ID is associated to an output variable (can be 0).

		// Struct constructor.
		Event( TimeStamp t, MessageID m, bool d, Receiver r ) : time_stamp( t ), msg_id( m ), default_event( d ), receiver( r ) {}
	};

	// This functor defines that events are sorted in the event queue according to their timestamp.
	struct EventOrder {
		bool operator() ( const Event* e1, const Event* e2 ) const {
			return e1->time_stamp < e2->time_stamp;
		}
	};

	// This is the definition of the event queue.
	typedef std::set<Event*, EventOrder> EventQueue;
}


// To implement an application that uses the backend/frontend mechanism, inherit from class 'BackEndApplicationBase'.
class SimpleEventQueueFMUBase : public BackEndApplicationBase
{
public:

	virtual void initializeScalarVariables();
	virtual int initializeBackEnd( int argc, const char* argv[] );
	virtual void initializeParameterValues();
	virtual int doStep( const fmi2Real& syncTime, const fmi2Real& lastSyncTime );

	// This function defines the inputs/outputs/parameters of the
	// ns-3 simulation (to be implemented by inheriting application).
	virtual void initializeSimulation() = 0;

	// This function runs an ns-3 simulation (to be implemented by inheriting application).
	virtual void runSimulation( const double& sync_time ) = 0;

protected:

	// This function adds new events to the event queue.
	void addNewEventForMessage( const Ns3FMUBackendEventQueue::TimeStamp& msg_receive_time,
	const Ns3FMUBackendEventQueue::MessageID& msg_id,
	const Ns3FMUBackendEventQueue::Receiver& receiver );

	// Send debug message.
	void debug( const std::string& msg ) const;

private:

	// This function resets all integer inputs (i.e., input messages).
	void resetIntegerInputs();

	// This function resets all integer outputs (i.e., output messages).
	void resetIntegerOutputs();

	//
	// Internal class members.
	//

	// Output variable indicating the time of the next scheduled event.
	fmi2Real next_event_time;

	// Default step size (parameter).
	fmi2Real default_event_step_size;

	// Random generator seed (parameter).
	fmi2Integer random_seed;

	// Event queue.
	Ns3FMUBackendEventQueue::EventQueue event_queue_;
	Ns3FMUBackendEventQueue::EventQueue::iterator current_event_;

	// Output file stream (for debugging).
	std::ofstream* debug_;
};


#endif // _NS3_FMU_BACKEND_BASE
