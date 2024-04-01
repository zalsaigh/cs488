#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Usage: $0 <luastub>"
    exit 1
fi

LUASTUB="$1"

make && cd Assets && ../A4 ${LUASTUB}.lua  && cd output && ./animate.sh ${LUASTUB}