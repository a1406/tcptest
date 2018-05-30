#!/bin/sh
export ASAN_OPTIONS=detect_leaks=true:detect_stack_use_after_return=true:check_initialization_order=true:log_path=./memerr.log
export LSAN_OPTIONS=suppressions=../suppr.txt
cd `dirname $0`
filename="`pwd`/server_info.ini"

./stop.sh

ulimit -c unlimited

cd conn_srv
./conn_srv -d

cd ../game_srv
./game_srv -d

cd ..
#./show_all_pid.sh 
#./check_srv_alive.sh
