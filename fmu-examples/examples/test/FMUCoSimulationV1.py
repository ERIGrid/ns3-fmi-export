from ctypes import *
import sys
import os.path
import urllib.parse as urlparse, urllib.request as urllib
import xml.etree.ElementTree


def py_logger( c, instance_name, status, category, message ):
    #if not status is FMUCoSimulationV1.fmi_ok:
    print( '[{}] {}: {}'.format(
        instance_name.decode( 'utf-8' ),
        category.decode( 'utf-8' ),
        message.decode( 'utf-8' )
        ) )

def py_step_finished( c, status ):
    pass

def py_allocate_memory( nobj, size ):
    return 0

def py_free_memory( obj ):
    pass


class FMICallbackFunctionsV1( Structure ):

    logger_callback_prototype = CFUNCTYPE(
        None, # return type: void
        c_void_p, # fmiComponent c
        c_char_p, # fmiString instanceName
        c_int, # fmiStatus status
        c_char_p, # fmiString category
        c_char_p # fmiString message
        )

    step_finished_callback_prototype = CFUNCTYPE(
        None, # return type: void
        c_void_p, # fmiComponent c
        c_int # fmiStatus status
        )

    allocate_memory_callback_prototype = CFUNCTYPE(
        c_void_p, # return type: void*
        c_size_t, # size_t nobj
        c_size_t #size_t size
        )

    free_memory_callback_prototype = CFUNCTYPE(
        None, # return type: void
        c_void_p # void* obj
        )

    _fields_ = [
        ( 'logger', logger_callback_prototype ),
        ( 'stepFinished', step_finished_callback_prototype ),
        ( 'allocateMemory', allocate_memory_callback_prototype ),
        ( 'freeMemory', free_memory_callback_prototype )
        ]


