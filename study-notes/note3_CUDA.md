## Synchronization

There are some points in the algorithm that breaks down parallelism because some threads need to synchronize with other. 
While threads belong to the same block are easy to syncronize using `__syncthreads()` ans shared memory, there is no obvious
way to do so for threads belong to different threads. In this case they must share data through global memory. For sharing data through global memory,
it is recommended to **use two separate kernel invocations: one for writing to and one for reading from global memory** [1]. 
It is important to note that one should not rely on any executation order of threads from different blocks. 


References:  

[1] [NVIDIA CUDA C Programming Guide](https://developer.download.nvidia.com/compute/DevZone/docs/html/C/doc/CUDA_C_Programming_Guide.pdf), p. 65-66.
