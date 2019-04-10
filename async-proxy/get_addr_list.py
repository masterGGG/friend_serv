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

def pack():
    send_buf = struct.pack("=LLHLL", 18, 0, 0x3000, 0, 0)
    return send_buf

def unpack(recv_buf, body_unpack_f = ''):
    if len(recv_buf) < 18: raise Exception()
    head = struct.unpack("=LLHLL", recv_buf[:18])
    if head[0] > 18:
		addr_list = []
		num = struct.unpack('=L', recv_buf[18:22])
		for a in range(num[0]):
			begin = 22+(a*64)
			end = begin +64
			addr = struct.unpack('=64s', recv_buf[begin:end])
			addr_list.append(addr[0])

		return addr_list
    else:
        return None

if __name__ == '__main__':
    IP = 'test.service-config.taomee.com'
    PORT = 19155

    print unpack(send_data(pack(), IP, PORT))
