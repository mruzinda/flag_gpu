import subprocess
import os
import signal
import socket
import sys

def set_parameter():
    bank_list = ['A','B','C','D','E','F','G','H','I','J','K','L']
    proc = [None]*len(bank_list)
    hpc_host = ['inky','pinky','blinky']
    num_hpcs = len(hpc_host)
    num_instances = len(bank_list)/num_hpcs
    bash_prof = '~/.bash_profile'
    db_bash = '/home/onr_python/digital_backend/dibas.bash'
    user = os.getenv('USER')

    for i in range(0,num_hpcs):
        for x in range(0,num_instances):
            ssh_hpc = "ssh %s@%s "
            source_bash = "'source %s && source %s && "

            if len(sys.argv) > 2:
                if sys.argv[1] == 'REQSTI':
                    int_len_val = int(sys.argv[2])
                    int_length = "hashpipe_check_status -I %d -k REQSTI -f %f'"
                    cmd = ssh_hpc + source_bash + int_length
                    cmd = cmd % (user,hpc_host[i],bash_prof,db_bash,x,int_len_val)
                    print "Integration length changed to %f" % (int_len_val)
                if sys.argv[1] == 'BWEIFILE':
                    wfile = sys.argv[2] + bank_list[x+num_instances*i] + '.bin'
                    wflag = "hashpipe_check_status -I %d -k WFLAG -s 1 && " # Flag required to update weight file name.
                    weight_file = "hashpipe_check_status -I %d -k BWEIFILE -s %s'"
                    cmd = ssh_hpc + source_bash + wflag + weight_file
                    cmd = cmd % (user,hpc_host[i],bash_prof,db_bash,x,x,wfile)
                    print "Weight file name changed to %s" % (wfile)
            else:
                print "Please add arguments. REQSTI if running correlator or BWEIFILE if running beamformer followed by a value or file name respectively."

            print cmd
            proc[x+num_instances*i] = subprocess.Popen(cmd, shell=True)


if __name__ == "__main__":
    set_parameter()








