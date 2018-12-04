from FMUCoSimulationV1 import *
from extractFMU import *
import os


def send_messages( fmu, time, msg_id_offset ):
    # Set inputs for all devices.
    input_names = [ 'device{}_data_send'.format(i) for i in range( n_devices ) ]
    input_values = [ ( i + msg_id_offset ) for i in range( n_devices ) ]
    fmu.setInteger( input_names, input_values )
    
    for device, msg_id in zip( input_names, input_values ):
        print( 'At time {:.5f}: SEND msg_id = {} from {}'.format( time, msg_id, device ) )
    
    # Iterate the FMU (simulation with step size 0) to trigger an ns-3 simulation run.
    fmu.doStep(
        current_communication_point = time,
        communication_step_size = 0.
        )
    
    print( 'The maximum delay is {}.'.format( fmu.getReal( [ 'max_device_delay' ] )[0] ) )


def retrieve_messages( fmu, time, stop_time = 10. ):
    while ( time < stop_time ):

        # Get output variable 'next_event_time'
        get_real_values = fmu.getReal( [ 'next_event_time' ] )
        next_event_time = get_real_values[0]

        # Calculate step size for next simulation step.
        step_size = next_event_time - time

        if ( next_event_time >= stop_time ): break

        # Advance internal time of FMU.
        fmu.doStep(
            current_communication_point = time,
            communication_step_size = step_size
            )

        time += step_size

        # Process events by iterating the FMU once (simulation with step size 0).
        fmu.doStep(
            current_communication_point = time,
            communication_step_size = 0.
            )

        # Get output variables.
        output_names = [ 'device{}_data_receive'.format(i) for i in range( n_devices ) ]
        output_values = fmu.getInteger( output_names )

        received_msg = { output_names[i]: output_values[i] for i in range( n_devices ) if output_values[i] is not 0 }
        for device, msg_id in received_msg.items():
            print( 'At time {:.5f}: RECEIVE msg_id = {} at {}'.format( time, msg_id, device ) )

    return time


#
# Extract, instantiate and initialize the FMU.
#
fmu_name = 'LSS2'
fmu_path = os.getcwd()

extractFMU(
    os.path.join( fmu_path, '..', fmu_name + '.fmu' ),
    fmu_path,
    command = 'unzip -q -o {fmu} -d {dir}'
    )

fmu = FMUCoSimulationV1(
    fmu_name,
    fmu_path
    )

# Instantiate FMU.
fmu.instantiateSlave(
    name = 'test_sim_ict',
    visible = False,
    interactive = False,
    logging_on = False
    )

# Define how many devices we consider here.
n_devices = 10
fmu.setInteger( [ 'n_devices' ], [ n_devices] )

# Set random generator seed.
fmu.setInteger( [ 'random_seed' ], [ 1 ] )

start_time = 0.
time = start_time

# Initialize FMU.
fmu.initializeSlave(
    start_time = start_time,
    stop_time_defined = False
    )

#
# Simulate WITHOUT co-channel interference.
#
print( 'Simulate WITHOUT co-channel interference.' )

fmu.setInteger( [ 'interfere' ], [ 0 ] )
send_messages( fmu, time, 1 )
time = retrieve_messages( fmu, time )

#
# Now, do the same thing again, but simulate WITH co-channel interference.
#
print( 'Simulate WITH co-channel interference.' )

fmu.setInteger( [ 'interfere' ], [ 1 ] )
send_messages( fmu, time, n_devices + 1 )
time = retrieve_messages( fmu, time )


# Done.
fmu.terminateSlave()
fmu.freeSlaveInstance()

