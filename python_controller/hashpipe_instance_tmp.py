import subprocess
import os
import signal
import socket
import sys

def start_instances():
    proc = [None]*12
    fn = [None]*12
    bank_list = ['A','B','C','D','E','F','G','H','I','J','K','L']
    last_byte = ['8','9','10','11','12','13','14','15','16','17','18','19']
    UDP_PORT = 60000
    datadir = "/home/tmp_output"
    weightdir = "/home/weight_files"
    weightfile = "dummy_A.bin"
    core_count = 0

    if sys.argv[0] == 'CORR':
        plugin_name = 'flag_x'
    else:
        plugin_name = 'flag_b'

    user = os.getenv('USER')
    for x in range(0,12):
        UDP_IP = "10.17.16." + last_byte[x]
        if (x >= 0) and (x <= 3):
            if x == 0:
                core_count = 0
                
            cmd = "ssh -t %s@inky source ~/.bash_profile;"
            cmd = cmd % (user)
            print cmd
            #cmd = "ssh -n blinky source ~/.bash_profile"
            #cmd = "ssh -n blinky"
            #os.system(cmd)
            #subprocess.Popen(cmd.split(" "))

            #sock = socket.socket(socket.AF_INET, # Internet
            #                     socket.SOCK_DGRAM) # UDP
            #sock.bind((UDP_IP, UDP_PORT))
            #proc_list, addr = sock.recvfrom(1024)
            proc_list = cmd + " 'hashpipe -p " + plugin_name  + " -I " + str(x%4) + " -o XID=" + str(x) + " -o INSTANCE=" + str(x%4) + " -o BANKNAM=" + bank_list[x] + " -o DATADIR=" + datadir + " -o BINDHOST=" + UDP_IP + " -o WEIGHTD=" + weightdir + " -o BWEIFILE=" + weightfile + " -o BINDPORT=60000 -o GPUDEV=" + str(x%2) + " -c " + str(0+core_count) + " flag_net_thread -c " + str(1+core_count) + " flag_transpose_beamform_thread -c " + str(2+core_count) + " flag_beamsave_thread'"
            core_count = core_count + 3
            proc[x] = subprocess.Popen(proc_list, shell=True, stdin=subprocess.PIPE)
            fn[x] = proc[x].stdin.fileno()

        if (x >= 4) and (x <= 7):
            if x == 4:
                core_count = 0

            #cmd = "ssh -n pinky source ~/.bash_profile"
            cmd = "ssh -t %s@pinky source ~/.bash_profile;"
            cmd = cmd % (user)
            print cmd
            #os.system(cmd)
            #subprocess.Popen(cmd.split(" "))
            #sock = socket.socket(socket.AF_INET, # Internet
            #                     socket.SOCK_DGRAM) # UDP
            #sock.bind((UDP_IP, UDP_PORT))
            #proc_list, addr = sock.recvfrom(1024)
            proc_list = cmd + " 'hashpipe -p " + plugin_name  + " -I " + str(x%4) + " -o XID=" + str(x) + " -o INSTANCE=" + str(x%4) + " -o BANKNAM=" + bank_list[x] + " -o DATADIR=" + datadir + " -o BINDHOST=" + UDP_IP + " -o WEIGHTD=" + weightdir + " -o BWEIFILE=" + weightfile + " -o BINDPORT=60000 -o GPUDEV=" + str(x%2) + " -c " + str(0+core_count) + " flag_net_thread -c " + str(1+core_count) + " flag_transpose_beamform_thread -c " + str(2+core_count) + " flag_beamsave_thread'"
            core_count = core_count + 3
            proc[x] = subprocess.Popen(proc_list, shell=True, stdin=subprocess.PIPE)
            fn[x] = proc[x].stdin.fileno()
        if (x >= 8) and (x <= 11):
            if x == 8:
                core_count = 0

            #cmd = "ssh -n inky source ~/.bash_profile"
            cmd = "ssh -t %s@blinky source ~/.bash_profile;"
            cmd = cmd % (user)
            print cmd
            #os.system(cmd)
            #subprocess.Popen(cmd.split(" "))

            #sock = socket.socket(socket.AF_INET, # Internet
            #                     socket.SOCK_DGRAM) # UDP
            #sock.bind((UDP_IP, UDP_PORT))
            #proc_list, addr = sock.recvfrom(1024)
            proc_list = cmd + " 'hashpipe -p " + plugin_name  + " -I " + str(x%4) + " -o XID=" + str(x) + " -o INSTANCE=" + str(x%4) + " -o BANKNAM=" + bank_list[x] + " -o DATADIR=" + datadir + " -o BINDHOST=" + UDP_IP + " -o WEIGHTD=" + weightdir + " -o BWEIFILE=" + weightfile + " -o BINDPORT=60000 -o GPUDEV=" + str(x%2) + " -c " + str(0+core_count) + " flag_net_thread -c " + str(1+core_count) + " flag_transpose_beamform_thread -c " + str(2+core_count) + " flag_beamsave_thread'"
            core_count = core_count + 3
            proc[x] = subprocess.Popen(proc_list, shell=True, stdin=subprocess.PIPE)
            fn[x] = proc[x].stdin.fileno()


if __name__ == "__main__":
    start_instances()


    #process_list = "hashpipe -p " + plugin_name  + " -I " + str(x%4) + " -o XID=" + str(x) + " -o INSTANCE=" + str(x%4) + " -o BANKNAM=" + bank_list[x] + " -o DATADIR=" + datadir + " -o BINDHOST=" + UDP_IP + " -o WEIGHTD=" + weightdir + " -o BWEIFILE=" + weightfile + " -o BINDPORT=60000 -o GPUDEV=" + str(x%2) + " -c " + str(0+core_count) + " flag_net_thread -c " + str(1+core_count) + " flag_transpose_beamform_thread -c " + str(2+core_count) + " flag_beamsave_thread"






