# First written by Mark Ruzindana

Before generating m4 dependencies with 'aclocal', one must establish the libraries and tools used for configuration using 'libtoolize'. 

In most cases, just typing in 'libtoolize' should work, but this configure script expects version 2.4.6. The default libtool version on RHEL 7 is 2.4.2. This should not be tampered with because other parts of the system might be dependent on this particular version. So change the default at your own risk.

In order to run the updated version, download and install it if that has not already been done. Then rather than just typing libtoolize, specify the path to the executable, for example, the current path is /usr/local/bin/ so I would type:

sudo /usr/local/bin/libtoolize -v --force

The -v stands for verbose and is optional, but I like to see what is happening as it is being executed. And the --force forces an overwrite of any files that were already generated. After this, just type:

aclocal

to generate the m4 dependencies then go through the README_GB.md file for further configuration, compilation and installation instructions.
