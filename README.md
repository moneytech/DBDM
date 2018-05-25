# DBDM
A simple **D**isplacement **B**ased **D**ynamic **M**emory for embedded systems without a heap memory.

## About
Tiny embedded systems may lack a heap memory and therefore lack access to dynamic memory allocation functions usually available in C, such as *malloc* and *free*. This project implements a displacement based dynamic memory, which gives access to dynamic memory functions *dbdm_malloc* and *dbdm_free*, using a stack-allocated buffer.

Any standard implementation of dynamic memory relies on writing a *header* before the allocated memory block. This header usually contains information about the size of the allocated memory block, and the position of the next header in the memory segment. This header contains various pointers needed for evaluating the following memory block as well as for coalescing the memory. These pointers, though simplifying the implementation of the system,can consume a substantial amount of memory in small embedded systems and are therefore undesired.    
The given implementation relies on headers being displacement based, as in, a header contains information about the number of bytes it contains, as well as the number of bytes between itself and the next following memory segment - this allows for a minimal header size.  
To use the dynamic memory, a "heap" must be allocated - in our case, we allocate a buffer on the available stack of the microprocessor, which is then available for allocations.
Upon freeing a memory block, the implementation is able to coalesce the memory to correctly reclaim the memory to the buffer.

Refer to [dbdm.c](dbdm.c) for in-depth explanation of the workings of the implementation.  
The implementation is tested in [main.c](main.c).

## Keywords
Implementation of malloc and free  
Displacement based memory management  
Dynamic memory embedded systems
Buffer based dynamic memory stack allocated