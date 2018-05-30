#!/bin/sh
export ASAN_OPTIONS=detect_leaks=true:detect_stack_use_after_return=true:check_initialization_order=true:log_path=./memerr.log
export LSAN_OPTIONS=suppressions=../suppr.txt
cd `dirname $0`
filename="`pwd`/server_info.ini"

# conn_srv退出
kill -9 `cat conn_srv/pid.txt`
# conn_srv退出
kill -9 `cat game/pid.txt`

# 清理共享内存
grep -F 'shm' $filename | awk -F'=' '{print $NF}'|while read key;do ipcrm -M $key ;done

ulimit -c unlimited

cd conn_srv
./conn_srv -d

cd ../game_srv
./game_srv -d

cd ..
#./show_all_pid.sh 
#./check_srv_alive.sh
