#########################################################################
# File Name: stop.sh
# Author: ian
# mail: ian@taomee.com
# Created Time: Tue 19 Mar 2019 10:27:15 AM CST
#########################################################################
#!/bin/bash


ps aux | grep redis-server | grep $1 | awk '{print $2}' | xargs kill
