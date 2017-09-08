/*Demonstrates do/for directive in omp*/
#include <stdio.h>
#include <omp.h>
#include <time.h>
#define CHUNKSIZE 100
#define N 100000

int main(){
    int i, chunk; 
    float a[N], b[N], c[N];

    for(i=0; i<N; i++){
        a[i] = b[i] = 1.0 * i;
    }

    chunk = CHUNKSIZE;
    clock_t start = clock(), diff;
    #pragma omp parallel num_threads(16) shared(a,b,c,chunk) private(i)
    {
        #pragma omp for schedule(dynamic, chunk) nowait
       /*shares iterations across the team. Nowait allows a threa not to wait other threads to finish*/
        for(i=0; i<N; i++){
            c[i] = a[i] + b[i];
        }

    }
    diff = clock() - start; 
    int msec = diff * 1000 / CLOCKS_PER_SEC;
    printf("time = %d ms\n", msec);
    return 1; 
}

