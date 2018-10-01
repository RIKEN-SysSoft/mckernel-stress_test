#!/bin/sh

# WORKDIR is defined in /work/mcktest/bin/config.sh
MCKDIR=${WORKDIR}/mck

#TIMEOUT=180
TIMEOUT=60

#MCREBOOTOPTION="-k 1 -f LOG_LOCAL6  -c 1-7,9-15,17-23,25-31 -m 1G@0,1G@1 -r 1-7:0+9-15:8+17-23:16+25-31:24 -o root"
MCREBOOTOPTION="-k 0 -f LOG_LOCAL6  -c 1-7,9-15,17-23,25-31 -m 1G@0,1G@1 -r 1-7:0+9-15:8+17-23:16+25-31:24 -o root"

