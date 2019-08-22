import subprocess
import os
import signal
import socket
import sys

def remote_and_run():
    proc = [None]*12
    bank_list = ['A','B','C','D','E','F','G','H','I','J','K','L']
    py_dir = '~/onr_gpu/python_controller'
    bash_prof = '~/.bash_profile'
    db_bash = '/home/onr_python/digital_backend/dibas.bash'

    if len(sys.argv) > 1:
        mode_name = sys.argv[1]
    else:
        mode_name = None

    print mode_name
    user = os.getenv('USER')
    for x in range(0,12):
        if (x >= 0) and (x <= 3):
            cmd = "ssh %s@inky 'source %s && source %s && python %s/start_mode.py %s %s'"
            cmd = cmd % (user,bash_prof,db_bash,py_dir,mode_name,bank_list[x])
            print cmd
            proc[x] = subprocess.Popen(cmd, shell=True)

        if (x >= 4) and (x <= 7):
            cmd = "ssh %s@pinky 'source %s && source %s && python %s/start_mode.py %s %s'"
            cmd = cmd % (user,bash_prof,db_bash,py_dir,mode_name,bank_list[x])
            print cmd
            proc[x] = subprocess.Popen(cmd, shell=True)

        if (x >= 8) and (x <= 11):
            cmd = "ssh %s@blinky 'source %s && source %s && python %s/start_mode.py %s %s'"
            cmd = cmd % (user,bash_prof,db_bash,py_dir,mode_name,bank_list[x])
            print cmd
            proc[x] = subprocess.Popen(cmd, shell=True)


if __name__ == "__main__":
    remote_and_run()








