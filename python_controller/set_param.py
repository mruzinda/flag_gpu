import subprocess
import sys
import os

def set_paramaters():
    param_val = sys.argv[4]
    for x in range(0,4):
        if sys.argv[3] == 'SCANLEN'
            cmd = 'hashpipe_check_status -I %s -k SCANLEN %d'
            cmd = cmd % (x,param_val)
            print cmd
            subprocess.Popen(cmd, shell=True)
        if sys.argv[3] == 'REQSTI':
            cmd = 'hashpipe_check_status -I %s -k REQSTI %d'
            cmd = cmd % (x,param_val)
            print cmd
            subprocess.Popen(cmd, shell=True)
        if sys.argv[3] == 'BWEIFILE':
            cmd = 'hashpipe_check_status -I %s -k BWEIFILE %s'
            cmd = cmd % (x,param_val)
            print cmd
            subprocess.Popen(cmd, shell=True)
 

if __name__ == "__main__":
    set_parameters()
