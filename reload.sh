#!/bin/bash

# Author: colayjwang
# Created Time: 2016-12-27 17:27:21
# Description: If TimingTask process doesn`t exist, try to reload it.Usually using a crontab mission to execute this.

all_pid=`ps -e | grep TimingTask | grep -v grep | gawk '{print $1}'`

echo "all_pid: "$all_pid

parent_id=`cat ./parent_pid`
echo "parent_id: "$parent_id

find_flag=0
for pid in $all_pid
do
    if [ $parent_id -eq $pid ]
    then
        echo "find parent_id: "$pid
        find_flag=1
        break
    fi
done

echo "find_flag: "$find_flag

if [ $find_flag -eq 0 ]
then
    echo "try to reload TimingTask!"
    killall TimingTask
    ./TimingTask &
else
    echo "do not need to reload TimingTask!"
fi

