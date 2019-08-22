import subprocess
import os
import signal

proc = [None]*4
fn = [None]*4

bank_list = ['A','B','C','D']
datadir = "/home/tmp_output"
weightdir = "/home/weight_files"
weightfile = "dummy_A.bin"
core_count = 0
for x in range(0,4):
    process_list = "hashpipe -p flag_b -I " + str(x) + " -o XID=" + str(x) + " -o INSTANCE=" + str(x) + " -o BANKNAM=" + bank_list[x] + " -o DATADIR=" + datadir + " -o BINDHOST=10.17.16." + str(x+4) + " -o WEIGHTD=" + weightdir + " -o BWEIFILE=" + weightfile + " -o BINDPORT=60000 -o GPUDEV=" + str(x%2) + " -c " + str(0+core_count) + " flag_net_thread -c " + str(1+core_count) + " flag_transpose_beamform_thread -c " + str(2+core_count) + " flag_beamsave_thread"
    core_count = core_count + 3 
    proc[x] = subprocess.Popen(process_list, shell=True, stdin=subprocess.PIPE)

for i in range(0,4):
    fn[i] = proc[i].stdin.fileno()




