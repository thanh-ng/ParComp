# Parallel Computing - Study Note

## Fundamental concepts in O.S. and multithreaded computing   
I present here important concepts that I think they are very fundamental but not easy to understand only from lecture notes in my class.They are listed in the order that I recall them, not in their "correct" order. 
* False sharing : [utube link](https://www.youtube.com/watch?v=dznxqe1Uk3E)  
* Understanding OS and Kernel: A kernel manages the communication between the software (user-level applications) and the hardward. OS = kernel space + user space (applications). For example, some can add applications to LINUX kernel to make OS such as Ubuntu, Centos. ([source](https://www.go4expert.com/articles/operating-kernel-types-kernels-t24793/))  
* Kernel threads vs. user threads: 
  * Differences: [link](http://www.cs.iit.edu/~cs561/cs450/ChilkuriDineshThreads/dinesh's%20files/User%20and%20Kernel%20Level%20Threads.html)
  * Why a user thread must be mapped to a kernel thread? [link](https://stackoverflow.com/questions/14791278/threads-why-must-all-user-threads-be-mapped-to-a-kernel-thread)   

* Priority Inversion: an good example of priority inversion and how priority inheritance ressolve the problem [link](http://www.drdobbs.com/jvm/what-is-priority-inversion-and-how-do-yo/230600008)   

* **Race condition**: e.g., incrementing a variable is typically implemented on modern
machines as follows
  * Load count into a register
  * Increment count
  * Store count back into memory
  
  
