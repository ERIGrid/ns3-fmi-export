#!/bin/sh

export CXXFLAGS="-D_USE_MATH_DEFINES -D_BSD_SOURCE -include limits.h"

# Retrieve path to directory containing this script.
SCRIPT_DIR="$(dirname $(readlink -f $0))"

${SCRIPT_DIR}/../../fmi-export/ns3_fmu_create.py -v -m SimpleFMU -s scratch/SimpleFMU.cc -f 1 channel_delay=0.2
${SCRIPT_DIR}/../../fmi-export/ns3_fmu_create.py -v -m TC3 -s scratch/TC3.cc -f 1

python3 ${SCRIPT_DIR}/test/testSimpleFMU.py
python3 ${SCRIPT_DIR}/test/testTC3.py
