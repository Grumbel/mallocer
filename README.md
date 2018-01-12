mallocer
========

mallocer is a program to experiment with memory allocation and memory
usage in Linux. When run it will simply start allocating new memory of
a given size at a given interval. Memory can be filled with random
data, uninitialized or zeroed.

This program has no utility by itself, but is meant to be used
together with a process monitor to watch how Linux behaves when memory
is allocated.

The companion program to mallocer is
[procmem](https://github.com/Grumbel/procmem).


Compiling
---------

    mkdir build
    cd build
    cmake ..
    make

Optional flags for CMake:

    -DCMAKE_BUILD_TYPE=Release
    -DWARNINGS=ON
    -DWERROR=ON


Installing
----------

    make install


Usage
-----

    Usage: mallocer [OPTION...]
    A program to experiment with memory allocation
    
      -c, --calloc               Use calloc() instead of malloc()
      -f, --fill                 Fill allocated memory with data
      -i, --interval=MSEC        Time in milisec between allocations
      -I, --increment=BYTES      Increase allocation size by BYTES on each step
      -s, --size=BYTES           Bytes to allocate on each step
      -v, --verbose              Produce verbose output
      -?, --help                 Give this help list
          --usage                Give a short usage message
    
    Mandatory or optional arguments to long options are also mandatory or optional
    for any corresponding short options.


Running
-------

    $ ./mallocer  -s $[1024*125] -I 1024 -i 10
    make: 'mallocer' is up to date.
    First byte in main is at: 0x7ffc77b4f9c7
    mallocer is going to allocate some memory...
    1) trying to allocate 129024 with malloc()
    allocation succesful, new total memory: 129024 at 0x55a63c1318e0
    2) trying to allocate 130048 with malloc()
    allocation succesful, new total memory: 259072 at 0x55a63c1510f0
    distance to last allocation -129040
    3) trying to allocate 131072 with malloc()
    allocation succesful, new total memory: 390144 at 0x55a63c170d00
    distance to last allocation -130064
    4) trying to allocate 132096 with malloc()
    allocation succesful, new total memory: 522240 at 0x7f0f90abb010
    distance to last allocation -45532367332112
    5) trying to allocate 133120 with malloc()
    allocation succesful, new total memory: 655360 at 0x7f0f90a9a010
    distance to last allocation 135168
