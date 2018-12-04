# -----------------------------------------------------------------
# Copyright (c) 2018, AIT Austrian Institute of Technology GmbH.
# -----------------------------------------------------------------

#
# Collection of helper functions for creating FMU CS according to FMI 2.0 for ns-3
#

### Import helper functions for specific FMI versions.
from .fmi1 import *
from .fmi2 import *


def generateNs3FMU( script_name,
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
    modules ) :
    """Generate an FMU for ns-3.

    Keyword arguments:
        script_name -- name of ns-3 script (string)
        fmi_version -- FMI version (string)
        fmi_model_identifier -- FMI model identfier for FMU (string)
        fmi_input_vars -- definition of input variable names (dict: key = type, value = name)
        fmi_output_vars -- definition of output variable names (dict: key = type, value = name)
        fmi_params -- definition of output variable names (dict: key = type, value = name)
        start_values -- definition of start values (map of strings to strings)
        optional_files -- definition of additional files (list of strings)
        ns3_fmu_root_dir -- path to installed fmippex ns-3 module (string)
        ns3_install_dir -- ns-3 installation directory (string)
        fmipp_include_dir -- path to installed FMI++ source directory (string)
        fmipp_lib_dir -- path to compiled FMI++ libraries (string)
        verbose -- verbosity flag (boolean)
        litter -- do not clean-up intermediate files (boolean)
        modules -- named tuple containing all imported modules
    """

    # Create FMU model description.
    model_description_name = \
        createModelDescription( fmi_version, fmi_model_identifier, script_name, ns3_install_dir,
            fmi_input_vars, fmi_output_vars, fmi_params, start_values, optional_files, verbose, modules )

    # Create FMU shared library.
    fmu_shared_library_name = createSharedLibrary( fmi_model_identifier, fmi_version, ns3_fmu_root_dir,
        fmipp_include_dir, fmipp_lib_dir, verbose, modules )

    # Check if working directory for FMU creation already exists.
    if ( True == modules.os.path.isdir( fmi_model_identifier ) ):
        modules.shutil.rmtree( fmi_model_identifier, False )

    # Retrieve platform information.
    platform_type = str()
    platform_id = modules.platform.platform().lower()
    platform_bits = modules.platform.architecture()[0][:2]
    if 'linux' in platform_id:
        platform_type = 'linux' + platform_bits
    elif 'cygwin' in platform_id:
        platform_type = 'cygwin' + platform_bits
    elif 'windows' in platform_id:
        platform_type = 'win' + platform_bits
    else:
        modules.log( '\n[ERROR] platform not supported: ', modules.platform.platform() )
        modules.sys.exit(6)

    # Working directory path for the FMU DLL.
    binaries_dir = modules.os.path.join( fmi_model_identifier, 'binaries', platform_type )

    # Create working directory (incl. sub-directories) for FMU creation.
    modules.os.makedirs( binaries_dir )

    # Resources directory path.
    resources_dir = modules.os.path.join( fmi_model_identifier, 'resources' )

    # Create resources directory for FMU creation.
    modules.os.makedirs( resources_dir )

    # Copy all files to working directory.
    modules.shutil.copy( model_description_name, fmi_model_identifier ) # XML model description.
    for file_name in optional_files: # Additional files.
        modules.shutil.copy( file_name, resources_dir )
    modules.shutil.copy( fmu_shared_library_name, binaries_dir ) # FMU DLL.

    # Create ZIP archive.
    if ( True == modules.os.path.isfile( fmi_model_identifier + '.zip' ) ):
        modules.os.remove( fmi_model_identifier + '.zip' )
    modules.shutil.make_archive( fmi_model_identifier, 'zip', fmi_model_identifier )

    # Finally, create the FMU!!!
    fmu_file_name = fmi_model_identifier + '.fmu'
    if ( True == modules.os.path.isfile( fmu_file_name ) ):
        modules.os.remove( fmu_file_name )
    modules.os.rename( fmi_model_identifier + '.zip', fmu_file_name )

    # Clean up.
    if ( False == litter ):
        for fn in [ model_description_name, 'build.log', 'fmiFunctions.o' ]:
            modules.os.remove( fn ) if modules.os.path.isfile( fn ) else None
        modules.shutil.rmtree( fmi_model_identifier, False )
        for file_name in modules.glob.glob( fmi_model_identifier + '.*' ):
            if not ( ( ".fmu" in file_name ) or ( ".~" in file_name ) ): modules.os.remove( file_name )

    # Return name of created FMU.
    return fmu_file_name


