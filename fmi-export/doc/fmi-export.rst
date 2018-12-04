FMI++ Export Module for |ns3|
-----------------------------

.. |ns3| replace:: ns-3


This package enables the FMI-compliant simulation coupling with |ns3| scripts, i.e., |ns3| script are launched and executed through an FMI-compliant co-simulation interface.
In terms of FMI terminology, |ns3| is the slave application, and generated FMUs launch |ns3| and synchronize its execution during runtime (tool coupling).



Model Description
*****************

The source code for the new module lives in the directory ``src/fmi-export``.


Design
======

The *fmi-export* module provides the following:

* *Generic FMI adapter*: To make |ns3| simulations accessible via an FMI-compliant interface, |ns3| scripts have to implement classes that derive from the abstract class *SimpleEventQueueFMUBase*.

* *Python scripts for generating FMUs*: The Python script *ns3_fmu_create.py* generates FMUs for Co-Simulation (tool coupling) from |ns3| scripts that implement the abstract class *SimpleEventQueueFMUBase*.


Scope and Limitations
=====================

This is a prototype implementation of an FMI-compliant co-simulation interface.
At the moment it only supports the use of ns-3 by running individual simulations for every co-simulation step.
This approach basically used ns-3 like a "function", i.e., ns-3 is called with a certain set of inputs, executes a simulation and returns the results.
The development of a more interactive way of including ns-3 into a co-simulation is planned in the future.


References
==========

* E. Widl and M. Mueller. Generic FMI-compliant Simulation Tool Coupling. Proceedings of the 12th International Modelica Conference, Prague, Czech Republic, May 15-17, 2017. http://dx.doi.org/10.3384/ecp17132321


Usage
*****

Prerequisites
=============

In addition to |ns3|, the following tools/libraries need to be installed:

* Cmake
* Boost: all header files plus compiled *date_time*, *system* and *filesystem* libraries


Building the fmi-export module
==============================

1. This module relies on a lot of functionality provided by the FMI++ library.
   Hence, in order to install this module, the latest version of the FMI++ library should be cloned from its repository:

::

        $ git clone https://git.code.sf.net/p/fmipp/code fmipp

2. Copy the ``fmi-export`` directory to the ``src`` directory, i.e., the directory with all the other |ns3| modules.

3. Change into the |ns3| directory and configure *waf* with the *--with-fmi-export* flag set to the previously cloned FMI++ library:

::

        $ ./waf configure --with-fmi-export=/path/to/cloned/fmipp/code

4. Build the module using *waf*:

::

        $ ./waf


FMI-compliant |ns3| scripts
===========================

|ns3| scripts have to implement the abstract class *SimpleEventQueueFMUBase*.
This class provides a simple event queue that can be synchronized via an FMI-compliant interface.
Incoming/outgoing messages are *associated with input/output variables*.
Whenever a message is sent from a network node, the associated input variable is set to a non-zero integer value, referred to as *message ID*.
When the same message is received at another network node, the associated output variable is set to the same message ID.
The sending/receiving of messages is simulated with individual |ns3| simulation runs, whose results are stored as events in the queue.

To define an FMI-compliant |ns3| script, a new class inheriting from *SimpleEventQueueFMUBase* has to be implementedwhich provides the following two functions:

* Function *initializeSimulation()*: This function allows to define simulation variables (typically member variables of the inheriting class) as inputs/outputs/parameters of the |ns3| simulation.

* Function *runSimulation( const double& sync_time )*: This function is called whenever a new |ns3| simulation has to be run. Most of the code it contains is what you would typically find in an |ns3| script. Results of such a simulation can be added to the event queue via function *addNewEventForMessage( evt_time, msg_id, output_var )*.

After the definition of the class, the macro *CREATE_NS3_FMU_BACKEND* has to be used.
This macro replaces the typical main function of |ns3| scripts.


FMU generation using Python scripts
===================================

FMUs can be generated using the Python script *ns3_fmu_create.py*.
The FMU is created by executing the Python script from the command line:

::

    ns3_fmu_create.py [-h] [-v] -m <model_id> -s <ns3_script> \
      [-f <fmi_version>] [<additional_file_1> ... <additional_file_N>] \
      [var1=start_val1 ... varN=start_valN]

Optional arguments are enclosed by squared brackets [...].

Mandatory input arguments
##########################

* -m, --model-id: Specify the FMU model identifier. *Attention*: The FMU model identifier must fulfill the restrictions for C function names!
* -s, --script: Path to |ns3| script (absolute or relative).

Optional input arguments
########################

* -h, --help: Display the help screen.
* -v, --verbose: Turn on log messages.
* -l, --litter: Do not clean-up intermediate files (e.g., log file with debug messages from compilation).
* -f, --fmi-version: Specify FMI version (1 or 2, default is 2)

Additional files may be specified (e.g., CSV input lists) that will be automatically copied to the FMU.
The specified files paths may be absolute or relative.

Start values for variables and parameters may be defined.
For instance, to set variable with name *var1* to value *12.34*, specify *var1=12.34* in the command line as optional argument.


Using an FMU generated for |ns3|
================================

During simulation, interaction with the FMU is basically limited to the use of three (types of) functions:

* *Setter functions*: Set the value (message ID) of input variables. This corresponds to sending a message. 0 means no input.

* *Getter functions*: Retrieve the value (message ID) of output variables. This corresponds to receiving a message. 0 means no output.

* *Synchronization*: An FMU for co-simulation is synchronized from one synchronization point to the next by calling the *doStep( ..., com_point, step_size, ...)* function, where *com_point* is the time of the last successful FMU synchronization and *step_size* is the length of the next simulation step.
  In the case of FMUs for |ns3|, which internally implement an event queue, there are three distinct ways of calling this function:
  
  1. *Time advance*: When doStep(...) is called with step_size > 0, then the FMU tries to advance its internal simulation time accordingly.

  2. *Receiving messages*: When calling doStep(...) with step_step = 0 (an FMU iteration) at a time corresponding to an internal event, the value(s) of the associated output variable(s) will be set to the according message ID.
     To retrieve the actual message ID, call the getter function *after* the FMU iteration.

  3. *Sending messages*: When calling doStep(...) with step_size = 0 (an FMU iteration) even though there is no internal event scheduled at this time, then the FMU assumes that one or more new messages have been sent and a new |ns3| simulation should be run.
     To trigger an |ns3| simulation, provide new message IDs via the setter functions directly before the FMU iteration (but after a time advance).



Examples
********

Example FMUs are available in the separate module *fmu-examples*.


Troubleshooting
***************

Nothing yet.