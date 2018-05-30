#!/bin/sh
cd `dirname $0`
filename="`pwd`/server_info.ini"

# conn_srv退出
kill -9 `cat conn_srv/pid.txt`
# conn_srv退出
kill -9 `cat game/pid.txt`

# 清理共享内存
grep -F 'shm_addr' $filename | awk -F'=' '{print $NF}'|while read key;do ipcrm -M $key ;done
