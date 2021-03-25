#!/usr/bin/python3

from FMUCoSimulationV1 import *
from extractFMU import *
from pathlib import Path
import math, sys

model_name = 'TC3'

extractFMU(
    Path( __file__ ).parent / '..' / ( model_name + '.fmu' ),
    Path( __file__ ).parent,
    command = 'unzip -o -u {fmu} -d {dir}'
    )

fmu = FMUCoSimulationV1(
    model_name,
    Path( __file__ ).parent
    )

# Instantiate FMU.
fmu.instantiateSlave(
    name = 'test_sim_ict',
    visible = False,
    interactive = False,
    logging_on = False
    )

start_time = 0.
stop_time = 4.

# Set default event step size.
fmu.setReal( [ 'default_event_step_size' ], [ 1.0 ] )

# Set random generator seed.
fmu.setInteger( [ 'random_seed' ], [ 1 ] )

# Initialize FMU.
fmu.initializeSlave(
    start_time = start_time,
    stop_time = stop_time,
    stop_time_defined = True
    )

time = start_time
msg_id = 1

send_step_size = 1.0
next_send_time = start_time + send_step_size

while ( time < stop_time ):

    print( '==========================================' )

    # Get output variable 'next_event_time'
    get_real_values = fmu.getReal( [ 'next_event_time' ] )
    next_event_time = get_real_values[0]

    # Calculate step size for next simulation step.
    step_size = min( next_event_time - time, next_send_time - time )

    if ( time + step_size >= stop_time ): break

    # Advance internal time of FMU.
    fmu.doStep(
        current_communication_point = time,
        communication_step_size = step_size
        )

    time += step_size

    if ( next_event_time == sys.float_info.max ): next_event_time = 'no next event specified'

    print( 'simulation time : {}'.format( time ) )
    print( 'next event time : {}'.format( next_event_time ) )
    print( 'next send time : {}'.format( next_send_time ) )

    # Send messages at regular time intervals.
    if ( math.fabs( time - next_send_time ) < 1e-9 ):
        debug_msg = 'At time {:.5f}: SEND messages to controller with ID = {} and  ID = {}'
        print( debug_msg.format( time, msg_id, -msg_id ) )

        # Set input variable 'u3_send' and 'u4_send'.
        fmu.setInteger( [ 'u3_send', 'u4_send' ], [ msg_id, -msg_id ] )

        msg_id += 1

        next_send_time += send_step_size

    # Process events (i.e., send and retrieve messages) by iterating 
    # the FMU once (doStep with step size 0).
    fmu.doStep(
        current_communication_point = time,
        communication_step_size = 0.
        )

    # Get output variable 'ctrl_receive'
    get_integer_values = fmu.getInteger( [ 'ctrl_receive' ] )
    ctrl_msg_id = get_integer_values[0]
    print( 'ctrl_msg_id = {}'.format( ctrl_msg_id ) )

    if 0 != ctrl_msg_id:
        debug_msg = 'At time {:.5f}: RECEIVE message at controller with ID = {}'
        print( debug_msg.format( time, ctrl_msg_id ) )
        
        debug_msg = 'At time {:.5f}: SEND message from controller with ID = {}'
        print( debug_msg.format( time, 10 * ctrl_msg_id ) )

        fmu.setInteger( [ 'ctrl_send' ], [ 10 * ctrl_msg_id ] )

        fmu.doStep(
            current_communication_point = time,
            communication_step_size = 0.
            )

    # Get output variable 'Transformer_receive'
    trafo_msg_id = fmu.getInteger( [ 'tap_receive' ] )[0]
    if 0 != trafo_msg_id:
        print( 'At time {:.5f}: RECEIVE message at transformer with ID = {}'.format( time, trafo_msg_id ) )

    #timer.sleep(1)

# Done.
fmu.terminateSlave()
fmu.freeSlaveInstance()
