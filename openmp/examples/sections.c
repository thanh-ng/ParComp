#include <omp.h>
#include <stdio.h>

int main(){
    #pragma omp parallel sections
    {
       #pragma omp section
        {/*Typically each section is assigned to a thread or sometimes more threads
          * It can assign the master thread to a section
          */
            int id = omp_get_thread_num();
            if(id == 0)
                printf("number of threads = %d\n", omp_get_num_threads());
        }
        #pragma omp section
        {
            printf("section 1, id = %d\n", omp_get_thread_num());
        }
        #pragma omp section
        {
            printf("section 2, id = %d\n", omp_get_thread_num());
        }
    }
    }
    return 1;
}
