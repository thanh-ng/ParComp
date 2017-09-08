#include <stdio.h>
#include <omp.h>


int main(){
    /*What to learn here:
     * get thread tid
     * set number of threads 
     * get number of threads
     * private 
     */

    int nthreads, tid;
    #pragma omp parallel num_threads(8) private(nthreads, tid)
    {/* set the number of threads, fork a team of threads with each thread having a private tid variable (tid is copied across threads)*/

        /* obtain thread id */
        tid = omp_get_thread_num();
        printf("thread = %d\n", tid);

        /* Only master thread does */
        if (tid == 0){
            nthreads = omp_get_num_threads(); 
            printf("number of threads = %d\n", nthreads); 
         }
    } /* all threads join master thread and terminate*/
	return 0;
}
