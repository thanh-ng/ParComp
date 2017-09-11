#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#define N 1000
double random(double a, double b){
    /*Draw a random number in [a,b]*/
    double x = (double)rand() / (double) RAND_MAX;
    return a + x*(b-a);
}

int main(){
    double x, y, pi;
    int i;
    int count = 0;
    omp_set_num_threads(16);
    #pragma omp parallel for private(x,y) reduction(+:count)    
        for(i = 0; i<N; i++){
            x = random(-1.0,1.0); y = random(-1.0, 1.0);
            if(x*x + y*y <= 1)
                count++; 
        }
    
    printf("pi = %f after %d trials\n", 4. * (double) count / (double) N, N );
}

