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

    if sys.argv[1] == 'CORR':
        plugin_name = 'flag_x'
    else:
        plugin_name = 'flag_b'

    user = os.getenv('USER')
    cores_per_instance = 3
    cores_per_cpu = 12
    for x in range(0,12):
        UDP_IP = "10.17.16." + last_byte[x]
        if sys.argv[2] == bank_list[x]:
            #core_count = (3*x)%12
            core_count = (cores_per_instance*x)%cores_per_cpu
            #if (x == 0):
            #    core_count = 0
            #if (x == 4):
            #    core_count = 0
            #if (x == 8):
            #    core_count = 0
           
            proc_list = "hashpipe -p " + plugin_name  + " -I " + str(x%4) + " -o XID=" + str(x) + " -o INSTANCE=" + str(x%4) + " -o BANKNAM=" + bank_list[x] + " -o DATADIR=" + datadir + " -o BINDHOST=" + UDP_IP + " -o WEIGHTD=" + weightdir + " -o BWEIFILE=" + weightfile + " -o BINDPORT=60000 -o GPUDEV=" + str(x%2) + " -c " + str(0+core_count) + " flag_net_thread -c " + str(1+core_count) + " flag_transpose_beamform_thread -c " + str(2+core_count) + " flag_beamsave_thread"
            print proc_list
            proc[x] = subprocess.Popen(proc_list, shell=True, stdin=subprocess.PIPE)
            fn[x] = proc[x].stdin.fileno()

if __name__ == "__main__":
    start_instances()








