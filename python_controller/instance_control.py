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
    mode_name = ''

    if len(sys.argv) > 1:
        mode_name = sys.argv[1]

    print mode_name
    user = os.getenv('USER')
    hpc_idx = range(0,num_hpcs)
    instance_list = range(0,num_instances)

    # Selects which HPCs will run the hashpipe processes as well as the selected instances.
    if len(sys.argv) > 4:
        if sys.argv[4] == 'inky':
            hpc_idx = [0]
            if len(sys.argv) > 5:
                instance_idx = sys.argv[5]
                instance_list = map(int, instance_idx.split(','))# map() converts index in list from string to int
        elif sys.argv[4] == 'pinky':
            hpc_idx = [1]
            if len(sys.argv) > 5:
                instance_idx = sys.argv[5]
                instance_list = map(int, instance_idx.split(','))# map() converts index in list from string to int
        elif sys.argv[4] == 'blinky':
            hpc_idx = [2]
            if len(sys.argv) > 5:
                instance_idx = sys.argv[5]
                instance_list = map(int, instance_idx.split(','))# map() converts index in list from string to int
 
    for i in hpc_idx:
        for x in instance_list:
            ssh_hpc = "ssh %s@%s "
            source_bash = "'source %s && source %s && "
            clean_shmem = "hashpipe_clean_shmem -I %d && "
            start_mode = "python %s/start_mode_socket.py %s %s'"

            if len(sys.argv) > 2:
                scan_len_val = int(sys.argv[3])
                print scan_len_val
                scan_length = "hashpipe_check_status -I %d -k SCANLEN -f %d && "
                cmd = ssh_hpc + source_bash + clean_shmem + scan_length + start_mode
                cmd = cmd % (user,hpc_host[i],bash_prof,db_bash,x,x,scan_len_val,py_dir,mode_name,bank_list[x+num_instances*i])
            else:
                cmd = ssh_hpc + source_bash + clean_shmem + start_mode
                cmd = cmd % (user,hpc_host[i],bash_prof,db_bash,x,py_dir,mode_name,bank_list[x+num_instances*i])

            print cmd
            proc[x+num_instances*i] = subprocess.Popen(cmd, shell=True)


if __name__ == "__main__":
    remote_and_run()








