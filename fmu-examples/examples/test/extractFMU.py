# -------------------------------------------------------------------
# Copyright (c) 2013-2017, AIT Austrian Institute of Technology GmbH.
# All rights reserved. See file FMIPP_LICENSE for details.
# -------------------------------------------------------------------

# Extract an FMU.
def extractFMU( fmuFilePath, outputDirPath, command = None ):
    '''Extract an FMU to a folder.
    
    fmuFilePath -- path to the FMU file (string)
    outputDirPath -- folder to which the FMU should be extracted (string)
    command (optional) -- specify the command to unzip the FMU (string)
    
    The command for unzipping should be given as a string, using tags '{fmu}'
    and '{dir}' as placeholders for the FMU file path and the output directory.
    For instance:
      - unzip: 'unzip {fmu} -d {dir}'
      - 7-zip: '"C:\\Program Files\\7-Zip\\7z.exe" -o{dir} x {fmu}'
    '''
    import os, zipfile, urllib.parse as urlparse, urllib.request as urllib

    # Check if specified file is indeed a zip file.
    if not zipfile.is_zipfile( fmuFilePath ):
        print( '%s is not a valid ZIP archive' % fmuFilePath )
        return

    # Check if output directory exists.
    if not os.path.isdir( outputDirPath ):
        print( '%s is not a valid path' % outputDirPath )
        return

    try:
        # Extract FMU file name from complete path.
        fmuFileName = os.path.split( fmuFilePath )[1];
        # Split FMU file name (name & extension).
        fmuSplitFileName = os.path.splitext( fmuFileName )
        
        # Check if this is a valid FMU file name.
        if '.fmu' != fmuSplitFileName[1]:
            print( '%s is not a valid FMU file name' % fmuFileName )

        # Extract model name from FMU file name.
        fmuModelName = fmuSplitFileName[0]

        # Create sub-directory in output directory.
        extractDirPath = os.path.join( outputDirPath, fmuModelName )
        try:
            os.mkdir( extractDirPath )
        except OSError: # Directory already exists
            print( 'directory already exists: %s' % extractDirPath )

        if command is None:
            # Access FMU.
            fmu = zipfile.ZipFile( fmuFilePath, 'r' )

            # Extract FMU to output directory.
            fmu.extractall( extractDirPath )
        else:
            os.system( command.format( fmu = fmuFilePath, dir = extractDirPath ) )
            
        # Return URI to extracted FMU.
        return urlparse.urljoin( 'file:', urllib.pathname2url( extractDirPath ) ) 
    except:
        print( 'failed to extract file: %s' % fmuFilePath )


if __name__ == '__main__':

    import sys

    if len( sys.argv ) != 3:
        print( 'Usage:\n\tpython extractFMU.py <path-to-fmu> <path-to-output-dir>\n' )
        sys.exit()

    fmuFilePath = sys.argv[1]
    outputDirPath = sys.argv[2]

    extractDirURI = extractFMU( fmuFilePath, outputDirPath )
    print( 'extracted FMU to: %s' % extractDirURI )
