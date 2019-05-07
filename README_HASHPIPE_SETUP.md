These are the commands needed to setup and configure everything for hashpipe:

- 'source /home/onr_python/digital_backend/dibas.bash'

- In the ~/onr_gpu/src: 'sudo /opt/local/bin/libtoolize --force'

- aclocal

- Perform the 'libtoolize' and 'aclocal' command as seen above in ~/onr_gpu/lib/hashpipe/src

-Then run the install script in ~/onr_gpu/: './install'

For more details on the commands in the install script, go over README_GB.md. 

Now, your executables have been installed and you can run hashpipe.

And example of the hashpipe command is:

'hashpipe -p flag_x -o XID=0 INSTANCE=0 -o BANKNAM=A -o DATADIR=/home/tmp_output -o BINDHOST=10.17.16.1 -o BINDPORT=60000 -o GPUDEV=0 -c 0 flag_net_thread -c 1 flag_transpose_thread -c 2 flag_correlator_thread -c 3 flag_corsave_thread'

To change a parameter before you run hashpipe, use:

'hashpipe_check_status -k SCANLEN -f 30' - The changes the SCANLEN keyword to 30which means that it is a 30 second scan so data will be acquired for 30 seconds.
