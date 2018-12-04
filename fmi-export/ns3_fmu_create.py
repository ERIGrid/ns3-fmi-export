#!/usr/bin/python

# -----------------------------------------------------------------
# Copyright (c) 2018, AIT Austrian Institute of Technology GmbH.
# -----------------------------------------------------------------

#
# This file is used to create FMUs for CoSimulation from ns-3 applications.
# 

# Setup for Python 2.
try:
    import sys, os, shutil, time, getpass, uuid, getopt, json, subprocess, glob, argparse, urlparse, urllib, collections, platform
except:
    pass

# Setup for Python 3.
try:
    import sys, os, shutil, time, getpass, uuid, getopt, json, subprocess, glob, argparse, urllib.parse as urlparse, urllib.request as urllib, collections, platform
except:
    pass

def log( *arg ):
    print( ' '.join( map( str, arg ) ) )
    sys.stdout.flush()


from scripts.utils import *
from scripts.generate_fmu import *


def main( ns3_fmu_root_dir = os.path.dirname( __file__ ), parser = None ):

    # Create container for all used Python modules, which will be passed to all called functions.
    # This makes it easier to run this script with different Python version (2.x and 3.x).
    Modules = collections.namedtuple( 'Modules', [ 'sys', 'os', 'shutil', 'time', 'getpass', 'uuid', 'urlparse', 'urllib', 'getopt', 'json', 'subprocess', 'glob', 'argparse', 'log', 'platform' ] )
    modules = Modules( sys, os, shutil, time, getpass, uuid, urlparse, urllib, getopt, json, subprocess, glob, argparse, log, platform )

    # Retrieve parsed command line arguments.
    cmd_line_args = parseCommandLineArguments( modules ) if ( parser == None ) else parser()

    # FMI model identifier.
    fmi_model_identifier = cmd_line_args.model_id

    # ns-3 script file name.
    script_file_path = cmd_line_args.script

    # Set NS3 install dir.
    ns3_install_dir = cmd_line_args.ns3_install_dir

    # Verbose flag.
    verbose = cmd_line_args.verbose

    # Litter flag.
    litter = cmd_line_args.litter

    # FMI version
    fmi_version = cmd_line_args.fmi_version
    if ( True == verbose ): modules.log( '[DEBUG] Using FMI version', fmi_version )

    # Check if specified ns-3 script exists.
    if ( False == os.path.isfile( script_file_path ) ):
        modules.log( '\n[ERROR] Invalid ns-3 script: ', script_file_path )
        sys.exit(4)

    # Retrieve start values and additional files from command line arguments.
    ( optional_files, start_values ) = parseAdditionalInputs( cmd_line_args.extra_arguments, verbose, modules  )

    # Retrieve information saved during waf configuration.
    ns3_config_file_name = modules.os.path.join( ns3_fmu_root_dir, 'fmi_export_conf.json' )
    if ( False == os.path.isfile( ns3_config_file_name ) ):
            modules.log( '\n[ERROR] Please run \'./waf configure --with-fmippex=<fmipp-root-dir>\'.' )
            modules.sys.exit(5)
    ns3_config = modules.json.load( open( ns3_config_file_name ) )
    
    # No ns-3 install directory provided -> read from config file.
    if ( None == ns3_install_dir ):
        ns3_install_dir = ns3_config[ 'NS3_ROOT_DIR' ]

    # Retriebe information about FMI++ include path and libraries directory.
    fmipp_include_dir = ns3_config[ 'FMIPP_INCLUDE_PATH' ]
    fmipp_lib_dir = ns3_config[ 'FMIPP_LIB_PATH' ]

    # Check if specified NS3 install directory exists.
    if ( False == modules.os.path.isdir( ns3_install_dir ) ):
        modules.log( '\n[ERROR] ns-3 install directory does not exist: ', ns3_install_dir )
        modules.sys.exit(5)

    # Copy the script to ns-3's scratch directory, compile it and
    # retrieve FMI input/output variable names from the script.
    ( script_name, fmi_input_vars, fmi_output_vars, fmi_params ) = \
        prepareNs3Script( script_file_path, ns3_install_dir, verbose, modules )

    if ( True == verbose ):
        modules.log( '[DEBUG] FMI model identifier: ', fmi_model_identifier )
        modules.log( '[DEBUG] ns-3 script: ', script_file_path )
        modules.log( '[DEBUG] ns-3 install directory: ', ns3_install_dir )
        modules.log( '[DEBUG] Aditional files: ' )
        for file_name in optional_files:
            modules.log( '\t', file_name )

    try:
        fmu_name = generateNs3FMU(
            script_name,
            fmi_version,
            fmi_model_identifier,
            fmi_input_vars,
            fmi_output_vars,
            fmi_params,
            start_values,
            optional_files,
            ns3_fmu_root_dir,
            ns3_install_dir,
            fmipp_include_dir,
            fmipp_lib_dir,
            verbose,
            litter,
            modules )

        if ( True == verbose ): modules.log( "[DEBUG] FMU created successfully:", fmu_name )

    except Exception as e:
        modules.log( e )
        modules.sys.exit( e.args[0] )

# Main function
if __name__ == "__main__":
    main()
