/*data-sharing attribute clauses
 * private: private to each threa, uninitialzed
 * firstprivate: private + initialization 
 * lastprivate: private + copy only from the last thread execution of the loop 
 * shared: shared among the team, shared variables are initialized arbitrarily if they are not intialized inside the parallel code 
 * default: user-specific data-sharing 
 * reduction: initialize a local copy depending on ops. Helpful in dealing with dependencies between iterations
 */
#include <stdio.h>
#include <omp.h>
#include <time.h> 

#define N 100
int main(){
    int s = 0;
    int i;
    #pragma omp parallel num_threads(8) //shared(s)  private(i) 
    {
      // s = 0;
        #pragma omp for reduction (+:s) //firstprivate(s) lastprivate(s)
        for(i=0; i<N; i++)
            s += i;
    }
    printf("sum = %d\n",s);
    return 1;
}
