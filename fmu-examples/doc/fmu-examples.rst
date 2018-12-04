
FMU Example Module for |ns3|
-----------------------------

.. |ns3| replace:: ns-3

The *fmu-example* module provides examples for using the *fmi-export* module.
The module comprises dedicated models (clients and servers), helpers and simulation scripts implementing example applications, whose functionality is then exported as FMU for Co-Simulation.
Furthermore, test applications (written in Python) show how the resulting FMUs can be used in a simulation.

The example applications are test cases from the `ERIGrid <https://erigrid.eu/>`_ project:

* *SimpleFMU*:
  A very simple test case where a client sends data to a server.

* *TC3*:
  This test case comprises two smart meters sending data to a voltage controller, which sends data to actuate the tap position of an OLTC transformer.
  This test case is described in detail in ERIGrid deliverable D-JRA2.2.

* *LSS2*:
  This test case also looks on the data transmission of smart meters to a controller, focusing on the effect of co-channel interference of Wi-Fi networks.
  This test case is described in detail in ERIGrid deliverable D-JRA2.3.


Model Description
*****************

Design
======

This module provides the models for the test cases mentioned above.
These models implement dedicated clients and servers, which provide the functionality to extract the end-to-end delay of message transmissions.
Based on these end-to-end delays, the |ns3| simulation scripts add events to the event queue of the FMU.

Models
=======

The classes *ClientBase* and *ServerBase* are the bases classes for all the clients and servers implemented for the example applications.
The implemented clients and servers are examples of how callback functions can be used to calculate end-to-end delays.

Helpers
=======

Helpers for including the clients and servers into simulations scripts are provided.
All helpers are specializations of the template base classes *ClientHelperBase* and *ServerHelperBase*.


Usage
*****

Prerequisites
=============

In addition to |ns3|, the *fmi-export* module must be installed.


Building the fmu-export module
==============================

1. Copy the ``fmu-examples`` directory to the ``src`` directory, i.e., the directory with all the other |ns3| modules.

2. Change into the |ns3| directory and build the module using *waf*:

::

        $ ./waf


Example applications
====================

The examples are implemented in dedicated |ns3| scripts, which can be found in the module's subdirectory ``examples/scratch``.
The scripts can be translated to FMUs for Co-Simulations using Python script *ns3_fmu_create.py* (from module *fmi-export*).
Test applications (written in Python) using these FMUs can be found in module's subdirectory ``examples/test``.


Example SimpleFMU
=================

Overview
########

The |ns3| script of the simple example can be found (``SimpleFMU.cc``).
It implements a simple simulation in which one node (A) send messages to another node (B).

The script defines class *SimpleFMU*, which inherits from class *SimpleEventQueueFMUBase*:
* The class defines three class member variables:

  1. Variable *nodeA_send* is of type fmi2Integer and will be used as input variable for the final FMU
  2. Variable *nodeB_receive* is of type fmi2Integer and will be used as output variable for the final FMU
  3. Variable *channel_delay* is of type fmi2Real and will be used as parameter for the final FMU

* Function *initializeSimulation()*:
  This function uses the macros *addIntegerInput(...)*, *addIntegerOutput(...)* and *addRealParameter(...)* to define the class member variables as input, output and parameter, respectively.
  This definition is sufficient to create later on the FMU with an input, output and parameter with exactly the same names as the corresponding variables in the script.

* Function *runSimulation( const double& sync_time )*:
  This function runs an |ns3| simulation every time new inputs are set to the FMU (and the FMU is iterated, see above).
  Most of the code is just like for normal |ns3| scripts, there are a few differences however:

  * At the beginning of the function, variable *nodeA_send* is checked to be equal to zero, in order to know if indeed a message has been sent (i.e., a message ID is available as input).
  * At the end of the simulation, the end-to-end delay of the message is retrieved.
  * Finally, the event queue is updated with a new event using function *addNewEventForMessage(...)*.
    The inputs are the time at which the message is received (i.e., the time of the FMU synchronization plus the end-to-end delay), the message ID and the pointer to the corresponding output variable.

* At the very end of the script, macro *CREATE_NS3_FMU_BACKEND* is used with the name of the new class (SimpleFMU).
  This macro is basically a replacement for a main function.


Creating the FMU
################

Create the FMU with the help of Python script *ns3_fmu_create.py*.
In the command line, go to the example directory (``src/fmu-examples/examples``) and issue the following command:

