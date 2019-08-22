import subprocess
import os
import signal
import socket
import sys

class Backend:

    def __init__(self, master):
        self.proc = [None]*12
        self.fn = [None]*12
        #bank_list = ['A','B','C','D','E','F','G','H','I','J','K','L']
        self.last_byte = ['8','9','10','11','12','13','14','15','16','17','18','19']
        self.udp_port = 60000

    def start_instances(self):
        user = os.getenv('USER')
        for x in range(0,12):
            self.udp_ip = "10.17.16." + self.last_byte[x]
            if (x >= 0) and (x <= 3):
                #cmd = "ssh -f %s@blinky source ~/.bash_profile"
                #cmd = cmd % (user)
                cmd = "ssh -f blinky source ~/.bash_profile"
                #os.system(cmd)
                subprocess.Popen(cmd.split(" "))
                sock = socket.socket(socket.AF_INET, # Internet
                                     socket.SOCK_DGRAM) # UDP
                sock.bind((self.udp_ip, self.udp_port))
                proc_list, addr = sock.recvfrom(1024)
                self.proc[x] = subprocess.Popen(proc_list, shell=True, stdin=subprocess.PIPE)
                self.fn[x] = self.proc[x].stdin.fileno()

            if (x >= 4) and (x <= 7):
                cmd = "ssh -f pinky source ~/.bash_profile"
                #os.system(cmd)
                subprocess.Popen(cmd.split(" "))
                sock = socket.socket(socket.AF_INET, # Internet
                                     socket.SOCK_DGRAM) # UDP
                sock.bind((self.udp_ip, self.udp_port))
                proc_list, addr = sock.recvfrom(1024)
                self.proc[x] = subprocess.Popen(proc_list, shell=True, stdin=subprocess.PIPE)
                self.fn[x] = self.proc[x].stdin.fileno()

            if (x >= 8) and (x <= 11):
                cmd = "ssh -f inky source ~/.bash_profile"
                #os.system(cmd)
                subprocess.Popen(cmd.split(" "))
                sock = socket.socket(socket.AF_INET, # Internet
                                     socket.SOCK_DGRAM) # UDP
                sock.bind((self.udp_ip, self.udp_port))
                proc_list, addr = sock.recvfrom(1024)
                self.proc[x] = subprocess.Popen(proc_list, shell=True, stdin=subprocess.PIPE)
                self.fn[x] = self.proc[x].stdin.fileno()

if __name__ == "__main__":
    b = Backend()
    b.start_instances()


    #process_list = "hashpipe -p " + plugin_name  + " -I " + str(x%4) + " -o XID=" + str(x) + " -o INSTANCE=" + str(x%4) + " -o BANKNAM=" + bank_list[x] + " -o DATADIR=" + datadir + " -o BINDHOST=" + UDP_IP + " -o WEIGHTD=" + weightdir + " -o BWEIFILE=" + weightfile + " -o BINDPORT=60000 -o GPUDEV=" + str(x%2) + " -c " + str(0+core_count) + " flag_net_thread -c " + str(1+core_count) + " flag_transpose_beamform_thread -c " + str(2+core_count) + " flag_beamsave_thread"






