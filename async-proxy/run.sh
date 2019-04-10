#########################################################################
# File Name: run.sh
# Author: ian
# mail: ian@taomee.com
# Created Time: Wed Dec 26 22:56:52 2018
#########################################################################
#!/bin/bash
cd /home/ian/local_service/redis-5.0.3/src
if [ $1 = 0 ]
then
killall redis-server
else
./redis-server ../redis.conf &
fi

# relation server
cd /home/ian/workspace/relationship/async-proxy
if [ $1 = 0 ]
then
./server.sh stop 
#rm log/*
else
./server.sh start file
fi