::

  $ ./../../fmi-export/ns3_fmu_create.py -v -m SimpleFMU \
      -s scratch/SimpleFMU.cc -f 1 channel_delay=0.2


This command does the following:

* It defines the FMU's model identifier as *SimpleFMU*.
  This means that the resulting FMU will be called *SimpleFMU.fmu*.

* It defines ``scratch/SimpleFMU.cc`` as the |ns3| script to be used for the simulation.

* The parameter *channel_delay* is set to 0.3.


The output of the script in the command line should be something along the following lines. (Note that *waf* is called twice during the process.)

::

    [DEBUG] Using FMI version 1
    [DEBUG] Found start value:  channel_delay = 0.2
    Waf: Entering directory `/cygdrive/c/Development/erigrid/ns-3-allinone/ns-3-dev/build'
    Waf: Leaving directory `/cygdrive/c/Development/erigrid/ns-3-allinone/ns-3-dev/build'
    Build commands will be stored in build/compile_commands.json
    'build' finished successfully (1.363s)

    Modules built:
    antenna                   aodv                      applications
    bridge                    buildings                 config-store
    core                      csma                      csma-layout
    dsdv                      dsr                       energy
    flow-monitor              fmi-export (no Python)    fmu-examples (no Python)
    internet                  internet-apps             lr-wpan
    lte                       mesh                      mobility
    mpi                       netanim (no Python)       network
    nix-vector-routing        olsr                      point-to-point
    point-to-point-layout     propagation               sixlowpan
    spectrum                  stats                     test (no Python)
    topology-read             traffic-control           uan
    virtual-net-device        wave                      wifi
    wimax

    Modules not built (see ns-3 tutorial for explanation):
    brite                     click                     fd-net-device
    openflow                  tap-bridge                visualizer

    [DEBUG] successfully compiled ns-3 script
    Waf: Entering directory `/cygdrive/c/Development/erigrid/ns-3-allinone/ns-3-dev/build'
    Waf: Leaving directory `/cygdrive/c/Development/erigrid/ns-3-allinone/ns-3-dev/build'
    Build commands will be stored in build/compile_commands.json
    'build' finished successfully (1.349s)
    [DEBUG] successfully created JSON script
    [DEBUG] FMI model identifier:  SimpleFMU
    [DEBUG] ns-3 script:  scratch/SimpleFMU.cc
    [DEBUG] ns-3 install directory:  /cygdrive/c/Development/erigrid/ns-3-allinone/ns-3-dev
    [DEBUG] Aditional files:
    [DEBUG] Added start value to model description:  channel_delay = 0.2
    [DEBUG] FMU created successfully: SimpleFMU.fmu


Using the FMU in a simulation
#############################

Python script ``testSimpleFMU.py`` uses the generated FMU in a simulation.
It can be found in the module's subdirectory ``examples/test``.
When running the simulation script, the output should be similar to the following:

::

    [test_sim_ict] WARNING: The path specified for the FMU's entry point does not exist: ""
    Use directory of main application as working directory instead.
    [test_sim_ict] MIME-TYPE: Wrong MIME type: application/x-waf --- expected:
    Waf: Entering directory `/cygdrive/c/Development/erigrid/ns-3-allinone/ns-3-dev/build'
    Waf: Leaving directory `/cygdrive/c/Development/erigrid/ns-3-allinone/ns-3-dev/build'
    Build commands will be stored in build/compile_commands.json
    'build' finished successfully (1.406s)
    ================================================
    simulation time : 0.0
    next event time : 0.0
    next send time : 1.0
    ================================================
    simulation time : 1.0
    next event time : no next event specified
    next send time : 1.0
    At time 1.00000: SEND message with ID = 1
    ================================================
    simulation time : 1.301686399
    next event time : 1.301686399
    next send time : 2.0
    At time 1.30169: RECEIVE message with ID = 1
    ================================================
    simulation time : 2.0
    next event time : no next event specified
    next send time : 2.0
    At time 2.00000: SEND message with ID = 2
    ================================================
    simulation time : 2.301686399
    next event time : 2.301686399
    next send time : 3.0
    At time 2.30169: RECEIVE message with ID = 2
    ================================================
    simulation time : 3.0
    next event time : no next event specified
    next send time : 3.0
    At time 3.00000: SEND message with ID = 3
    ================================================
    simulation time : 3.301686399
    next event time : 3.301686399
    next send time : 4.0
    At time 3.30169: RECEIVE message with ID = 3
    ================================================


Example TC3
===========

Creating the FMU
################

