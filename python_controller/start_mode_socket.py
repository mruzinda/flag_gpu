import subprocess
import os
import signal
import socket
import sys
import time

# Class to write log files of the standard output of each hashpipe instance.
class LogFile:
    def __init__(self, logfile_name="", dir_name="/home/logFiles/"):
        self.name = logfile_name
        self.fhs = {}
        self.dir_name = dir_name
        self.extension = ".log"

        if not os.path.isdir(self.dir_name):
            os.mkdir(self.dir_name)

    def write(self, value):
        f = self.getFID()
        f.write(value)
        f.close()

    def getFID(self):
        f = self.fhs.get(self.name)
        fname = self.name

        if f is None:
            fname = self.dir_name + fname + self.extension
            f = open(fname, "a")
            self.fhs[fname] = f

        return f

    def fileno(self):
        f = self.getFID()
        return f.fileno()

# Function to start hashpipe instances on each machine.
def start_instances():

    # Initializations that will go to config file:
    bank_list = ['A','B','C','D','E','F','G','H','I','J','K','L']
    last_byte = ['8','9','10','11','12','13','14','15','16','17','18','19']
    instances_per_cpu = len(bank_list) # 12
    proc = None
    fh = None
    UDP_PORT = 60000
    UDP_PORT1 = 60001
    datadir = "/home/tmp_output"
    weightdir = "/home/weight_files"
    weightfile = "dummy_w_A.bin"
    log_Dir = "/home/logFiles/"

    # Determine which algorithm needs to be run:
    if sys.argv[1] == 'CORR':
        plugin_name = 'flag_x'
        cores_per_instance = 4
        cores_per_cpu = 16
    else:
        plugin_name = 'flag_b'
        cores_per_instance = 3
        cores_per_cpu = 12

    # Additional initialisations
    user = os.getenv('USER')
    bank = sys.argv[2]
    print bank

    x = bank_list.index(bank) # Hashpipe instance index used to determine correct IP address
    bank_name = 'BANK' + bank_list[x]
    UDP_IP = "10.17.16." + last_byte[x]
    
    # Process data on appropriate cores, write stdout to log files and prepare stdin for reading
    if sys.argv[1] == 'CORR':
        core_count = (cores_per_instance*x)%cores_per_cpu
        proc_list = "hashpipe -p " + plugin_name + " -I " + str(x%4) + " -o XID=" + str(x) + " -o INSTANCE=" + str(x%4) + " -o BANKNAM=" + bank_list[x] + " -o DATADIR=" + datadir + " -o BINDHOST=" + UDP_IP + " -o BINDPORT=60000 -o GPUDEV=" + str(x%2) + " -c " + str(0+core_count) + " flag_net_thread -c " + str(1+core_count) + " flag_transpose_thread -c " + str(2+core_count) + " flag_correlator_thread -c" + str(3+core_count) + " flag_corsave_thread"
        print proc_list
        proc = subprocess.Popen(proc_list, shell=True, stdin=subprocess.PIPE, stdout=LogFile(logfile_name=bank_name, dir_name=log_Dir))
        fh = proc.stdin.fileno()
    else:
        core_count = (cores_per_instance*x)%cores_per_cpu
        proc_list = "hashpipe -p " + plugin_name  + " -I " + str(x%4) + " -o XID=" + str(x) + " -o INSTANCE=" + str(x%4) + " -o BANKNAM=" + bank_list[x] + " -o DATADIR=" + datadir + " -o BINDHOST=" + UDP_IP + " -o WEIGHTD=" + weightdir + " -o BWEIFILE=" + weightfile + " -o BINDPORT=60000 -o GPUDEV=" + str(x%2) + " -c " + str(0+core_count) + " flag_net_thread -c " + str(1+core_count) + " flag_transpose_beamform_thread -c " + str(2+core_count) + " flag_beamsave_thread"
        print proc_list
        proc = subprocess.Popen(proc_list, shell=True, stdin=subprocess.PIPE, stdout=LogFile(logfile_name=bank_name, dir_name=log_Dir))
        fh = proc.stdin.fileno()

    # Socket initializations
    port_idx = bank_list.index(bank) # Finds the index of the specified bank
    UDP_IP = "10.17.16." + last_byte[port_idx]
    sock = socket.socket(socket.AF_INET, # Internet
                         socket.SOCK_DGRAM) # UDP
    sock.bind((UDP_IP,UDP_PORT1))
  
    #time.sleep(10) # Wait while hashpipe is starting up 
    #print "%s: Send command (set parameters or START, STOP or QUIT) when ready." % (bank_list[x]) 
    # When the command is received from command_instances.py, write it to stdin of the instance
    while True:
        cmd_proc, addr = sock.recvfrom(1024)
        print cmd_proc
        os.write(fh, cmd_proc+'\n')
        time.sleep(5)
        if cmd_proc == 'QUIT':
            print cmd_proc
            break

if __name__ == "__main__":
    start_instances()








