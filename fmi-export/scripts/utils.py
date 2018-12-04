# -----------------------------------------------------------------
# Copyright (c) 2018, AIT Austrian Institute of Technology GmbH.
# -----------------------------------------------------------------

#
# Collection of helper functions for creating FMU CS from dedicated ns-3 scripts.
#

# Parse command line arguments.
def parseCommandLineArguments( modules ):
    # Create new parser.
    parser = modules.argparse.ArgumentParser( description = 'Generate FMUs for Co-Simulation (tool coupling) for dedicated ns-3 scripts.', prog = 'ns3_fmu_create' )

    # Define optional arguments.
    parser.add_argument( '-v', '--verbose', action = 'store_true', help = 'turn on log messages' )
    parser.add_argument( '-l', '--litter', action = 'store_true', help = 'do not clean-up intermediate files' )
    parser.add_argument( '-t', '--ns3-install-dir', default = None, help = 'path to ns-3 installation directory', metavar = 'NS3-INSTALL-DIR' )
    parser.add_argument( '-f', '--fmi-version', choices = [ '1', '2' ], default = '2', help = 'specify FMI version (default: 2)' )

    # Define mandatory arguments.
    required_args = parser.add_argument_group( 'required arguments' )
    required_args.add_argument( '-m', '--model-id', required = True, help = 'specify FMU model identifier', metavar = 'MODEL-ID' )
    required_args.add_argument( '-s', '--script', required = True, help = 'path to ns-3 script', metavar = 'PATH_TO_SCRIPT' )

    # Parse remaining optional arguments (start values, additional files).
    parser.add_argument( 'extra_arguments', nargs = '*', default = None, help = 'extra files and/or start values', metavar = 'additional arguments' )

    # Add help for additional files.
    parser.add_argument_group( 'additional files', 'Additional files may be specified as extra arguments. These files will be automatically copied to the resources directory of the FMU.' )

    # Add help for start values.
    parser.add_argument_group( 'start values', 'Specify start values for FMU input variables and parameters.' )

    return parser.parse_args()


# Parse additional command line inputs (start values, additional files).
def parseAdditionalInputs( extra_arguments, verbose, modules ):
    # List of optional files (e.g., weather file)
    optional_files = []

    # Dictionary of start values.
    start_values = {}

    # Retrieve additional files from command line arguments.
    if extra_arguments != None:
        for item in extra_arguments:
            if '=' in item:
                start_value_pair = item.split( '=' )
                varname = start_value_pair[0].strip(' "\n\t')
                value = start_value_pair[1].strip(' "\n\t')
                if ( True == verbose ): modules.log( '[DEBUG] Found start value: ', varname, '=', value )
                start_values[varname] = value;
            elif ( True == modules.os.path.isfile( item ) ): # Check if this is an additional input file.
                optional_files.append( item )
                if ( True == verbose ): modules.log( '[DEBUG] Found additional file: ', item )
            else:
                modules.log( '\n[ERROR] Invalid input argument: ', item )
                modules.sys.exit(7)

    return ( optional_files, start_values )


# Parse NS3 deck file.
def prepareNs3Script( script_file_path, ns3_install_dir, verbose, modules ):
    # Lists containing the FMI input and output variable names.
    fmi_input_vars = {}
    fmi_output_vars = {}
    fmi_params = {}

    # Define path to ns-3's scratch directory.
    ns3_scratch_dir = modules.os.path.join( ns3_install_dir, 'scratch' )

    # Copy script to ns-3's scratch directory.
    modules.shutil.copy( script_file_path, ns3_scratch_dir )

    # Define command for compiling the ns-3 script.
    script_path, script_name = modules.os.path.split( script_file_path )
    script_name_root, script_name_ext = modules.os.path.splitext( script_name )    
    compile_script_cmd = './waf build'

    # Compile shared libraries from FMI++ code.
    exit_code = modules.subprocess.call( compile_script_cmd, shell=True, cwd=ns3_install_dir )
    if ( 0 != exit_code ): # Compilation failed.
        modules.log( '[ERROR] compilation of script failed:', script_file_path )
        modules.sys.exit(8)
    elif ( True == verbose ):
        modules.log( '[DEBUG] successfully compiled ns-3 script' )
        
    # Define command for executing the ns-3 script (producing a JSON
    # file listing the names input/output variables and parameter).
    run_script_cmd = './waf --run "{0} --only-write-variable-names-json"'.format( script_name_root )
    json_file_path = modules.os.path.join( ns3_install_dir, 'build', 'scratch', script_name_root + '.json' )

    # Execute the script.
    exit_code = modules.subprocess.call( run_script_cmd, shell=True, cwd=ns3_install_dir )
    if ( 0 != exit_code ): # Execution failed.
        modules.log( '[ERROR] generation of JSON file failed (execution failed)' )
        modules.sys.exit(9)
    elif ( False == modules.os.path.isfile( json_file_path ) ):
        modules.log( '[ERROR] generation of JSON file failed (file not found)' )
        modules.sys.exit(9)
    elif ( True == verbose ):
        modules.log( '[DEBUG] successfully created JSON script' )


    json_data = modules.json.load( open( json_file_path ) )

    input_labels = [ 'RealInputs', 'IntegerInputs', 'BooleanInputs', 'StringInputs' ]
    output_labels = [ 'RealOutputs', 'IntegerOutputs', 'BooleanOutputs', 'StringOutputs' ]
    param_labels = [ 'RealParameters', 'IntegerParameters', 'BooleanParameters', 'StringParameters' ]

    for label, value in json_data.items():
        if label in input_labels:
            fmi_input_vars[ str( label ) ] = map( str, value )
        elif label in output_labels:
            fmi_output_vars[ str( label ) ] = map( str, value )
        elif label in param_labels:
            fmi_params[ str( label ) ] = map( str, value )

    return ( script_name_root, fmi_input_vars, fmi_output_vars, fmi_params )
