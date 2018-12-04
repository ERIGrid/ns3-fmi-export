from FMUCoSimulationV1 import *
from extractFMU import *

import math, os, sys

model_name = 'SimpleFMU'
work_dir = os.getcwd()

extractFMU(
    os.path.join( work_dir, '..', model_name + '.fmu' ), 
    work_dir, 
    command = 'unzip -o -u {fmu} -d {dir}' 
    )

fmu = FMUCoSimulationV1( 
    model_name, 
    work_dir 
    )

instance_name = "test_sim_ict"
visible = False
interactive = False
logging_on = False

# Instantiate FMU.
fmu.instantiateSlave(
    name = instance_name,
    visible = visible,
    interactive = interactive,
    logging_on = logging_on
    )

start_time = 0.
stop_time = 4.

# Initialize FMU.
fmu.initializeSlave(
    start_time = start_time,
    stop_time = stop_time,
    stop_time_defined = True
    )

# Set channel delay.
fmu.setReal( [ 'channel_delay' ], [ 0.3 ] )

time = start_time
msg_id = 1

send_step_size = 1.0
next_send_time = start_time + send_step_size

while ( time < stop_time ):

    print( '================================================' )

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

    # Advance simulation time.
    time = min( next_event_time, next_send_time )

    if ( next_event_time == sys.float_info.max ): next_event_time = 'no next event specified'
    
    print( 'simulation time : {}'.format( time ) )
    print( 'next event time : {}'.format( next_event_time ) )
    print( 'next send time : {}'.format( next_send_time ) )

    # Send messages at regular time intervals.
    if ( math.fabs( time - next_send_time ) < 1e-9 ):
        print( 'At time {:.5f}: SEND message with ID = {}'.format( time, msg_id ) )

        # Set input variable 'nodeA_send'.
        fmu.setInteger( [ 'nodeA_send' ], [ msg_id ] )

        msg_id += 1

        next_send_time += send_step_size

    # Process events by iterating the FMU once (doStep with step size 0).
    fmu.doStep(
        current_communication_point = time,
        communication_step_size = 0.
        )

    # Get output variable 'nodeC_receive'
    get_integer_values = fmu.getInteger( [ 'nodeB_receive' ] )
    nodeB_msg = get_integer_values[0]
    if ( 0 != nodeB_msg ):
        print( 'At time {:.5f}: RECEIVE message with ID = {}'.format( time, nodeB_msg ) )

# Done.
fmu.terminateSlave()
fmu.freeSlaveInstance()