Create the FMU with the help of Python script *ns3_fmu_create.py*.
In the command line, go to the example directory (``src/fmu-examples/examples``) and issue the following command:

::

  $ ./../../fmi-export/ns3_fmu_create.py -v -m TC3 -s scratch/TC3.cc -f 1


Using the FMU in a simulation
#############################

Python script ``testTC3.py`` uses the generated FMU in a simulation.
It can be found in the module's subdirectory ``examples/test``.
When running the simulation script, the output should be similar to the following:

::

    [test_sim_ict] WARNING: The path specified for the FMU's entry point does not exist: ""
    Use directory of main application as working directory instead.
    [test_sim_ict] MIME-TYPE: Wrong MIME type: application/x-waf --- expected:
    Waf: Entering directory `/cygdrive/c/Development/erigrid/ns-3-allinone/ns-3-dev/build'
    Waf: Leaving directory `/cygdrive/c/Development/erigrid/ns-3-allinone/ns-3-dev/build'
    Build commands will be stored in build/compile_commands.json
    'build' finished successfully (1.415s)
    ==========================================
    simulation time : 0.0
    next event time : 0.0
    next send time : 1.0
    ctrl_msg_id = 0
    ==========================================
    simulation time : 1.0
    next event time : 1.0
    next send time : 1.0
    At time 1.00000: SEND messages to controller with ID = 1 and  ID = -1
    ctrl_msg_id = 0
    ==========================================
    simulation time : 1.039132019
    next event time : 1.039132019
    next send time : 2.0
    ctrl_msg_id = 1
    At time 1.03913: RECEIVE message at controller with ID = 1
    At time 1.03913: SEND message from controller with ID = 10
    ==========================================
    simulation time : 1.049157658
    next event time : 1.049157658
    next send time : 2.0
    ctrl_msg_id = -1
    At time 1.04916: RECEIVE message at controller with ID = -1
    At time 1.04916: SEND message from controller with ID = -10
    ==========================================
    simulation time : 1.079941038
    next event time : 1.079941038
    next send time : 2.0
    ctrl_msg_id = 0
    At time 1.07994: RECEIVE message at transformer with ID = 10
    ==========================================
    simulation time : 1.089867677
    next event time : 1.089867677
    next send time : 2.0
    ctrl_msg_id = 0
    At time 1.08987: RECEIVE message at transformer with ID = -10
    ==========================================
    simulation time : 2.0
    next event time : 2.0
    next send time : 2.0
    At time 2.00000: SEND messages to controller with ID = 2 and  ID = -2
    ctrl_msg_id = 0
    ==========================================
    simulation time : 2.039015019
    next event time : 2.039015019
    next send time : 3.0
    ctrl_msg_id = 2
    At time 2.03902: RECEIVE message at controller with ID = 2
    At time 2.03902: SEND message from controller with ID = 20
    ==========================================
    simulation time : 2.049040658
    next event time : 2.049040658
    next send time : 3.0
    ctrl_msg_id = -2
    At time 2.04904: RECEIVE message at controller with ID = -2
    At time 2.04904: SEND message from controller with ID = -20
    ==========================================
    simulation time : 2.076761038
    next event time : 2.076761038
    next send time : 3.0
    ctrl_msg_id = 0
    At time 2.07676: RECEIVE message at transformer with ID = 20
    ==========================================
    simulation time : 2.090786677
    next event time : 2.090786677
    next send time : 3.0
    ctrl_msg_id = 0
    At time 2.09079: RECEIVE message at transformer with ID = -20
    ==========================================
    simulation time : 3.0
    next event time : 3.0
    next send time : 3.0
    At time 3.00000: SEND messages to controller with ID = 3 and  ID = -3
    ctrl_msg_id = 0
    ==========================================
    simulation time : 3.041898019
    next event time : 3.041898019
    next send time : 4.0
    ctrl_msg_id = -3
    At time 3.04190: RECEIVE message at controller with ID = -3
    At time 3.04190: SEND message from controller with ID = -30
    ==========================================
    simulation time : 3.052203658
    next event time : 3.052203658
    next send time : 4.0
    ctrl_msg_id = 3
    At time 3.05220: RECEIVE message at controller with ID = 3
    At time 3.05220: SEND message from controller with ID = 30
    ==========================================
    simulation time : 3.079644038
    next event time : 3.079644038
    next send time : 4.0
    ctrl_msg_id = 0
    At time 3.07964: RECEIVE message at transformer with ID = -30
    ==========================================
    simulation time : 3.096913677
    next event time : 3.096913677
    next send time : 4.0
    ctrl_msg_id = 0
    At time 3.09691: RECEIVE message at transformer with ID = 30
    ==========================================


