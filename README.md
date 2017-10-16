# Parallel Computing - Study Note

## Fundamental concepts in O.S. and multithreaded computing   
I present here important concepts that I think they are very fundamental but not easy to understand only from lecture notes in my class.They are listed in the order that I recall them, not in their "correct" order. 
* **False sharing** : [utube link](https://www.youtube.com/watch?v=dznxqe1Uk3E)  
When a shared set of variables can fit the cache line, every time a thread in a processor modifies any of the variables in the set, it forces the memory update to maintain cache coherence (4 states MESI in cache line). Such memory update hurts the performance 

  ![](/figs/false_sharing.png)
* **Understanding OS and Kernel**: A kernel manages the communication between the software (user-level applications) and the hardward. OS = kernel space + user space (applications). For example, some can add applications to LINUX kernel to make OS such as Ubuntu, Centos. ([source](https://www.go4expert.com/articles/operating-kernel-types-kernels-t24793/))  
* **Kernel threads vs. user threads**: 
  * Differences: [link](http://www.cs.iit.edu/~cs561/cs450/ChilkuriDineshThreads/dinesh's%20files/User%20and%20Kernel%20Level%20Threads.html)
  * Why a user thread must be mapped to a kernel thread? [link](https://stackoverflow.com/questions/14791278/threads-why-must-all-user-threads-be-mapped-to-a-kernel-thread)   

* **Priority Inversion**: an good example of priority inversion and how priority inheritance ressolve the problem [link](http://www.drdobbs.com/jvm/what-is-priority-inversion-and-how-do-yo/230600008)   

* **Race condition**: e.g., incrementing a variable is typically implemented on modern
machines as follows
  * Load count into a register
  * Increment count
  * Store count back into memory  
  
  ![](/figs/increment.png)
  
  
## Dependency Patterns  
* Data dependencies:
  * True (flow) dependency: Read-After-Write (RAW) 
  * Anti-dependency: Write-after-Read (WAR) 
  * Output dependency: Write-after-Write (WAW)
* Loop-carried dependencies   

## Parallel Control Flow Patterns
* **Fork-join**: fork a flow into multiple parallel flows and then rejoin into a single flow   
* **Map**: perform a single function over every element of a collection 
* **Stencil**: perform a single function over a subset of neighboring elements of a clollection  
* **Reduce**: Combine each element in a collection using a combiner (e.g., addition, multiplication). Requirement: the combiner is associative. 
* **Scan**: Compute partial reduction of a collection (e.g., prefix sum)   
  ![](/figs/scan.png)  
* **Recurrence**: an advanced version of map where an iteration can depends on the output of the previous iteration. 
## Parallel Data Management Patterns  
* **Gather**: Read a collection of data given a collection of indices  
  * **Zip**: interleaves data 
  * **Unzip**
  * **Shift**
* **Scatter**: The inverse of gather
* **Pack** 
* **Pipeline** 
* **Geometric Decompostion** 
