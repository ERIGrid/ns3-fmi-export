# -----------------------------------------------------------------
# Copyright (c) 2018, AIT Austrian Institute of Technology GmbH.
# -----------------------------------------------------------------

#
# Collection of helper functions for creating FMU CS according to FMI 1.0
#


# Get templates for the XML model description depending on the FMI version.
def fmi1GetModelDescriptionTemplates( verbose, modules ):
    # Template string for XML model description header.
    header = '<?xml version="1.0" encoding="UTF-8"?>\n<fmiModelDescription fmiVersion="1.0" modelName="__MODEL_NAME__" modelIdentifier="__MODEL_IDENTIFIER__" description="NS3 FMI CS export" generationTool="FMI++ NS3 Export Utility" generationDateAndTime="__DATE_AND_TIME__" variableNamingConvention="flat" numberOfContinuousStates="0" numberOfEventIndicators="0" author="__USER__" guid="{__GUID__}">\n\t<VendorAnnotations>\n\t\t<Tool name="waf">\n\t\t\t<Executable preArguments="--run" arguments="__SCRIPT_NAME__" executableURI="__WAF_URI__"/>\n\t\t</Tool>\n\t</VendorAnnotations>\n\t<ModelVariables>\n'

    # Template string for XML model description of scalar variables.
    scalar_variable_node = '\t\t<ScalarVariable name="__VAR_NAME__" valueReference="__VAL_REF__" variability="__VARIABILITY__" causality="__CAUSALITY__">\n\t\t\t<__VAR_TYPE____START_VALUE__/>\n\t\t</ScalarVariable>\n'

    # Template string for XML model description footer.
    footer = '\t</ModelVariables>\n\t<Implementation>\n\t\t<CoSimulation_Tool>\n\t\t\t<Capabilities canHandleVariableCommunicationStepSize="true" canHandleEvents="true" canRejectSteps="false" canInterpolateInputs="false" maxOutputDerivativeOrder="0" canRunAsynchronuously="false" canBeInstantiatedOnlyOncePerProcess="true" canNotUseMemoryManagementFunctions="true"/>\n\t\t\t<Model entryPoint="" manualStart="false" type="application/x-waf">__ADDITIONAL_FILES__\n\t\t\t</Model>\n\t\t</CoSimulation_Tool>\n\t</Implementation>\n</fmiModelDescription>'

    return ( header, scalar_variable_node, footer )


# Add URI to waf.
def fmi1AddWafUriToModelDescription( waf_uri, header, footer, verbose, modules ):
    header = header.replace( '__WAF_URI__', waf_uri )
    return ( header, footer )


# Add name of script as input argument to waf.
def fmi1AddScriptToModelDescription( script_name, header, footer, vebose, modules ):
    header = header.replace( '__SCRIPT_NAME__', script_name )
    return ( header, footer )


# Add optional files to XML model description.
def fmi1AddOptionalFilesToModelDescription( optional_files, header, footer, verbose, modules ):
    if ( 0 == len( optional_files ) ):
        footer = footer.replace( '__ADDITIONAL_FILES__', '' )
    else:
        additional_files_description = ''
        indent = '\n\t\t\t'

        for file_name in optional_files:
            additional_files_description += indent + '\t<File file=\"fmu://resources/' + modules.os.path.basename( file_name ) + '\"/>'
            if ( True == verbose ): modules.log( '[DEBUG] Added additional file to model description: ', modules.os.path.basename( file_name ) )
        additional_files_description += indent

        footer = footer.replace( '__ADDITIONAL_FILES__', additional_files_description )

    return ( header, footer )


# Create DLL for FMU.
def fmi1CreateSharedLibrary( fmi_model_identifier, ns3_fmu_root_dir, fmipp_include_dir, fmipp_lib_dir, verbose, modules ):
    # Define name of shared library.
    fmu_shared_library_name = str()
    build_script_name = str()

    platform_id = modules.platform.platform().lower()
    if 'linux' in platform_id:
        fmu_shared_library_name = fmi_model_identifier + '.so'
        build_script_name = 'fmi1_build.sh'
    elif 'cygwin' in platform_id:
        fmu_shared_library_name = fmi_model_identifier + '.dll'
        build_script_name = 'fmi1_build_cygwin.sh'

    # Check if batch file for build process exists.
    build_process_batch_file = modules.os.path.join( ns3_fmu_root_dir, 'scripts', build_script_name )
    if ( False == modules.os.path.isfile( build_process_batch_file ) ):
        modules.log( '\n[ERROR] Could not find file: ', build_process_batch_file )
        raise Exception( 8 )
    # Remove possible leftovers from previous builds.
    for file_name in modules.glob.glob( fmi_model_identifier + '.*' ):
        modules.os.remove( file_name )
    if ( True == modules.os.path.isfile( 'fmiFunctions.obj' ) ): modules.os.remove( 'fmiFunctions.obj' )
    # Compile FMU shared library.
    build_process = modules.subprocess.Popen(
        [ build_process_batch_file, fmi_model_identifier, fmipp_include_dir, fmipp_lib_dir ] )
    stdout, stderr = build_process.communicate()

    if ( False == modules.os.path.isfile( fmu_shared_library_name ) ):
        modules.log( '\n[ERROR] Not able to create shared library: ', fmu_shared_library_name )
        raise Exception( 17 )

    return fmu_shared_library_name


# Retrieve variability of scalar variable from JSON-file label.
def fmi1GetScalarVariableVariability( label, modules ):
    for label_type in [ 'Inputs', 'Outputs' ]:
        if label_type in label:
            return 'discrete' # all inputs/outputs to ns-3 FMUs are of type 'discrete'
    return 'parameter'


# Retrieve causality of scalar variable from JSON-file label.
def fmi1GetScalarVariableCausality( label, modules ):
    if 'Inputs' in label:
        return 'input'
    elif 'Outputs' in label:
        return 'output'
    else:
        return 'internal'