Example LSS2
============

Creating the FMU
################

Create the FMU with the help of Python script *ns3_fmu_create.py*.
In the command line, go to the example directory (``src/fmu-examples/examples``) and issue the following command:

::

  $ ./../../fmi-export/ns3_fmu_create.py -v -m LSS2 -s scratch/LSS2.cc \
      -f 1 max_jitter=0.01


Using the FMU in a simulation
#############################

Python script ``testLSS2.py`` uses the generated FMU in a simulation.
It can be found in the module's subdirectory ``examples/test``.
When running the simulation script, the output should be similar to the following:

::

    [test_sim_ict] WARNING: The path specified for the FMU's entry point does not exist: ""
    Use directory of main application as working directory instead.
    [test_sim_ict] MIME-TYPE: Wrong MIME type: application/x-waf --- expected:
    Waf: Entering directory `/cygdrive/c/Development/erigrid/ns-3-allinone/ns-3-dev/build'
    Waf: Leaving directory `/cygdrive/c/Development/erigrid/ns-3-allinone/ns-3-dev/build'
    Build commands will be stored in build/compile_commands.json
    'build' finished successfully (1.371s)
    Simulate WITHOUT co-simulation interference.
    At time 0.00000: SEND msg_id = 1 from device0_data_send
    At time 0.00000: SEND msg_id = 2 from device1_data_send
    At time 0.00000: SEND msg_id = 3 from device2_data_send
    At time 0.00000: SEND msg_id = 4 from device3_data_send
    At time 0.00000: SEND msg_id = 5 from device4_data_send
    At time 0.00000: SEND msg_id = 6 from device5_data_send
    At time 0.00000: SEND msg_id = 7 from device6_data_send
    At time 0.00000: SEND msg_id = 8 from device7_data_send
    At time 0.00000: SEND msg_id = 9 from device8_data_send
    At time 0.00000: SEND msg_id = 10 from device9_data_send
    The maximum delay is 0.0220982834417.
    At time 0.01544: RECEIVE msg_id = 4 at device3_data_receive
    At time 0.01546: RECEIVE msg_id = 3 at device2_data_receive
    At time 0.01548: RECEIVE msg_id = 5 at device4_data_receive
    At time 0.01550: RECEIVE msg_id = 7 at device6_data_receive
    At time 0.01570: RECEIVE msg_id = 8 at device7_data_receive
    At time 0.01716: RECEIVE msg_id = 2 at device1_data_receive
    At time 0.01911: RECEIVE msg_id = 6 at device5_data_receive
    At time 0.02028: RECEIVE msg_id = 10 at device9_data_receive
    At time 0.02149: RECEIVE msg_id = 9 at device8_data_receive
    At time 0.02210: RECEIVE msg_id = 1 at device0_data_receive
    Simulate WITH co-simulation interference.
    At time 0.02210: SEND msg_id = 11 from device0_data_send
    At time 0.02210: SEND msg_id = 12 from device1_data_send
    At time 0.02210: SEND msg_id = 13 from device2_data_send
    At time 0.02210: SEND msg_id = 14 from device3_data_send
    At time 0.02210: SEND msg_id = 15 from device4_data_send
    At time 0.02210: SEND msg_id = 16 from device5_data_send
    At time 0.02210: SEND msg_id = 17 from device6_data_send
    At time 0.02210: SEND msg_id = 18 from device7_data_send
    At time 0.02210: SEND msg_id = 19 from device8_data_send
    At time 0.02210: SEND msg_id = 20 from device9_data_send
    The maximum delay is 0.179793258798.
    At time 0.11613: RECEIVE msg_id = 16 at device5_data_receive
    At time 0.12119: RECEIVE msg_id = 13 at device2_data_receive
    At time 0.13336: RECEIVE msg_id = 19 at device8_data_receive
    At time 0.16874: RECEIVE msg_id = 11 at device0_data_receive
    At time 0.16930: RECEIVE msg_id = 12 at device1_data_receive
    At time 0.16990: RECEIVE msg_id = 15 at device4_data_receive
    At time 0.17166: RECEIVE msg_id = 14 at device3_data_receive
    At time 0.17218: RECEIVE msg_id = 20 at device9_data_receive
    At time 0.17288: RECEIVE msg_id = 17 at device6_data_receive
    At time 0.20189: RECEIVE msg_id = 18 at device7_data_receive

