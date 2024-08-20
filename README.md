Memory Allocation Simulator in C
This repository contains a memory allocation simulator written in C, developed as part of a school assignment. The simulator operates on a simulated heap and implements both implicit and explicit free list approaches for managing memory blocks. 
Users can choose between First-fit and Best-fit allocation strategies, with immediate coalescing of free blocks to optimize memory usage.

Features
Heap Simulation: Simulates a 32-bit system heap with double-word alignment.
Allocation Strategies: Supports First-fit and Best-fit approaches.
Primary Functions:
myalloc(size): Allocates memory.
myrealloc(pointer, size): Reallocates memory.
myfree(pointer): Frees allocated memory.
mysbrk(size): Grows or shrinks the heap.

Important Notice
This project was developed as an educational assignment. All code is original, with no external code used in the implementation of the primary functions (myalloc, myrealloc, myfree, mysbrk). 
Any references to external sources, such as examples on writing to files, are properly cited in the accompanying report.
