#########################################################################
# File Name: stop.sh
# Author: ian
# mail: ian@taomee.com
# Created Time: Tue 19 Mar 2019 10:27:15 AM CST
#########################################################################
#!/bin/bash

cd master/ && ../redis-server redis.conf && cd ../

cd slave/ && ../redis-server redis.conf && cd ../

cd sentinel && ../redis-sentinel sentinel.conf && cd ../
