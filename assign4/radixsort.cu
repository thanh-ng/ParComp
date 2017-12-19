#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h> 



__global__ void copyByIndex(int *out, int *in, int *index, int n){

    int tid = blockIdx.x*blockDim.x + threadIdx.x;
    if(tid < n){
        out[index[tid]] = in[tid]; 
    }
}


__global__ void copyByIdentity(int *out, int *in, int n){

    int tid = blockIdx.x*blockDim.x + threadIdx.x;
    if(tid < n){
        out[tid] = in[tid]; 
    }
}

// Compute exclusive prefix sum 
__global__ void scan(int *d_out, int *d_in_array, int *SUMS, int *INCR, int *bias, int d_pow, int nBlocks, int n){
        // When using dynamically allocated shared memory, only one pointer to the allocated space will be given to the kernel code. If you want to divide up that space, you must do so yourself

	extern __shared__ int temp[]; // 4* blockSize , each block will allocate its own shared memory        

	int tid = blockIdx.x*blockDim.x + threadIdx.x; // thread id across blocks 
	//int bid = blockIdx.x; 
	// bool DEBUG = false; 
    int offset = 1; // offset for each iteration of Scan, located in the global memory 
	int blockSize = n / nBlocks; // number of elements to be processed in a block, blockSize / 2 = blockDim, REQ: power of 2
	int local_tid = (2*tid) % blockSize; 
	int bid = tid / (blockSize / 2); 

	//int bid = tid / (blockSize / 2); 	
	int t_index_zero, t_index_one;         
	int offset2; 

    // int my_monitor = 1; 
    // *bias = 0;


    temp[blockSize + local_tid ] =  (d_in_array[2*tid] / d_pow ) % 2 ;
    temp[blockSize + local_tid  + 1] = (d_in_array[2*tid + 1] / d_pow ) % 2;

    temp[local_tid] = 1 - (d_in_array[2*tid] / d_pow ) % 2 ;;
    temp[local_tid + 1] = 1 - (d_in_array[2*tid + 1] / d_pow ) % 2;

    temp[2*blockSize + local_tid] = (d_in_array[2*tid] / d_pow ) % 2; 
    temp[2*blockSize + local_tid +1] = (d_in_array[2*tid + 1] / d_pow ) % 2;
		
		//INCR[bid] = 0; 
		
		// MAYBE the copy here need to complete before proceeding to the next steps!
		//============= DEBUG
		//if(DEBUG){
		//    printf("INPUT:    [Block #%d] temp[%d] = %d, temp[%d] = %d\n", bid,  2*tid, temp[local_tid], 2*tid + 1, temp[local_tid+1]);
		//}
		//===================

        	// Sweep up
    for (int d = blockSize>>1; d > 0; d >>=1){
        __syncthreads();
            if (tid % ( blockSize / 2 ) < d) {
                int ai = offset * ( local_tid + 1) - 1;
                int bi = offset * ( local_tid + 2) -1;

                temp[bi] += temp[ai];

				temp[blockSize + bi] += temp[blockSize + ai];
       		}
            	offset *= 2;
    }

        	// for exclusive prefix sum 
    if (tid % (blockSize/2) == 0 ) { 
        // *bias += temp[blockSize-1]; 
        SUMS[bid] = temp[2*blockSize-1]; 
		//NZ[bid] = temp[blockSize-1]; 
			//temp[3*blockSize] = blockSize - temp[2*blockSize-1]; 
		temp[blockSize-1] = 0; 
		temp[2*blockSize-1] = 0;
	}

     __syncthreads(); 
         
         
         



    //======= Scan SUMS ==========
    // ASSUMPTION: nBlocks <= blockSize so that SUMS can be processed by a single block. 
    // In practice, blockSize = 1024. With this assumption, the max length of array input is 2**20; 

	offset2 = 1;
		//if ( 2*tid + 1 <= nBlocks -1){ 
	temp[3*blockSize + local_tid] = SUMS[local_tid]; 
	temp[3*blockSize + local_tid + 1] = SUMS[local_tid + 1]; 

	if(tid % (blockSize/2) == 0){ // zero padding
		for(int i = nBlocks; i < blockSize; i++){
			temp[3*blockSize + i] = 0;
		}
	}

    // printf("BEFORE\n");
    // printf("Block #%d, temp[3*blockSize + %d] = %d, temp[3*blockSize + %d] = %d \n", bid, local_tid, temp[3*blockSize + local_tid],local_tid + 1, temp[3*blockSize + local_tid + 1] );

    //__syncthreads(); 
        
    // Sweep up SUMS
	for(int d = blockSize>>1; d > 0; d >>= 1){
		__syncthreads(); 
			
    	if (tid % ( blockSize / 2 ) < d) {
        	int ai = offset2 * ( local_tid + 1) - 1;
        	int bi = offset2 * ( local_tid + 2) -1;

        	temp[3*blockSize + bi] += temp[3*blockSize + ai];
                        
   		}
    	offset2 *= 2;

	}

	if (tid % ( blockSize / 2 ) == 0 ) {  
		*bias = n - temp[3*blockSize + nBlocks-1];
		//printf("tid = %d, bias = %d\n", tid, *bias);
		temp[4*blockSize - 1] = 0;
	}

    // Sweep down SUMS
	for(int d = 1; d < blockSize; d *= 2){
    	offset2 >>= 1;
    	__syncthreads();

    	if (tid % (blockSize / 2)  < d) {
        	int ai = offset2 * (local_tid +1) - 1;
        	int bi = offset2 * (local_tid +2) - 1;

        	int t = temp[3*blockSize + ai];
        	temp[3*blockSize + ai] = temp[3*blockSize + bi];
        	temp[3*blockSize + bi] += t;
    	}
	}


	__syncthreads();

    // printf("AFTER\n");
    // printf("Block #%d, temp[3*blockSize + %d] = %d, temp[3*blockSize + %d] = %d \n", bid, local_tid, temp[3*blockSize + local_tid],local_tid + 1, temp[3*blockSize + local_tid + 1] );

    // if(tid % (blockSize/2) == 0){
    // //    printf("INCR[%d] = %d\n", bid,temp[3*blockSize + bid] ) ;
    //  //    INCR[bid] = temp[3*blockSize + bid];
    //     temp[4*blockSize] = temp[3*blockSize + bid]; 
    // }

    //if ( 2*tid + 1 <= nBlocks -1){

    //    INCR[2*tid] = temp[3*blockSize + 2*tid]; 
    //    INCR[2*tid + 1] = temp[3*blockSize + 2*tid + 1]; 
    //}

    //INCR[local_tid] = temp[3*blockSize + local_tid];
    //INCR[local_tid+1] = temp[3*blockSize + local_tid + 1];

    //MY_INCR[2*tid] = temp[3*blockSize + local_tid];
    //MY_INCR[2*tid+1] = temp[3*blockSize + local_tid + 1];
        	
	//__syncthreads();

	//}
	
	//if(2*tid + 1 <= nBlocks -1){
	 //   printf("INCR[%d] = %d, INCR[%d] = %d\n", 2*tid, INCR[2*tid], 2*tid +1, INCR[2*tid +1]);
	//}
	/*
    if(tid == 0){
	    printf("INCR: \n");
	    for(int i = 0; i < blockSize -1; i++){
	        printf("%d,", INCR[i]);
	        }
	        printf("%d\n", INCR[blockSize-1]);
	}
	*/
	        //==========================

        	// Sweep down 
    	for(int d = 1; d < blockSize; d *= 2){
            	offset >>= 1;
            	__syncthreads();

            	if (tid % (blockSize / 2)  < d) {
                    int ai = offset * (local_tid + 1) - 1;
                    int bi = offset * (local_tid + 2) - 1;

                    int t0 = temp[ai];
                    temp[ai] = temp[bi];
                    temp[bi] += t0;
                    
                int t1 = temp[blockSize + ai];
                    temp[blockSize + ai] = temp[blockSize + bi];
                    temp[blockSize + bi] += t1;
           		 }
    	 	}


        // if(tid % (blockSize / 2) == 0){
        //     INCR[0] = 0; 
        //     *bias = SUMS[0]; 
        //     for(int i = 1; i < nBlocks; i++){
        //         INCR[i] = INCR[i-1] + SUMS[i-1]; 
        //         *bias += SUMS[i]; 
        //     }
            
        // }

    	__syncthreads();


        // if(tid == 0){
        //     printf("INCR: \n");
        //     for(int i = 0; i < nBlocks -1; i++){
        //         printf("%d,", INCR[i]);
        //         }
        //         printf("%d\n", INCR[nBlocks-1]);
                
        //     // printf("bias = %d\n", *bias);
        // }
        
    	
    	//==============DEBUG============================
    	// Check temp 
        // if(DEBUG){
    	// printf("OUTPUT:    [Block #%d] temp[%d] = %d, temp[%d] = %d\n", bid,  2*tid, temp[local_tid], 2*tid + 1, temp[local_tid+1]);
    	
    	// // Check INCR 
    	// if(tid == 0){
    	//     printf("INCR: \n");
    	//     for(int i = 0; i < nBlocks -1; i++){
    	//         printf("%d,", INCR[i]);
    	//         }
    	//         printf("%d\n", INCR[nBlocks-1]);
    	        
    	//     //printf("bias = %d\n", *bias);
    	// }
    	
    	// Check bias
    	
    	//===============================================
        // }
    	t_index_zero = (temp[local_tid] + bid*blockSize - temp[3*blockSize + bid])* (1 - temp[2*blockSize + local_tid]) + (temp[blockSize + local_tid] + temp[3*blockSize + bid] + *bias) * temp[2*blockSize + local_tid] ;
    	t_index_one = (temp[local_tid + 1] + bid*blockSize - temp[3*blockSize + bid])* (1 - temp[2*blockSize + local_tid + 1]) + (temp[blockSize + local_tid + 1] + temp[3*blockSize + bid] + *bias)* temp[2*blockSize + local_tid + 1];

        // t_index_one: =f(tid), thus each thread has a different t_index_one



        //my_monitor = tid; 
        //printf("bid = %d, tid = %d, my_monitor = %d\n", bid, tid, my_monitor);
        // d_out[t_index_zero] = d_in_array[2*tid];
        // d_out[t_index_one] = d_in_array[2*tid + 1]; 
    	
        // t_index_zero = *bias; //(temp[local_tid] + bid*blockSize - INCR[bid])* (1 - temp[2*blockSize + local_tid]) ;
        // t_index_one = *bias; //(temp[local_tid + 1] + bid*blockSize - INCR[bid])* (1 - temp[2*blockSize + local_tid + 1]);

        // printf("bid = %d, tid = %d, t_index_zero = %d, t_index_one = %d\n", bid, tid, t_index_one, t_index_zero);


        d_out[2*tid] = t_index_zero;
        d_out[2*tid + 1] = t_index_one; 
        //@(TODO): something wrong with indices for zero! More specifically, INCR[bid]



}

