# The ns-3 FMI Export Module

## About

Module **fmi-export** enables the FMI-compliant simulation coupling with ns-3 scripts, i.e., ns-3 script are launched and executed through an FMI-compliant co-simulation interface.
In terms of FMI terminology, ns-3 is the slave application, and generated FMUs launch ns-3 and synchronize its execution during runtime (tool coupling).

Module **fmu-examples** provides examples for using the **fmi-export** module.
The module comprises dedicated models (clients and servers), helpers and simulation scripts implementing example applications, whose functionality is then exported as FMU for Co-Simulation.
Furthermore, test applications (written in Python) show how the resulting FMUs can be used in a simulation.

## Prerequisites and installation on Linux

In addition to ns-3, the following tools/libraries need to be installed:

* **Cmake**
* **Boost**: all header files plus compiled *date_time*, *system* and *filesystem* libraries


Follow these instructions to install the **fmi-export** module:

1. This module relies on a lot of functionality provided by the FMI++ library.
   Hence, in order to install this module, the latest version of the FMI++ library should be cloned from its repository:
```
     $ git clone https://git.code.sf.net/p/fmipp/code fmipp
```

2. Get the [source code from GitHub](https://github.com/ERIGrid/ns3-fmi-export).
```
     $ git clone git@github.com:ERIGrid/ns3-fmi-export.git
```

3. From the source code, copy the *fmi-export* directory (and the *fmu-examples* directory if you want to include examples) to the *src* subdirectory of ns-3, i.e., the directory with all the other ns-3 modules.

4. Change into the ns-3 directory and configure `waf` with the *--with-fmi-export* flag set to the previously cloned FMI++ library:
```
     $ ./waf configure --with-fmi-export=/path/to/cloned/fmipp/code
```

5. Build the module using `waf`:
```
     $ ./waf
```

## Prerequisites and installation in a Cygwin environment (Windows)

ns-3 is mainly developed for Linux, but it can also be installed on Windows in a Cygwin environment:

1. Run the [Cygwin installer](https://cygwin.com/install.html)

2. During installation, chose all required packages.
   For instance, for *Cygwin version 2.891 (32-bit)* the following packages are the minimum requirement:

   - cmake (version 3.6.2-1)
   - gcc-g++ (version 7.3.0-3)
   - git (version 2.17.0-1)
   - libboost-devel (version 1.66.0-1)
   - make (version 4.2.1-2)
   - mercurial (version 4.3.2-1)
   - python2-pip (version 9.0.1-1)
   - unzip (version 6.0-17)

3. In the *Cygwin Terminal*, set the compiler flags:
```
    $ export CXXFLAGS="-D_USE_MATH_DEFINES -D_BSD_SOURCE -include limits.h"
```

4. In the *Cygwin Terminal*, follow the [standard installation instructions for Linux](https://www.nsnam.org/support/faq/setup/):
```
    $ hg clone http://code.nsnam.org/ns-3-allinone
    $ cd ns-3-allinone
    $ ./download.py
    $ ./build.py
```

5. Retrieve the latest version of the FMI++ library from its repository:
```
     $ git clone https://git.code.sf.net/p/fmipp/code fmipp
```

6. Get the [source code from GitHub](https://github.com/ERIGrid/ns3-fmi-export).
```
     $ git clone git@github.com:ERIGrid/ns3-fmi-export.git
```

7. From the source code, copy the *fmi-export* directory (and the *fmu-examples* directory if you want to include examples) to the *src* subdirectory of ns-3, i.e., the directory with all the other ns-3 modules.

8. Change into the ns-3 directory and configure `waf` with the *--with-fmi-export* flag set to the previously cloned FMI++ library:
```
     $ ./waf configure --with-fmi-export=/path/to/cloned/fmipp/code
```

9. Build the module using `waf`:
```
     $ ./waf
```

**IMPORTANT NOTE**: The extra compilers flags from step 3 should always be set when running `./waf configure` or `./waf`.


## FMI-compliant ns-3 scripts

ns-3 scripts have to implement the abstract class *SimpleEventQueueFMUBase*.
This class provides a simple event queue that can be synchronized via an FMI-compliant interface.
Incoming/outgoing messages are *associated with input/output variables*.
Whenever a message is sent from a network node, the associated input variable is set to a non-zero integer value, referred to as *message ID*.
When the same message is received at another network node, the associated output variable is set to the same message ID.
The sending/receiving of messages is simulated with individual ns-3 simulation runs, whose results are stored as events in the queue.

To define an FMI-compliant ns-3 script, a new class inheriting from *SimpleEventQueueFMUBase* has to be implementedwhich provides the following two functions:

* Function *initializeSimulation()*:
  This function allows to define simulation variables (typically member variables of the inheriting class) as inputs/outputs/parameters of the ns-3 simulation.

* Function *runSimulation( const double& sync_time )*:
  This function is called whenever a new ns-3 simulation has to be run.
  Most of the code it contains is what you would typically find in an ns-3 script.
  Results of such a simulation can be added to the event queue via function *addNewEventForMessage( evt_time, msg_id, output_var )*.

After the definition of the class, the macro *CREATE_NS3_FMU_BACKEND* has to be used.
This macro replaces the typical main function of ns-3 scripts.


## FMU generation using Python scripts

FMUs can be generated using the Python script `ns3_fmu_create.py`.
The FMU is created by executing the Python script from the command line:
```
     ns3_fmu_create.py [-h] [-v] -m <model_id> -s <ns3_script> \
        [-f <fmi_version>] [<additional_file_1> ... <additional_file_N>] \
        [var1=start_val1 ... varN=start_valN]
```
Optional arguments are enclosed by squared brackets [...].

### Mandatory input arguments

* *-m, --model-id*: Specify the FMU model identifier. *Attention*: The FMU model identifier must fulfill the restrictions for C function names!
* *-s, --script*: Path to ns-3 script (absolute or relative).

### Optional input arguments

* *-h, --help*: Display the help screen.
* *-v, --verbose*: Turn on log messages.
* *-l, --litter*: Do not clean-up intermediate files (e.g., log file with debug messages from compilation).
* *-f, --fmi-version*: Specify FMI version (1 or 2, default is 2)

Additional files may be specified (e.g., CSV input lists) that will be automatically copied to the FMU.
The specified files paths may be absolute or relative.

Start values for variables and parameters may be defined.
For instance, to set variable with name *var1* to value *12.34*, specify *var1=12.34* in the command line as optional argument.


## Using an FMU generated for ns-3

During simulation, interaction with the FMU is basically limited to the use of three (types of) functions:

* *Setter functions*: Set the value (message ID) of input variables. This corresponds to sending a message. 0 means no input.

* *Getter functions*: Retrieve the value (message ID) of output variables. This corresponds to receiving a message. 0 means no output.

* *Synchronization*: An FMU for co-simulation is synchronized from one synchronization point to the next by calling the *doStep( ..., com_point, step_size, ...)* function, where *com_point* is the time of the last successful FMU synchronization and *step_size* is the length of the next simulation step.
  In the case of FMUs for ns-3, which internally implement an event queue, there are three distinct ways of calling this function:

  1. *Time advance*: When doStep(...) is called with step_size > 0, then the FMU tries to advance its internal simulation time accordingly.

  2. *Receiving messages*: When calling doStep(...) with step_step = 0 (an FMU iteration) at a time corresponding to an internal event, the value(s) of the associated output variable(s) will be set to the according message ID.
     To retrieve the actual message ID, call the getter function *after* the FMU iteration.

  3. *Sending messages*: When calling doStep(...) with step_size = 0 (an FMU iteration) even though there is no internal event scheduled at this time, then the FMU assumes that one or more new messages have been sent and a new ns-3 simulation should be run.
     To trigger an ns-3 simulation, provide new message IDs via the setter functions directly before the FMU iteration (but after a time advance).


## Examples

The example applications are test cases from the [ERIGrid](https://erigrid.eu/) project:

* *SimpleFMU*:
  A very simple test case where a client sends data to a server.

* *TC3*:
  This test case comprises two smart meters sending data to a voltage controller, which sends data to actuate the tap position of an OLTC transformer.
  This test case is described in detail in ERIGrid deliverable D-JRA2.2.

* *LSS2*:
  This test case also looks on the data transmission of smart meters to a controller, focusing on the effect of co-channel interference of Wi-Fi networks.
  This test case is described in detail in ERIGrid deliverable D-JRA2.3.

Module **fmu-examples** provides the models for these test cases.
These models implement dedicated clients and servers, which provide the functionality to extract the end-to-end delay of message transmissions.
Based on these end-to-end delays, the ns-3 simulation scripts add events to the event queue of the FMU.

The classes *ClientBase* and *ServerBase* are the bases classes for all the clients and servers implemented for the example applications.
The implemented clients and servers are examples of how callback functions can be used to calculate end-to-end delays.
Helpers for including the clients and servers into simulations scripts are provided.
All helpers are specializations of the template base classes *ClientHelperBase* and *ServerHelperBase*.


The examples are implemented in dedicated ns-3 scripts, which can be found in the module's subdirectory *examples/scratch*.
The scripts can be translated to FMUs for Co-Simulations using Python script `ns3_fmu_create.py` (from module *fmi-export*).
Test applications (written in Python) using these FMUs can be found in module's subdirectory *examples/test*.

To build the examples, copy directory *fmu-examples* to ns-3's *src* directory (i.e., the directory with all the other ns-3 modules).
Then change into the ns-3 root directory and build the module using `waf`.


### Example SimpleFMU

#### Overview

The ns-3 script of the simple example can be found (``SimpleFMU.cc``).
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
  This function runs an ns-3 simulation every time new inputs are set to the FMU (and the FMU is iterated, see above).
  Most of the code is just like for normal ns-3 scripts, there are a few differences however:

  * At the beginning of the function, variable *nodeA_send* is checked to be equal to zero, in order to know if indeed a message has been sent (i.e., a message ID is available as input).
  * At the end of the simulation, the end-to-end delay of the message is retrieved.
  * Finally, the event queue is updated with a new event using function *addNewEventForMessage(...)*.
    The inputs are the time at which the message is received (i.e., the time of the FMU synchronization plus the end-to-end delay), the message ID and the pointer to the corresponding output variable.

* At the very end of the script, macro *CREATE_NS3_FMU_BACKEND* is used with the name of the new class (SimpleFMU).
  This macro is basically a replacement for a main function.


#### Creating the FMU

Create the FMU with the help of Python script `ns3_fmu_create.py`.
In the command line, go to the example directory *src/fmu-examples/examples* and issue the following command:
```
  $ ./../../fmi-export/ns3_fmu_create.py -v -m SimpleFMU -s scratch/SimpleFMU.cc -f 1 channel_delay=0.2
```

This command does the following:

* It defines the FMU's model identifier as *SimpleFMU*.
  This means that the resulting FMU will be called *SimpleFMU.fmu*.

* It defines *scratch/SimpleFMU.cc* as the ns-3 script to be used for the simulation.

* The parameter *channel_delay* is set to 0.3.


The output of the script in the command line should be something along the following lines. (Note that `waf` is called twice during the process.)
```
    [DEBUG] Using FMI version 1
    [DEBUG] Found start value:  channel_delay = 0.2
    Waf: Entering directory `/cygdrive/c/Development/erigrid/ns-3-allinone/ns-3-dev/build'
    Waf: Leaving directory `/cygdrive/c/Development/erigrid/ns-3-allinone/ns-3-dev/build'
    Build commands will be stored in build/compile_commands.json
    'build' finished successfully (1.363s)

    Modules built:
    antenna			aodv					applications
    bridge				buildings				config-store
    core				csma					csma-layout
    dsdv				dsr					energy
    flow-monitor			fmi-export (no Python)	fmu-examples (no Python)
    internet			internet-apps			lr-wpan
    lte					mesh				mobility
    mpi				netanim (no Python)		network
    nix-vector-routing		olsr					point-to-point
    point-to-point-layout	propagation			sixlowpan
    spectrum			stats					test (no Python)
    topology-read		traffic-control			uan
    virtual-net-device		wave					wifi
    wimax

    Modules not built (see ns-3 tutorial for explanation):
    brite					click					fd-net-device
    openflow				tap-bridge				visualizer

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
```

#### Using the FMU in a simulation

Python script `testSimpleFMU.py` uses the generated FMU in a simulation.
It can be found in the module's subdirectory *examples/test*.
When running the simulation script, the output should be similar to the following:
```
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
```

### Example TC3

#### Creating the FMU

Create the FMU with the help of Python script `ns3_fmu_create.py`.
In the command line, go to the example directory *src/fmu-examples/examples* and issue the following command:
```
     $ ./../../fmi-export/ns3_fmu_create.py -v -m TC3 -s scratch/TC3.cc -f 1
```

#### Using the FMU in a simulation

Python script `testTC3.py` uses the generated FMU in a simulation.
It can be found in the module's subdirectory *examples/test*.
When running the simulation script, the output should be similar to the following:
```
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
```

### Example LSS2

#### Creating the FMU

Create the FMU with the help of Python script `ns3_fmu_create.py`.
In the command line, go to the example directory *src/fmu-examples/examples* and issue the following command:
```
  $ ./../../fmi-export/ns3_fmu_create.py -v -m LSS2 -s scratch/LSS2.cc -f 1 max_jitter=0.01
```

#### Using the FMU in a simulation

Python script `testLSS2.py` uses the generated FMU in a simulation.
It can be found in the module's subdirectory *examples/test*.
When running the simulation script, the output should be similar to the following:
```
    [test_sim_ict] WARNING: The path specified for the FMU's entry point does not exist: ""
    Use directory of main application as working directory instead.
    [test_sim_ict] MIME-TYPE: Wrong MIME type: application/x-waf --- expected:
    Waf: Entering directory `/cygdrive/c/Development/erigrid/ns-3-allinone/ns-3-dev/build'
    Waf: Leaving directory `/cygdrive/c/Development/erigrid/ns-3-allinone/ns-3-dev/build'
    Build commands will be stored in build/compile_commands.json
    'build' finished successfully (1.371s)
    Simulate WITHOUT co-channel interference.
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
    Simulate WITH co-channel interference.
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
```
