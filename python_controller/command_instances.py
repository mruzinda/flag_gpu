import subprocess
import os
import sys
import socket

bank_list = ['A','B','C','D','E','F','G','H','I','J','K','L']
last_byte = ['8','9','10','11','12','13','14','15','16','17','18','19']
total_instances = len(bank_list)
UDP_PORT = 60001
cmd = sys.argv[1]
print cmd

for x in range(0,total_instances):
#for x in range(0,2):
    UDP_IP = "10.17.16." + last_byte[x]
    #UDP_PORT = 60000 + x
    sock = socket.socket(socket.AF_INET, # Internet
                         socket.SOCK_DGRAM) # UDP
    sock.sendto(cmd, (UDP_IP, UDP_PORT))
