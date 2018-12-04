#!/bin/sh

export CXXFLAGS="-D_USE_MATH_DEFINES -D_BSD_SOURCE -include limits.h"

./../../fmi-export/ns3_fmu_create.py -v -m SimpleFMU -s scratch/SimpleFMU.cc -f 1 channel_delay=0.2
./../../fmi-export/ns3_fmu_create.py -v -m TC3 -s scratch/TC3.cc -f 1
./../../fmi-export/ns3_fmu_create.py -v -m LSS2 -s scratch/LSS2.cc -f 1 max_jitter=0.01

cd test

python testSimpleFMU.py
python testTC3.py
python testLSS2.py

cd ..