# Create model description.
def createModelDescription(
        fmi_version,
        fmi_model_identifier,
        script_name,
        ns3_install_dir,
        fmi_input_vars,
        fmi_output_vars,
        fmi_params,
        start_values,
        optional_files,
        verbose,
        modules ):

    # Retrieve templates for different parts of XML model description according to FMI version.
    ( model_description_header, scalar_variable_node, model_description_footer ) = getModelDescriptionTemplates( fmi_version, verbose, modules )

    # FMI model identifier.
    model_description_header = model_description_header.replace( '__MODEL_IDENTIFIER__', fmi_model_identifier )

    # Model name (just use the name of the script).
    fmi_model_name = script_name

    model_description_header = model_description_header.replace( '__MODEL_NAME__', fmi_model_name )

    # Creation date and time.
    model_description_header = model_description_header.replace( '__DATE_AND_TIME__', modules.time.strftime( "%Y-%m-%dT%H:%M:%S" ) )

    # Author name.
    model_description_header = model_description_header.replace( '__USER__', modules.getpass.getuser() )

    # GUID.
    model_description_header = model_description_header.replace( '__GUID__', str( modules.uuid.uuid1() ) )

    # URI of ns-3 main executable (waf).
    waf_uri = modules.urlparse.urljoin( 'file:', modules.urllib.pathname2url( ns3_install_dir ) ) + '/waf'
    ( model_description_header, model_description_footer ) = \
        addWafUriToModelDescription( waf_uri, model_description_header, model_description_footer, fmi_version, verbose, modules )

    # Define a string to collect all scalar variable definitions.
    model_description_scalars = ''

    # Add scalar input variables description. Value references for inputs start with 1.
    input_val_ref = 1
    for var_type, var_list in fmi_input_vars.items():
        for var in var_list:
            scalar_variable_description = scalar_variable_node
            scalar_variable_description = scalar_variable_description.replace( '__VAR_TYPE__', getScalarVariableType( var_type ) )
            scalar_variable_description = scalar_variable_description.replace( '__VARIABILITY__', getScalarVariableVariability( var_type, fmi_version, modules ) )
            scalar_variable_description = scalar_variable_description.replace( '__VAR_NAME__', var )
            scalar_variable_description = scalar_variable_description.replace( '__CAUSALITY__', "input" )
            scalar_variable_description = scalar_variable_description.replace( '__VAL_REF__', str( input_val_ref ) )
            scalar_variable_description = scalar_variable_description.replace( '__INITIAL__', '' )
            if var in start_values:
                start_value_description = ' start=\"' + start_values[var] + '\"'
                scalar_variable_description = scalar_variable_description.replace( '__START_VALUE__', start_value_description )
                if ( True == verbose ): modules.log( '[DEBUG] Added start value to model description: ', var, '=', start_values[var] )
            else:
                scalar_variable_description = scalar_variable_description.replace( '__START_VALUE__', '' )
            input_val_ref += 1
            # Write scalar variable description to file.
            model_description_scalars += scalar_variable_description;

    # Add scalar output variables description. Value references for outputs start with 1001 (except there are already input value references with corresponding values).
    output_val_ref = 1001 if ( input_val_ref < 1001 ) else input_val_ref
    for var_type, var_list in fmi_output_vars.items():
        for var in var_list:
            scalar_variable_description = scalar_variable_node
            scalar_variable_description = scalar_variable_description.replace( '__VAR_TYPE__', getScalarVariableType( var_type ) )
            scalar_variable_description = scalar_variable_description.replace( '__VARIABILITY__', getScalarVariableVariability( var_type, fmi_version, modules ) )
            scalar_variable_description = scalar_variable_description.replace( '__VAR_NAME__', var )
            scalar_variable_description = scalar_variable_description.replace( '__CAUSALITY__', "output" )
            scalar_variable_description = scalar_variable_description.replace( '__VAL_REF__', str( output_val_ref ) )
            if var in start_values:
                start_value_description = ' start=\"' + start_values[var] + '\"'
                scalar_variable_description = scalar_variable_description.replace( '__START_VALUE__', start_value_description )
                scalar_variable_description = scalar_variable_description.replace( '__INITIAL__', 'initial="exact"' )
                if ( True == verbose ): modules.log( '[DEBUG] Added start value to model description: ', var, '=', start_values[var] )
            else:
                scalar_variable_description = scalar_variable_description.replace( '__START_VALUE__', '' )
                scalar_variable_description = scalar_variable_description.replace( '__INITIAL__', '' )
            output_val_ref += 1
            # Write scalar variable description to file.
            model_description_scalars += scalar_variable_description;

    # Add scalar parameter description. Value references for inputs start with 2001.
    param_val_ref = 2001 if ( output_val_ref < 2001 ) else output_val_ref
    for var_type, var_list in fmi_params.items():
        for var in var_list:
            scalar_variable_description = scalar_variable_node
            scalar_variable_description = scalar_variable_description.replace( '__VAR_TYPE__', getScalarVariableType( var_type ) )
            scalar_variable_description = scalar_variable_description.replace( '__VARIABILITY__', getScalarVariableVariability( var_type, fmi_version, modules ) )
            scalar_variable_description = scalar_variable_description.replace( '__VAR_NAME__', var )
            scalar_variable_description = scalar_variable_description.replace( '__CAUSALITY__', getScalarVariableCausality( var_type, fmi_version, modules ) )
            scalar_variable_description = scalar_variable_description.replace( '__VAL_REF__', str( param_val_ref ) )
            scalar_variable_description = scalar_variable_description.replace( '__INITIAL__', '' )
            if var in start_values:
                start_value_description = ' start=\"' + start_values[var] + '\"'
                scalar_variable_description = scalar_variable_description.replace( '__START_VALUE__', start_value_description )
                if ( True == verbose ): modules.log( '[DEBUG] Added start value to model description: ', var, '=', start_values[var] )
            else:
                scalar_variable_description = scalar_variable_description.replace( '__START_VALUE__', '' )
            param_val_ref += 1
            # Write scalar variable description to file.
            model_description_scalars += scalar_variable_description;

    # Add name of script as input argument to waf.
    ( model_description_header, model_description_footer ) = \
        addScriptToModelDescription( script_name, model_description_header, model_description_footer, fmi_version, verbose, modules )

    # Optional files.
    ( model_description_header, model_description_footer ) = \
        addOptionalFilesToModelDescription( model_description_header, model_description_footer, optional_files, fmi_version, verbose, modules)

    # Create new XML model description file.
    model_description_name = 'modelDescription.xml'
    model_description = open( model_description_name, 'w' )
    model_description.write( model_description_header );
    model_description.write( model_description_scalars );
    model_description.write( model_description_footer );
    model_description.close()

    return model_description_name