class FMUCoSimulationV1:

    fmi_true = b'1'
    fmi_false = b'0'

    fmi_ok = 0
    fmi_warning = 1
    fmi_discard = 2
    fmi_error = 3
    fmi_fatal = 4
    fmi_pending = 5


    def __init__( self, fmu_name, fmu_path ):

        self.fmu_name = fmu_name
        self.fmu_path = fmu_path

        # Load the FMU shared library.
        self.__load_shared_library()

        # Set callback functions.
        self.__set_callbacks()

        # Read the model description.
        self.__parse_model_description()

        # Initialize the FMU functions.
        self.__init_functions()


    def __del__( self ):
        self.terminateSlave()
        self.freeSlaveInstance()


    def __load_shared_library( self ):
        if not os.path.isdir( self.fmu_path ):
            raise IOError( 'directory not found: {}'.format( self.fmu_path ) )

        # Construct path to shared library from FMU name and path.
        if sys.platform == 'cygwin':
            fmu_shared_library_path = \
                os.path.join( self.fmu_path, self.fmu_name, 'binaries', 'cygwin32', self.fmu_name + '.dll' )
        else:
            fmu_shared_library_path = \
                os.path.join( self.fmu_path, self.fmu_name, 'binaries', 'linux64', self.fmu_name + '.so' )

        # Load shared library.
        self.fmu_shared_library = cdll.LoadLibrary( fmu_shared_library_path )


    def __set_callbacks( self ):
        # Set callback functions.
        self.callback_functions = FMICallbackFunctionsV1(
            FMICallbackFunctionsV1.logger_callback_prototype( py_logger ),
            FMICallbackFunctionsV1.step_finished_callback_prototype( py_step_finished ),
            FMICallbackFunctionsV1.allocate_memory_callback_prototype( py_allocate_memory ),
            FMICallbackFunctionsV1.free_memory_callback_prototype( py_free_memory )
        )


    def __parse_model_description( self ):
        # Parse and store the model description.
        fmu_model_description_path = os.path.join( self.fmu_path, self.fmu_name, 'modelDescription.xml' )
        self.fmu_model_description = xml.etree.ElementTree.parse( fmu_model_description_path ).getroot()

        # Retrieve dict of variable names and value references.
        scalar_variables = self.fmu_model_description.find( 'ModelVariables' ).getchildren()
        self.fmu_var_dict = dict()
        for var in scalar_variables:
            self.fmu_var_dict[ var.get( 'name' ) ] = int( var.get( 'valueReference' ) )


    def __init_functions( self ):
        # Specify function fmiGetVersion
        func_name_get_version = self.fmu_name + '_fmiGetVersion'
        func_get_version = getattr( self.fmu_shared_library, func_name_get_version )
        func_get_version.restype = c_char_p

        # Specify function fmiGetTypesPlatform
        func_name_types_get_platform = self.fmu_name + '_fmiGetTypesPlatform'
        func_types_get_platform = getattr( self.fmu_shared_library, func_name_types_get_platform )
        func_types_get_platform.restype = c_char_p

        # Specify function fmiInstantiateSlave
        func_name_instantiate_slave = self.fmu_name + '_fmiInstantiateSlave'
        func_instantiate_slave = getattr( self.fmu_shared_library, func_name_instantiate_slave )
        func_instantiate_slave.restype = c_void_p # fmiComponent
        func_instantiate_slave.argtypes = (
            c_char_p, # fmiString instanceName
            c_char_p, # fmiString fmuGUID
            c_char_p, # fmiString fmuLocation
            c_char_p, # fmiString mimeType
            c_double, # fmiReal timeout
            c_char, # fmiBoolean visible
            c_char, # fmiBoolean interactive
            FMICallbackFunctionsV1, # fmiCallbackFunctions functions
            c_char # fmiBoolean loggingOn
            )

        # Specify function fmiInitializeSlave
        func_name_initialize_slave = self.fmu_name + '_fmiInitializeSlave'
        func_initialize_slave = getattr( self.fmu_shared_library, func_name_initialize_slave )
        func_initialize_slave.restype = c_int # fmiStatus
        func_initialize_slave.argtypes = (
            c_void_p, # fmiComponent c
            c_double, # fmiReal tStart
            c_char, # fmiBoolean StopTimeDefined
            c_double # fmiReal tStop
            )

        # Specify function fmiTerminateSlave
        func_name_terminate_slave = self.fmu_name + '_fmiTerminateSlave'
        func_terminate_slave = getattr( self.fmu_shared_library, func_name_terminate_slave )
        func_terminate_slave.restype = c_int # fmiStatus
        func_terminate_slave.argtypes = (
            c_void_p, # fmiComponent c
            )

        # Specify function fmiFreeSlaveInstance
        func_name_free_slave_instance = self.fmu_name + '_fmiFreeSlaveInstance'
        func_free_slave_instance = getattr( self.fmu_shared_library, func_name_free_slave_instance )
        func_free_slave_instance.restype = None # void
        func_free_slave_instance.argtypes = (
            c_void_p, # fmiComponent c
            )

        # Specify function fmiSetReal
        func_name_set_real = self.fmu_name + '_fmiSetReal'
        func_set_real = getattr( self.fmu_shared_library, func_name_set_real )
        func_set_real.restype = c_int # fmiStatus
        func_set_real.argtypes = (
            c_void_p, # fmiComponent c
            POINTER( c_int ), # const fmiValueReference vr[]
            c_size_t, # size_t nvr
            POINTER( c_double ) # const fmiReal value[]
            )

        # Specify function fmiGetReal
        func_name_get_real = self.fmu_name + '_fmiGetReal'
        func_get_real = getattr( self.fmu_shared_library, func_name_get_real )
        func_get_real.restype = c_int # fmiStatus
        func_get_real.argtypes = (
            c_void_p, # fmiComponent c
            POINTER( c_int ), # const fmiValueReference vr[]
            c_size_t, # size_t nvr
            POINTER( c_double ) # const fmiReal value[]
            )

        # Specify function fmiSetInteger
        func_name_set_integer = self.fmu_name + '_fmiSetInteger'
        func_set_integer = getattr( self.fmu_shared_library, func_name_set_integer )
        func_set_integer.restype = c_int # fmiStatus
        func_set_integer.argtypes = (
            c_void_p, # fmiComponent c
            POINTER( c_int ), # const fmiValueReference vr[]
            c_size_t, # size_t nvr
            POINTER( c_int ) # const fmiInteger value[]
            )

        # Specify function fmiGetInteger
        func_name_get_integer = self.fmu_name + '_fmiGetInteger'
        func_get_integer = getattr( self.fmu_shared_library, func_name_get_integer )
        func_get_integer.restype = c_int # fmiStatus
        func_get_integer.argtypes = (
            c_void_p, # fmiComponent c
            POINTER( c_int ), # const fmiValueReference vr[]
            c_size_t, # size_t nvr
            POINTER( c_int ) # const fmiInteger value[]
            )

        # Specify function fmiDoStep
        func_name_do_step = self.fmu_name + '_fmiDoStep'
        func_do_step = getattr( self.fmu_shared_library, func_name_do_step )
        func_do_step.restype = c_int # fmiStatus
        func_do_step.argtypes = (
            c_void_p, # fmiComponent c
            c_double, # fmiReal currentCommunicationPoint
            c_double, # fmiReal communicationStepSize
            c_char # fmiBoolean newStep
            )


    def getVersion( self ):
        func_name_get_version = self.fmu_name + '_fmiGetVersion'
        func_get_version = getattr( self.fmu_shared_library, func_name_get_version )
        return func_get_version()


    def getTypesPlatform( self ):
        func_name_types_get_platform = self.fmu_name + '_fmiGetTypesPlatform'
        func_types_get_platform = getattr( self.fmu_shared_library, func_name_types_get_platform )
        return func_types_get_platform()


    def instantiateSlave( self, name, timeout = 0., visible = False, interactive = False, logging_on = False ):
        fmu_uri = urlparse.urljoin( 'file:',
            urllib.pathname2url( os.path.abspath( os.path.join( self.fmu_path, self.fmu_name ) ) )
            )

        fmu_guid = self.fmu_model_description.get( 'guid' )

        model_info = self.fmu_model_description.find( 'Implementation/CoSimulation_Tool/Model' )
        if model_info is None:
            raise RuntimeError( 'XML model description has no element called "Implementation/CoSimulation_Tool/Model"' )
        fmu_mime_type = model_info.get( 'type' )

        func_name_instantiate_slave = self.fmu_name + '_fmiInstantiateSlave'
        func_instantiate_slave = getattr( self.fmu_shared_library, func_name_instantiate_slave )
        self.fmi_component = func_instantiate_slave(
            c_char_p( name.encode( 'utf-8' ) ),
            c_char_p( fmu_guid.encode( 'utf-8' ) ),
            c_char_p( fmu_uri.encode( 'utf-8' ) ),
            c_char_p( fmu_mime_type.encode('utf-8' ) ),
            c_double( timeout ),
            c_char( self.fmi_true if visible is True else self.fmi_false ),
            c_char( self.fmi_true if interactive is True else self.fmi_false ),
            self.callback_functions,
            c_char( self.fmi_true if logging_on is True else self.fmi_false )
            )


    def initializeSlave( self, start_time, stop_time_defined = False, stop_time = 0. ):
        func_name_initialize_slave = self.fmu_name + '_fmiInitializeSlave'
        func_initialize_slave = getattr( self.fmu_shared_library, func_name_initialize_slave )
        status = func_initialize_slave(
            self.fmi_component,
            c_double( start_time ),
            c_char( self.fmi_true if stop_time_defined is True else self.fmi_false ),
            c_double( stop_time )
            )

        assert( status == self.fmi_ok  )


    def getReal( self, var_names ):
        # Get the number of variables.
        n_vars = len( var_names )

        # Retrieve the value references.
        var_ref_ids = []
        for name in var_names:
            var_ref_ids.append( self.fmu_var_dict[ name ] )

        var_values = ( c_double * n_vars )()

        # Call FMU function.
        func_name_get_real = self.fmu_name + '_fmiGetReal'
        func_get_real = getattr( self.fmu_shared_library, func_name_get_real )
        status = func_get_real(
            self.fmi_component,
            ( c_int * n_vars )( *var_ref_ids ),
            c_size_t( n_vars ),
            var_values
            )

        # Check the FMU status.
        assert( status == self.fmi_ok  )

        return list( var_values )


    def setReal( self, var_names, var_values ):
        # Get the number of variables.
        n_vars = len( var_names )

        # Retrieve the value references.
        var_ref_ids = []
        for name in var_names:
            var_ref_ids.append( self.fmu_var_dict[ name ] )

        # Call FMU function.
        func_name_set_real = self.fmu_name + '_fmiSetReal'
        func_set_real = getattr( self.fmu_shared_library, func_name_set_real )
        status = func_set_real(
            self.fmi_component,
            ( c_int * n_vars )( *var_ref_ids ),
            c_size_t( n_vars ),
            ( c_double * n_vars )( *var_values )
            )

        # Check the FMU status.
        assert( status == self.fmi_ok  )


    def getInteger( self, var_names ):
        # Get the number of variables.
        n_vars = len( var_names )

        # Retrieve the value references.
        var_ref_ids = []
        for name in var_names:
            var_ref_ids.append( self.fmu_var_dict[ name ] )

        var_values = ( c_int * n_vars )()

        # Call FMU function.
        func_name_get_integer = self.fmu_name + '_fmiGetInteger'
        func_get_integer = getattr( self.fmu_shared_library, func_name_get_integer )
        status = func_get_integer(
            self.fmi_component,
            ( c_int * n_vars )( *var_ref_ids ),
            c_size_t( n_vars ),
            var_values
            )

        # Check the FMU status.
        assert( status == self.fmi_ok  )

        return list( var_values )


    def setInteger( self, var_names, var_values ):
        # Get the number of variables.
        n_vars = len( var_names )

        # Retrieve the value references.
        var_ref_ids = []
        for name in var_names:
            var_ref_ids.append( self.fmu_var_dict[ name ] )

        # Call FMU function.
        func_name_set_integer = self.fmu_name + '_fmiSetInteger'
        func_set_integer = getattr( self.fmu_shared_library, func_name_set_integer )
        status = func_set_integer(
            self.fmi_component,
            ( c_int * n_vars )( *var_ref_ids ),
            c_size_t( n_vars ),
            ( c_int * n_vars )( *var_values )
            )

        # Check the FMU status.
        assert( status == self.fmi_ok  )


    def doStep( self, current_communication_point, communication_step_size, new_step = True ):

        func_name_do_step = self.fmu_name + '_fmiDoStep'
        func_do_step = getattr( self.fmu_shared_library, func_name_do_step )
        status = func_do_step(
            self.fmi_component,
            c_double( current_communication_point ),
            c_double( communication_step_size ),
            c_char( self.fmi_true if new_step is True else self.fmi_false )
            )

        # Check the FMU status.
        assert( status == self.fmi_ok  )


    def terminateSlave( self ):

        func_name_terminate_slave = self.fmu_name + '_fmiTerminateSlave'
        func_terminate_slave = getattr( self.fmu_shared_library, func_name_terminate_slave )
        status = func_terminate_slave(
            self.fmi_component
            )

        # Check the FMU status.
        assert( status == self.fmi_ok  )


    def freeSlaveInstance( self ):

        func_name_free_slave_instance = self.fmu_name + '_fmiFreeSlaveInstance'
        func_free_slave_instance = getattr( self.fmu_shared_library, func_name_free_slave_instance )
        func_free_slave_instance(
            self.fmi_component
            )

        self.fmi_component = None
