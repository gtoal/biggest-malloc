# biggest-malloc
Determine the largest possible area claimable with a single malloc call.

In my code I frequently want to claim the most memory possible in a single malloc (or calloc) call - this is often the most efficient way to code something that handles large data, such as a sort program or a utility to process gigabytes of DNA data.
However I discovered - at least when working on my 8Gb Raspberry Pi 4 cluster - that although a malloc would appear to succeed, when I came to access all the memory that it claimed to have found for me ... I got a runtime error.  So this code - although it might work on other Linux systems - is currently specific to the Raspberry Pi, in that it applies a hack (or as we call it in the trade, a heuristic) to the ostensible max memory discovered that reduces it by enough that it appears to always work in practise.

The attached program is framed as a test program; at some point in the future I will probably make it into a function that can be called from your code in order to determine how much RAM to ask for, before making your one big malloc call at the head of your program.

Just to be clear, it tries to work out how much RAM is actually available, not how much the address space allows for - it does actually take into account RAM that has already been taken by other processes.  And it is concerned with physical RAM, not virtual memory, on the grounds that if your code starts paging it will slow down by orders of magnitude - this is for code that is meant to run entirely within the working set of physical memory without ever needing to swap.

After writing this, I found out about /proc/sys/vm/overcommit_memory from a discussion at stackexchange ( https://stackoverflow.com/questions/2798330/maximum-memory-which-malloc-can-allocate/ ), which gives some hints that might be useful in a later iteration.