# Get templates for the XML model description depending on the FMI version.
def getModelDescriptionTemplates( fmi_version, verbose, modules ):
    if ( '1' == fmi_version ): # FMI 1.0
       return fmi1GetModelDescriptionTemplates( verbose, modules )
    elif ( '2' == fmi_version ): # FMI 2.0
        return fmi2GetModelDescriptionTemplates( verbose, modules )


# Add URI to waf.
def addWafUriToModelDescription( waf_uri, header, footer, fmi_version, verbose, modules ):
    if ( '1' == fmi_version ): # FMI 1.0
        return fmi1AddWafUriToModelDescription( waf_uri, header, footer, verbose, modules )
    elif ( '2' == fmi_version ): # FMI 2.0
        return fmi2AddWafUriToModelDescription( waf_uri, header, footer, verbose, modules )


# Add name of script as input argument to waf.
def addScriptToModelDescription( script_name, header, footer, fmi_version, verbose, modules ):
    if ( '1' == fmi_version ): # FMI 1.0
        return fmi1AddScriptToModelDescription( script_name, header, footer, verbose, modules )
    elif ( '2' == fmi_version ): # FMI 2.0
        return fmi2AddScriptToModelDescription( script_name, header, footer, verbose, modules )


# Add optional files to XML model description.
def addOptionalFilesToModelDescription( header, footer, optional_files, fmi_version, verbose, modules ):
    if ( '1' == fmi_version ):
        return fmi1AddOptionalFilesToModelDescription( optional_files, header, footer, verbose, modules )
    if ( '2' == fmi_version ):
        return fmi2AddOptionalFilesToModelDescription( optional_files, header, footer, verbose, modules )


# Create DLL for FMU.
def createSharedLibrary( fmi_model_identifier, fmi_version, ns3_fmu_root_dir, fmipp_include_dir, fmipp_lib_dir, verbose, modules ):
    if ( '1' == fmi_version ):
        return fmi1CreateSharedLibrary( fmi_model_identifier, ns3_fmu_root_dir, fmipp_include_dir, fmipp_lib_dir, verbose, modules )
    if ( '2' == fmi_version ):
        return fmi2CreateSharedLibrary( fmi_model_identifier, ns3_fmu_root_dir, verbose, modules )


# Retrieve variability of scalar variable from JSON-file label.
def getScalarVariableVariability( label, fmi_version, modules ):
    if ( '1' == fmi_version ):
        return fmi1GetScalarVariableVariability( label, modules )
    if ( '2' == fmi_version ):
        return fmi2GetScalarVariableVariability( label, modules )


# Retrieve causality of scalar variable from JSON-file label.
def getScalarVariableCausality( label, fmi_version, modules ):
    if ( '1' == fmi_version ):
        return fmi1GetScalarVariableCausality( label, modules )
    if ( '2' == fmi_version ):
        return fmi2GetScalarVariableCausality( label, modules )


# Retrieve type of scalar variable from JSON-file label.
def getScalarVariableType( label ):
    for label_type in [ 'Inputs', 'Parameters', 'Outputs' ]:
        if label_type in label:
            return label[ 0 : -len( label_type ) ]
