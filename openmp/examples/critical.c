/*Demonstrates critical sections*/
#include <stdio.h>
#include <omp.h>
#include <time.h>

#define N 100

int main(){
    int i, count_p, count = 0;
    int a[N];
    for(i=0; i<N; i++)
        a[i] = i % 5;

    #pragma omp parallel num_threads(16) shared(a, count) private(count_p)
    {
        count_p = 0;
        #pragma omp for private(i)
        /*each thread is in charge of some iterations with its own i and count_p*/
        for(i=0; i<N; i++){
            if(a[i] == 3)
                count_p++;
        }

        #pragma omp critical (accumulate)
        {/* one thread is executed at a time. For this specific example, we actually don't need critical because count_p is private to each thread which is invoked exactly once inside the parallel section*/
            count += count_p;
        }
    }

    printf("count = %d\n", count);
    return 1;
}
