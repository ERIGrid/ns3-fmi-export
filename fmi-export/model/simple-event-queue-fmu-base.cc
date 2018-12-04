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

#include <string>
#include <sstream>
#include <cmath>
#include <limits>
#include <random>
#include <algorithm>


// ns-3 includes.
#include "ns3/core-module.h"

#include "simple-event-queue-fmu-base.h"

// FMI++ includes.
#include "export/include/BackEndApplicationBase.h"


using namespace ns3;
using namespace Ns3FMUBackendEventQueue;


// This function initializes the backend's scalar variables (parameters, inputs, outputs),
// which have to be class member variables (or global variables).
// Only calls to 'addRealInput(...)', 'addRealOutput(...)', etc. are allowed.
void
SimpleEventQueueFMUBase::initializeScalarVariables()
{
	// Define FMI real output variable (used by this backend implementation
	// to tell the timestamp of the next event in the event queue.
	addRealOutput( next_event_time );

	// Default step size (parameter).
	addRealParameter( default_event_step_size );

	// Random generator seed (parameter).
	addIntegerParameter( random_seed );

	// Initialize the user-defined FMI inputs/outputs/parameters.
	initializeSimulation();
}


void
SimpleEventQueueFMUBase::initializeParameterValues() {} // Nothing to be done here ...


// This function initializes the backend (everything except the scalar variables and internal
// parameter values). The input arguments are the command line input arguments when the backend
// is started (compare 'Capabilities' and 'VendorAnnotations' in modelDescription.xml).
int
SimpleEventQueueFMUBase::initializeBackEnd( int argc, const char* argv[] )
{
	// Initialize output for debug messages.
	if ( true == loggingOn() ) debug_ = new std::ofstream( "sim_ict_exe.log" );

	fmi2Real start_time = getCurrentCommunicationPoint();

	// Insert first dummy event into the event queue.
	current_event_ = event_queue_.insert( new Event( start_time, 0, true, 0 ) ).first;
	next_event_time = start_time;

	// If default event step size is zero, set it to the largest possible value.
	// This is equivalent to not using default events.
	if ( 0. == default_event_step_size ) default_event_step_size = std::numeric_limits<fmi2Real>::max();

	// Radom generator seed has to be a positive non-zero integer.
	if ( 1 > random_seed ) random_seed = 1;

	// Set ns-3 random generator seed.
	RngSeedManager::SetSeed( random_seed );

	return 0;
}


