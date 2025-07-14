#!/bin/bash

export LD_LIBRARY_PATH=${pwd}/lib:${LD_LIBRARY_PATH}

echo "ld path : ${LD_LIBRARY_PATH}"

sudo ./PersonalServer