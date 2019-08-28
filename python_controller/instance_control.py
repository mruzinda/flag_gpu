import subprocess
import os
import signal
import socket
import sys

def remote_and_run():
    bank_list = ['A','B','C','D','E','F','G','H','I','J','K','L']
    proc = [None]*len(bank_list)
    hpc_host = ['inky','pinky','blinky']
    num_hpcs = len(hpc_host)
    num_instances = len(bank_list)/num_hpcs
    py_dir = '~/onr_gpu/python_controller'
    bash_prof = '~/.bash_profile'
    db_bash = '/home/onr_python/digital_backend/dibas.bash'

    if len(sys.argv) > 1:
        mode_name = sys.argv[1]
    else:
        mode_name = None

    print mode_name
    user = os.getenv('USER')

    for i in range(0,num_hpcs):
        for x in range(0,num_instances):
            cmd = "ssh %s@%s 'source %s && source %s && python %s/start_mode_socket.py %s %s'"
            cmd = cmd % (user,hpc_host[i],bash_prof,db_bash,py_dir,mode_name,bank_list[x+num_instances*i])
            print cmd
            proc[x+num_instances*i] = subprocess.Popen(cmd, shell=True)


if __name__ == "__main__":
    remote_and_run()








