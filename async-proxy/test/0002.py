#!/usr/bin/python
#encoding=utf8
import socket, struct, hashlib, sys

def send_data(send_buf, ip = None, port = None, log = False):
    s = socket.socket(socket.AF_INET)
    s.connect((ip, port))
    if log: print("-------------NEW CONNECTION--------------")
    send_len = s.send(send_buf)
    if log: print("send_len:", send_len)
    recv_buf = s.recv(4)
    while len(recv_buf) < 4:
        recv_buf += s.recv(4 - len(recv_buf))
    recv_len = struct.unpack("=L", recv_buf[:4])[0]
    while len(recv_buf) < recv_len:
        recv_buf += s.recv(recv_len - len(recv_buf))
    if log: print("recv_len:", recv_len)
    s.close()
    return recv_buf

def pack(userid):
    body_buf = struct.pack("=L", userid)
    head_buf = struct.pack("=LLHLL", 18 + len(body_buf), 0, 0x0002, 0, userid)
    send_buf = head_buf
    send_buf += body_buf
    return send_buf

def unpack(recv_buf, body_unpack_f = ''):
    if len(recv_buf) < 18: raise Exception()
    head = struct.unpack("=LLHLL", recv_buf[:18])
    if head[0] > 18:
        #body = struct.unpack(body_unpack_f, recv_buf[18:])
        body1 = struct.unpack('=64s', recv_buf[18:82])
        body2 = struct.unpack('=64s', recv_buf[82:146])
        return str(head) + "\n" + str(body1) + "\n" + str(body2)
    else:
        body=()
        return str(head)

if __name__ == '__main__':
    IP = '127.0.0.1'
    PORT = 21140
    
    user_id = 12345
    #print unpack(send_data(pack(user_id), IP, PORT, True), '=64s64s')
    print unpack(send_data(pack(user_id), IP, PORT, True))