int main(int argc, char* argv[]){
    if(argc < 4){
        printf("Usage: %s seed N print_option\n", argv[0]);
        exit(1);
    }
    int SEED = atoi(argv[1]);
    int M = atoi(argv[2]);
    int PRINT = atoi(argv[3]); // print the output if 1, do not print if 0

    int N = pow(2, ceil( log2((float) M))); 
    int *in_array = (int*) malloc(N * sizeof(int));
    //printf("next = %d\n", *next);
    
    /* Initialize the random number generator for the given SEED */
    srand(SEED);
    /* Generate N pseudo-random integers in the interval [0, RAND_MAX] */
    for (int i = 0; i < M; i++)
        in_array[i] = rand();

    for(int i = M; i < N; i++)
        in_array[i] = 0; 

    // if(PRINT){
    //     printf("INPUT: \n");
    //     for (int i = 0; i < 20; i++)
    //         printf("%d\n", in_array[i]);

    // }

    // const int N = 16; // number of elements
	const int B = 1024; // number of elements to be processed in each block. REQ: power of 2 
	// N / B: number of blocks, with B/2 threads per block
    int nBlocks = N / B; 
        // Input

	//int in_array[N] = { 5, 6, 2, 7, 1, 0, 4, 3 };
	//int in_array[N] = {1804289383, 846930886, 1681692777, 1714636915, 1957747793, 424238335, 719885386, 1649760492};
	// int in_array[N] = {834, 86, 77, 15, 93, 35, 84, 92};
    // int in_array[N] = {5, 13, 6, 14, 2, 8, 12,  7, 15, 1, 0, 4, 9, 10, 3, 11};

    // int in_array[N] = {84, 92, 77, 93, 834, 86, 15, 35};

	int bit_width = 32; 
 
	int *d_out,*d_in_array;
 
    int index[N]; 
    int temp_in[N];

	int *SUMS; // To save the same of the array carried out by each block
	int *INCR; // Scanned result of SUMS

	int *bias; 
	int d_pow = 1; 



    int *d_index, *d_temp_in; 
    cudaMalloc( (void **) &d_index, N*sizeof(int));
    cudaMalloc( (void **) &d_temp_in, N*sizeof(int));


    cudaMalloc( (void **) &d_out, N*sizeof(int));
    cudaMalloc( (void **) &d_in_array, N*sizeof(int)); 
    cudaMalloc( (void **) &SUMS, nBlocks*sizeof(int));
    cudaMalloc( (void **) &INCR, nBlocks*sizeof(int));
    cudaMalloc( (void **) &bias, sizeof(int));



    struct timeval start, end;
    gettimeofday(&start, 0);
    for (int i = 0; i < bit_width; i++){
    	cudaMemcpy( d_in_array, in_array, N*sizeof(int), cudaMemcpyHostToDevice);
    	scan<<< nBlocks , B / 2, (4*B + 1)*sizeof(int)>>>(d_out, d_in_array, SUMS, INCR, bias, d_pow, nBlocks, N); 
    	cudaMemcpy( index, d_out, N*sizeof(int), cudaMemcpyDeviceToHost);

        // cudaThreadSynchronize();

        cudaMemcpy( d_in_array, in_array, N*sizeof(int), cudaMemcpyHostToDevice);
        cudaMemcpy( d_index, index, N*sizeof(int), cudaMemcpyHostToDevice);
        copyByIndex<<<nBlocks , B >>>(d_temp_in, d_in_array, d_index, N); 
        cudaMemcpy(temp_in, d_temp_in,  N*sizeof(int), cudaMemcpyDeviceToHost);
        // for (int i = 0; i < N; i++){
        //     temp_in[index[i]] = in_array[i];
        // }

        cudaMemcpy( d_temp_in, temp_in , N*sizeof(int), cudaMemcpyHostToDevice);
        copyByIdentity<<<nBlocks , B >>>(d_in_array, d_temp_in, N); 
        cudaMemcpy(in_array, d_in_array,  N*sizeof(int), cudaMemcpyDeviceToHost);


        // for (int i = 0; i < N; i++){
        //     in_array[i] = temp_in[i];
        // }

        // for (int i = 0; i < N; i++){
        //     printf("%d ", in_array[i]);
        // }
        // printf("\n");

    	d_pow *= 2; 
    }

    gettimeofday(&end, 0);

       // Print s after sorted 
    // for (int i = 0; i < N; i++){
    //         printf("%d\n", in_array[i]);
    // }
	


    if(PRINT){
        printf("Output: \n");
        for (int i = 0; i < 20; i++)
            printf("%d\n", in_array[N - M + i]);

    }
    else{
        printf("Sorting Time: %lld sec.\n", end.tv_sec - start.tv_sec);
    }


	cudaFree( d_in_array); 
	cudaFree( d_out);
	cudaFree( SUMS); 
	cudaFree( INCR); 
	cudaFree( bias); 
        return 0;
}
      
