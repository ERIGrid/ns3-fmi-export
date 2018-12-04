#!/bin/sh

#-----------------------------------------------------------------
# Copyright (c) 2018, AIT Austrian Institute of Technology GmbH.
#-----------------------------------------------------------------

# Check number of input arguments.
if [ "$#" -ne 3 ]; then
    echo "USAGE: fmi1_build.sh <fmi-model-identifier> <fmipp-include-dir> <fmipp-lib-dir>"
    exit 1
fi

# Define FMU model identifier.
MODEL_IDENTFIER=$1

# Define log file name.
LOG_FILE=build.log

# Delete debug file if it already exists.
if [ -e "$LOG_FILE" ]; then
    rm $LOG_FILE
fi

# Define FMI export functions implementation file.
FMI_FUNCTIONS_IMPLEMENTATION="${2}/export/functions/fmi_v1.0/fmiFunctions.cpp"

# Define include flags for compiler.
INCLUDE_FLAGS="-I${2} -I${2}/export/functions/fmi_v1.0"

# Define BOOST libraries for static linking.
BOOST_LIBS="-lboost_date_time -lboost_filesystem -lboost_system"

# Define compiler and linker.
COMPILER=g++

# Compile FMI front end component with correct model identifier.
${COMPILER} ${INCLUDE_FLAGS} -c -O2 -fPIC -DMODEL_IDENTIFIER=${MODEL_IDENTFIER} -DFRONT_END_TYPE=FMIComponentFrontEnd -DFRONT_END_TYPE_INCLUDE=\"export/include/FMIComponentFrontEnd.h\" ${FMI_FUNCTIONS_IMPLEMENTATION}
>> ${LOG_FILE}

# Compile final shared library for FMU.
${COMPILER} fmiFunctions.o ${3}/libfmipp_frontend.a ${BOOST_LIBS} -L/lib -lrt -pthread -shared -fPIC -o ${MODEL_IDENTFIER}.so >> ${LOG_FILE}
