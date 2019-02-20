#!/usr/bin/python
# -*- coding: UTF-8 -*-

import socket

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

PORT = 8888

network = '<broadcast>'
network='192.168.199.45'
print(network)
strmsg=""
hostname = socket.gethostname()
sysinfo = socket.gethostbyname_ex(hostname)
print(sysinfo)
strmsg="[{}".format(hostname)
for ips in sysinfo[2]:
    strmsg=strmsg+"," + ips
strmsg=strmsg+"]"
print(strmsg)
s.sendto('你好，我是小家智控核心，这个是广播'.encode('utf-8'), (network, PORT))