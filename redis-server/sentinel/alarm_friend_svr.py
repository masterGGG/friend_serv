# -*- coding: utf-8 -*-

import os
import sys
import struct
import socket

HOST = "127.0.0.1"
PORT = 21145

to_ip = sys.argv[1]
to_port = (int)(sys.argv[2])
head_len = 18 + 4 + 4 + len(to_ip) # + 16 + 4

data = struct.pack("=IIHIIII%ds" % len(to_ip),
        head_len, 0, 0XA001, 0, 0,
        to_port, len(to_ip), to_ip)

try:
    svr = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    svr.connect((HOST, PORT))
except socket.error as e:
    raise "*** socket error:%s" % str(e)

if (isinstance(svr, str)) :
    print("ERR : server: %s" % svr)
else:
    svr.send(data)
    rcode = struct.unpack('=IIHII', svr.recv(18))[3]
    if rcode != 0:
        print("ERR : respose: %d" % rcode)
svr.close()