// This function is called whenever the frontend's doStep(...) function is called.
// When it is called, the variables defined as inputs during initialization have
// already been synchronized to the latest input from the frontend.
int
SimpleEventQueueFMUBase::doStep( const fmi2Real& syncTime, const fmi2Real& lastSyncTime )
{
	std::stringstream debug_msg;
	debug_msg << "DOSTEP: t = " << syncTime << std::endl;

	if ( fabs( syncTime - lastSyncTime ) > 1e-9 ) // syncTime != lastSyncTime -> This is a time advance.
	{
		if ( syncTime > next_event_time ) return 1; // This synchronization step omitted at least one event. -> Abort!

		// Reset in- and outputs.
		resetIntegerInputs();
		resetIntegerOutputs();
	}
	else // syncTime == lastSyncTime: This is an event iteration! Check for new inputs and set ouputs.
	{
		if ( fabs( syncTime - next_event_time ) < 1e-9 ) // This synchronization coincides with an event in the queue.
		{
			debug_msg << "DOSTEP: coincides with event at t = " << next_event_time << std::endl;
			debug_msg << "DOSTEP: event has msg_id = " << (*current_event_)->msg_id << std::endl;

			if ( true == (*current_event_)->default_event ) { // This event is a default event. -> Add the next default event.
				fmi2Real next_default_event_time = (*current_event_)->time_stamp + default_event_step_size;
				event_queue_.insert( current_event_, new Event( next_default_event_time, 0, true, 0 ) );

				//std::stringstream debug_msg;
				debug_msg << "add new default event at t = " << next_default_event_time << std::endl;
				//debug( debug_msg.str() );
			}

			// Set output according to event (in case receiver has been defined).
			if ( 0 != (*current_event_)->receiver ) *(*current_event_)->receiver = (*current_event_)->msg_id;

			runSimulation( syncTime );

			// Get time of next scheduled event.
			if ( current_event_ != std::prev( event_queue_.end() ) ) {
				// There is a next event in the schedule --> increment current event and set time of this event as next event time.
				++current_event_;
				next_event_time = (*current_event_)->time_stamp;
				debug_msg << "DOSTEP: set next event time to t = " << next_event_time << std::endl;
			} else {
				// There is NO next event in the schedule --> set stop time as next event time.
				if ( true == getStopTimeDefined() ) {
					next_event_time = getStopTime(); // Retrieve stop time.
				} else {
					next_event_time = std::numeric_limits<fmi2Real>::max(); // No stop time defined, use other value.
				}
			}

		}
		else // This synchronization does not coincide with an event in the queue.
		{
			// Most likely the FMU is being synchronized because new inputs are available.
			// Reset outputs and run a new ns-3 simulation.
			resetIntegerOutputs();

			runSimulation( syncTime );
		}

		// Reset inputs.
		resetIntegerInputs();
	}

	debug_msg << "DOSTEP: next event time = " << next_event_time << std::endl;
	//debug_msg << "DOSTEP: current event time stamp = " << (*current_event_)->time_stamp << std::endl;
	debug( debug_msg.str() );

	return 0; // No errors, return value 0.
}


void
SimpleEventQueueFMUBase::addNewEventForMessage(
	const TimeStamp& msg_receive_time,
	const MessageID& msg_id,
	const Receiver& receiver )
{
	bool valid_event = false;
	Event* evt = 0;

	std::stringstream debug_msg;

	do {
		// Create new event.
		evt = new Event( msg_receive_time, msg_id, false, receiver );

		debug_msg << "add new evt at t = " << msg_receive_time << " - id = " << msg_id << std::endl;

		// Insert event into queue.
		std::pair<EventQueue::iterator ,bool> insert_result = event_queue_.insert( evt );
		valid_event = insert_result.second;

		// Delete the event, because it has not been inserted successfully (another event with the same timestamp alreay exists).
		if ( false == valid_event ) {
			debug_msg << "veto for event at t = " << msg_receive_time << std::endl;

			delete evt;
		}
		else if ( msg_receive_time < next_event_time )
		{
			debug_msg << "set new event as next event at t = " << msg_receive_time << std::endl;

			current_event_ = insert_result.first;
			next_event_time = msg_receive_time;
		}
	}
	while ( false == valid_event );

	debug_msg << "after adding new event: next event time = " << next_event_time << std::endl;
	//debug_msg << "after adding new event: current event time stamp = " << (*current_event_)->time_stamp << std::endl;
	debug( debug_msg.str() );
}


// Send debug message.
void
SimpleEventQueueFMUBase::debug( const std::string& msg ) const
{
	if ( true == loggingOn() ) (*debug_) << msg << std::endl;
}


// This function resets all integer inputs (i.e., input messages).
void
SimpleEventQueueFMUBase::resetIntegerInputs()
{
	std::vector<fmi2Integer*>::iterator it = integerInputs_.begin();
	std::vector<fmi2Integer*>::iterator end = integerInputs_.end();
	for ( ; it != end; ++it ) **it = 0;
}


// This function resets all integer outputs (i.e., output messages).
void
SimpleEventQueueFMUBase::resetIntegerOutputs()
{
	std::vector<fmi2Integer*>::iterator it = integerOutputs_.begin();
	std::vector<fmi2Integer*>::iterator end = integerOutputs_.end();
	for ( ; it != end; ++it ) **it = 0;
}
