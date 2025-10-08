#!/bin/bash

# 注意区分() 与 {}
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:../../extern/mysql-connector-c++/lib64/

echo "ld path : ${LD_LIBRARY_PATH}"

# 需要 sudo ./start.sh 执行，因为绑定的是80端口，需要root权限
nohup ./PersonalServer > output.log 2>&1 &

echo "start finish"