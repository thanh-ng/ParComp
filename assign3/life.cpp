/* 
* IMPLEMENTATION
* 	- two-dimensional data layouts 
* ASSUMPTION:
* 	- grid size: m x m
* 	- # number of processes = np, np is square and m is divisible by sqrt(np) 
* 
*/
#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
using namespace std; 

//////////////


/****** To compile a MPI program:
*       mpic++ my_file.cpp 
*
* To execute a MPI compiled program:
*       mpirun -np <num of processors> ./a.out
*/

// Consider a simple scenaria:



void init_grid(int** padded_data, int rows, int cols, int row_start_ind, int col_start_ind, int SIZE, int** BOARD){
// Initialize 'grid' using rank and the (global) 'BOARD'

	// inner area
	for(int i = 1; i < rows+1; i++)
		for(int j = 1; j < cols+1; j++)
			padded_data[i][j] = BOARD[row_start_ind+i-1][col_start_ind+j-1]; 

	
	// boundary cols of the padded_data
	for(int i = 0; i < rows + 2; i++){

		//**   left col
		if(col_start_ind == 0){
			padded_data[i][0] = 0;
		} 
		// upper-left corner
		else if (row_start_ind == 0 && i ==0){
			padded_data[i][0] = 0;
		}
		// lower-left corner
		else if(row_start_ind + rows > SIZE-1 && i == rows+1){
			padded_data[i][0] = 0;
		}
		else{

		// row_start_ind = 0 & i = 0
			// cout << row_start_ind+i-1 << ", " << col_start_ind-1 << endl;
			padded_data[i][0] = BOARD[row_start_ind+i-1][col_start_ind-1];

		}


		//*** right col
		if(col_start_ind + cols > SIZE-1){
			padded_data[i][cols+1] = 0;
		}

		// upper-right corner
		else if (row_start_ind == 0 && i == 0){
			padded_data[i][cols+1] = 0;
		} 
		// lower-right corner
		else if (row_start_ind + rows > SIZE-1 && i == rows+1){
			padded_data[i][cols+1] = 0;
		}
		else{ // col_start_ind + cols < SIZE-1 AND ( row_start_ind > 0 OR i > 0)

			padded_data[i][cols+1] = BOARD[row_start_ind+i-1][col_start_ind + cols];

		}
	}



	for(int j = 0; j < cols + 2; j++){
		// col_start_ind = 0 
		if(row_start_ind == 0){
			padded_data[0][j] = 0;
		} 
		// upper-left corner
		else if (col_start_ind == 0 && j ==0){
			padded_data[0][j] = 0;
		} 
		// upper-right corner
		else if(col_start_ind + cols > SIZE-1 && j == cols + 1){
			padded_data[0][j] = 0;
		}
		else{

		// row_start_ind = 0 & i = 0
			padded_data[0][j] = BOARD[row_start_ind-1][col_start_ind+j-1];

		}


		//
		if(row_start_ind + rows > SIZE-1){
			padded_data[rows+1][j] = 0;
		}else if (col_start_ind == 0 && j == 0){
			padded_data[rows+1][j] = 0;
		} 

		else if(col_start_ind + cols > SIZE-1 && j == cols + 1){
			padded_data[rows+1][j] = 0;
		}
		else{

			padded_data[rows+1][j] = BOARD[row_start_ind+rows][col_start_ind + j-1];

		}
	}

}


void print_padded_grid(int** padded_data, int rows, int cols){
	for(int i = 0; i < rows+2; i++){
		for(int j = 0; j < cols+2; j++){
			cout << padded_data[i][j] << " ";
		}
		cout << endl;
	}

}
void update_grid_inner(int** padded_data, int rows, int cols){
// Locally, not yet comm

	int old_grid[rows+2][ cols+2];
	// copy(&padded_data, &padded_data +(rows+2)*(cols+2), &old_grid[0][0]);
	// memcpy(old_grid, padded_data, (rows+2)*(cols+2)*sizeof(int));

	// copy grid data to old_grid 
	// (TODO): is there any faster way that looping?
	for(int i = 0; i < rows+2; i++){
		for(int j = 0; j < cols+2; j++){
			// cout << old_grid[i][j] << " ";
			old_grid[i][j] = padded_data[i][j];
		}	
				// cout << endl;

	}

	int s; // sum of all neighboring cells
	int val_t; 
	// update only the inner (unpadded) area of grid
	for(int r = 1; r < rows+1; r++){
		for(int c = 1; c < cols+1; c++){
			val_t = old_grid[r][c];
			s = - val_t;
			for(int i = -1; i <= 1; i++){
				for(int j = -1; j <= 1; j++){
					s += old_grid[r+i][c+j];
				}
			}

                        if (      (val_t == 1 && (s < 2 || s >3)   ) || (val_t == 0 && s == 3)       ){
                                padded_data[r][c] = 1 - val_t;

                        }

	
		}	
	}
}

void update_ghost_cells(
			int** padded_data, 
			int rows, 
			int cols, 
			int* upper_row, 
			int* lower_row, 
			int* left_col, 
			int* right_col,
			int ul_corner, 
			int ur_corner, 
			int ll_corner, 
			int lr_corner
){
/* Update the padded grid using ghost cells (whose values received from the grid's neighbors)
*/

	padded_data[0][0] = ul_corner; 
	padded_data[0][cols+1] = ur_corner; 
	padded_data[rows+1][0] = ll_corner; 
	padded_data[rows+1][cols+1] = lr_corner; 

	for(int j = 1; j <= cols; j++){
		padded_data[0][j] = upper_row[j-1];
		padded_data[rows+1][j] = lower_row[j-1]; 

	}


	for(int i = 1; i <= rows; i++){
		padded_data[i][0] = left_col[i-1];
		padded_data[i][cols+1] = right_col[i-1];
	}

}

void extract_boundary_cells(
	int** padded_data, 
	int rows, 
	int cols,
	int* upper_row, 
	int* lower_row, 
	int* left_col, 
	int* right_col,
	int &ul_corner, 
	int &ur_corner, 
	int &ll_corner, 
	int &lr_corner){
/* Extract ghost cells for the grid's neighbors

*/
	for(int j = 1; j <= cols; j++){
		upper_row[j-1] = padded_data[1][j];
		lower_row[j-1] = padded_data[rows][j]; 

	}

	for(int i = 1; i <= rows; i++){
		left_col[i-1] = padded_data[i][1];
		right_col[i-1] = padded_data[i][cols];
	}

	ul_corner = padded_data[1][1]; 
	ur_corner = padded_data[1][cols]; 
	ll_corner = padded_data[rows][1];
	lr_corner = padded_data[rows][cols]; 
}

void rank_to_pos(int m, int np, int rank, int* rows, int* cols, int* row_start_ind, int* col_start_ind){
// Convert rank to rows,cols, row_start_ind and col_start_ind for a local grid
// ASSUMPTION: np is square, m^2 is divisible by np
	int nblocks = (int) sqrt(np);
	int tmp =  (int) m / nblocks;
	*rows = tmp;
	*cols = tmp; 
	
	*row_start_ind = (rank / nblocks) * tmp;
	*col_start_ind = (rank % nblocks) * tmp;
}


int main(int argc, char *argv[]){

	//****** MPI *************

	MPI_Init(&argc, &argv);	
	int** padded_data; 
	int** BOARD; 
	int SIZE, N; 
	char c; 

	int rank; 
	int np;
	int rows, cols, row_start_ind, col_start_ind;  

	MPI_Comm_size(MPI_COMM_WORLD, &np);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

 	int nblocks = (int) sqrt(np); // ASSUME: nblocks > 1


	//******* Handle input and boadcast it to all processes in the comm ***********
	if(rank == 0){
		cin >> SIZE >> N; 
	}

	MPI_Bcast(&SIZE, 1, MPI_INT, 0, MPI_COMM_WORLD); 
	MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD); 


	// checking ASSUMPTION 
	if(nblocks*nblocks != np || ( SIZE % nblocks != 0)){
		if(rank == 0)
			cout << "[WARNING] For this implementation version, please make sure that #np is a square number && board_size is divisible by sqrt(#np) .\n";
		MPI_Finalize();

		return 0; 
	}

	BOARD = new int*[SIZE]; 
	for(int i = 0; i < SIZE; i++){
		BOARD[i] = new int[SIZE];
	}

	int tmp_data[SIZE][SIZE]; 

	if (rank == 0){
		for(int i = 0; i < SIZE; i++){
			for(int j = 0; j < SIZE; j++){
				cin >> c; 
				if (c == '.') 
					tmp_data[i][j] = 0; 
				else 
					tmp_data[i][j] = 1;
			}
		}
	}

	MPI_Bcast(tmp_data, SIZE*SIZE, MPI_INT, 0, MPI_COMM_WORLD); 

	// Initialize the input, BOARD,  in each process
	for(int i = 0; i < SIZE; i++){
			for(int j = 0; j < SIZE; j++){
				BOARD[i][j] = tmp_data[i][j];
		}
	}

	//*****************************************************************************


	rank_to_pos(SIZE, np, rank, &rows, &cols, &row_start_ind, &col_start_ind);

	padded_data = new int*[rows+2]; 
	for(int i = 0; i < rows +2; i++)
		padded_data[i] = new int[cols +2];

	init_grid(padded_data, rows, cols, row_start_ind, col_start_ind, SIZE, BOARD);

	//*** FOR DEBUG ***************************
	// update_grid_inner(padded_data, rows, cols);

	// printf("========#%d===========\n", rank); 
	// print_padded_grid(padded_data, rows, cols);

	//****** END OF DEBUG *********************
	// sending variables
	
	int b_upper_row[cols]; // tag = 0
	int b_lower_row[cols]; // tag = 1
	int b_left_col[rows]; // tag = 2
	int b_right_col[rows]; // tag = 3
	int b_ul_corner, b_ur_corner, b_ll_corner, b_lr_corner; // tag = 4,5,6,7 
	
	// receiving variables
	int upper_row[cols]; 
	int lower_row[cols]; 
	int left_col[rows]; 
	int right_col[rows]; 
	int ul_corner, ur_corner, ll_corner, lr_corner; 

	
	// initialize 
	for(int i = 0; i < cols; i++){
		upper_row[i] = 0;
		lower_row[i] = 0;
	}
	
	for(int i = 0; i < rows; i++){
		left_col[i] = 0;
		right_col[i] = 0;
	}

	ul_corner = 0; 
	ur_corner = 0; 
	ll_corner = 0; 
	lr_corner = 0; 
	

	for(int iter = 0; iter < N; iter ++){
		// Assumption: when enter an iter, ghost cells are NOT updated from the prevous iter
		extract_boundary_cells(padded_data, rows, cols, b_upper_row, b_lower_row, b_left_col, b_right_col, b_ul_corner, b_ur_corner, b_ll_corner, b_lr_corner);
		
		// ************  four corner blocks
		if(rank == 0){
			// printf("rank #%d, sending ..,\n", rank);

			MPI_Send(b_right_col, rows, MPI_INT, rank +1, 3, MPI_COMM_WORLD); // sender-based tag
			MPI_Send(&b_lr_corner, 1, MPI_INT, rank + nblocks + 1, 7, MPI_COMM_WORLD); 
			MPI_Send(b_lower_row, cols, MPI_INT, rank + nblocks, 1, MPI_COMM_WORLD); 

			// printf("rank #%d, receving ..,\n", rank);

			MPI_Recv(right_col, rows, MPI_INT, rank + 1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(lower_row, cols, MPI_INT, rank + nblocks, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&lr_corner, 1, MPI_INT, rank + nblocks + 1, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			// printf("rank #%d, recevED!\n", rank);

			update_ghost_cells(padded_data, rows, cols, upper_row, lower_row, left_col, right_col, ul_corner, ur_corner, ll_corner, lr_corner);
			update_grid_inner(padded_data, rows, cols);
		} else

		if(rank == nblocks-1){
			// printf("rank #%d, sending ..,\n", rank);

			MPI_Send(b_left_col, rows, MPI_INT, rank-1, 2, MPI_COMM_WORLD); // sender-based tag
			MPI_Send(&b_ll_corner, 1, MPI_INT, rank + nblocks - 1, 6, MPI_COMM_WORLD); 
			MPI_Send(b_lower_row, cols, MPI_INT, rank+ nblocks, 1, MPI_COMM_WORLD); 
			// printf("rank #%d, receving ..,\n", rank);

			
			MPI_Recv(left_col, rows, MPI_INT, rank-1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(lower_row, cols, MPI_INT, rank+nblocks, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&ll_corner, 1, MPI_INT, rank + nblocks-1, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			// printf("rank #%d, recevED!\n", rank);

			update_ghost_cells(padded_data, rows, cols, upper_row, lower_row, left_col, right_col, ul_corner, ur_corner, ll_corner, lr_corner);
			update_grid_inner(padded_data, rows, cols);
		} else


		if(rank == nblocks*nblocks - 1){
			// printf("rank #%d, sending ..,\n", rank);

			MPI_Send(b_left_col, rows, MPI_INT, rank-1, 2, MPI_COMM_WORLD); // sender-based tag
			MPI_Send(&b_ul_corner, 1, MPI_INT, rank - nblocks - 1, 4, MPI_COMM_WORLD); 
 			MPI_Send(b_upper_row, cols, MPI_INT, rank -  nblocks, 0, MPI_COMM_WORLD); 
			// printf("rank #%d, receving ..,\n", rank);

			MPI_Recv(left_col, rows, MPI_INT, rank-1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(upper_row, cols, MPI_INT, rank - nblocks, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&ul_corner, 1, MPI_INT, rank - nblocks-1, 7, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			// printf("rank #%d, recevED!\n", rank);

			update_ghost_cells(padded_data, rows, cols, upper_row, lower_row, left_col, right_col, ul_corner, ur_corner, ll_corner, lr_corner);
			update_grid_inner(padded_data, rows, cols);
       } else

		if(rank == nblocks*(nblocks - 1)){
			// printf("rank #%d, sending ..,\n", rank);

			MPI_Send(b_right_col, rows, MPI_INT, rank+1, 3, MPI_COMM_WORLD); 
			MPI_Send(&b_ur_corner, 1, MPI_INT, rank - nblocks + 1, 5, MPI_COMM_WORLD); 
			MPI_Send(b_upper_row, cols, MPI_INT, rank - nblocks, 0, MPI_COMM_WORLD); 
			// printf("rank #%d, receving ..,\n", rank);

			MPI_Recv(right_col, rows, MPI_INT, rank+1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(upper_row, cols, MPI_INT, rank - nblocks, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&ur_corner, 1, MPI_INT, rank - nblocks+1, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			// printf("rank #%d, recevED!\n", rank);

			update_ghost_cells(padded_data, rows, cols, upper_row, lower_row, left_col, right_col, ul_corner, ur_corner, ll_corner, lr_corner); 
			update_grid_inner(padded_data, rows, cols);
       	} else

       	//************** border blocks (if any) *******************
   		// horizontal, upper 
   		if ( rank > 0 && rank < nblocks-1){
			// printf("rank #%d, sending ..,\n", rank);

			MPI_Send(b_right_col, rows, MPI_INT, rank+1, 3, MPI_COMM_WORLD); 
			MPI_Send(b_left_col, rows, MPI_INT, rank-1, 2, MPI_COMM_WORLD); 
			MPI_Send(b_lower_row, cols, MPI_INT, rank + nblocks, 1, MPI_COMM_WORLD); 
			MPI_Send(&b_ll_corner, 1, MPI_INT, rank + nblocks - 1, 6, MPI_COMM_WORLD); 
			MPI_Send(&b_lr_corner, 1, MPI_INT, rank + nblocks + 1, 7, MPI_COMM_WORLD); 
			// printf("rank #%d, receving ..,\n", rank);

			MPI_Recv(right_col, rows, MPI_INT, rank+1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(left_col, rows, MPI_INT, rank -1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(lower_row, cols, MPI_INT, rank + nblocks, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			MPI_Recv(&ll_corner, 1, MPI_INT, rank + nblocks - 1, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&lr_corner, 1, MPI_INT, rank + nblocks + 1, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			// printf("rank #%d, recevED!\n", rank);

			update_ghost_cells(padded_data, rows, cols, upper_row, lower_row, left_col, right_col, ul_corner, ur_corner, ll_corner, lr_corner); 
			update_grid_inner(padded_data, rows, cols);

		} else
   		// horizontal, lower 
   		if ( rank > nblocks*(nblocks - 1) && rank < nblocks*nblocks - 1){
			// printf("rank #%d, sending ..,\n", rank);

			MPI_Send(b_right_col, rows, MPI_INT, rank+1, 3, MPI_COMM_WORLD); 
			MPI_Send(b_left_col, rows, MPI_INT, rank-1, 2, MPI_COMM_WORLD); 
			MPI_Send(b_upper_row, cols, MPI_INT, rank - nblocks, 0, MPI_COMM_WORLD); 
			
			MPI_Send(&b_ul_corner, 1, MPI_INT, rank - nblocks - 1, 4, MPI_COMM_WORLD); 
			MPI_Send(&b_ur_corner, 1, MPI_INT, rank - nblocks + 1, 5, MPI_COMM_WORLD); 
			// printf("rank #%d, receving ..,\n", rank);

			MPI_Recv(right_col, rows, MPI_INT, rank+1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(left_col, rows, MPI_INT, rank -1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(upper_row, cols, MPI_INT, rank - nblocks, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			MPI_Recv(&ul_corner, 1, MPI_INT, rank - nblocks - 1, 7, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&ur_corner, 1, MPI_INT, rank - nblocks + 1, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			// printf("rank #%d, recevED!\n", rank);

			update_ghost_cells(padded_data, rows, cols, upper_row, lower_row, left_col, right_col, ul_corner, ur_corner, ll_corner, lr_corner); 
			update_grid_inner(padded_data, rows, cols);

		} else

   		// vertical, left
   		if ( rank > 0 && rank < nblocks*(nblocks - 1) && rank % nblocks == 0 ){
			// printf("rank #%d, sending ..,\n", rank);

			MPI_Send(b_right_col, rows, MPI_INT, rank+1, 3, MPI_COMM_WORLD); 
			MPI_Send(b_lower_row, cols, MPI_INT, rank + nblocks, 1, MPI_COMM_WORLD); 
			MPI_Send(b_upper_row, cols, MPI_INT, rank - nblocks, 0, MPI_COMM_WORLD); 
			
			MPI_Send(&b_lr_corner, 1, MPI_INT, rank + nblocks + 1, 7, MPI_COMM_WORLD); 
			MPI_Send(&b_ur_corner, 1, MPI_INT, rank - nblocks + 1, 5, MPI_COMM_WORLD); 
			// printf("rank #%d, receving ..,\n", rank);

			MPI_Recv(right_col, rows, MPI_INT, rank + 1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(lower_row, cols, MPI_INT, rank + nblocks, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(upper_row, cols, MPI_INT, rank - nblocks, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			MPI_Recv(&lr_corner, 1, MPI_INT, rank + nblocks + 1, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&ur_corner, 1, MPI_INT, rank - nblocks + 1, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			// printf("rank #%d, recevED!\n", rank);

			update_ghost_cells(padded_data, rows, cols, upper_row, lower_row, left_col, right_col, ul_corner, ur_corner, ll_corner, lr_corner); 
			update_grid_inner(padded_data, rows, cols);

		} else

   		// vertical, right
   		if ( rank > nblocks-1 && rank < nblocks*nblocks - 1 && (rank + 1) % nblocks == 0 ){
			// printf("rank #%d, sending ..,\n", rank);

			MPI_Send(b_left_col, rows, MPI_INT, rank-1, 2, MPI_COMM_WORLD); 
			MPI_Send(b_lower_row, cols, MPI_INT, rank + nblocks, 1, MPI_COMM_WORLD); 
			MPI_Send(b_upper_row, cols, MPI_INT, rank - nblocks, 0, MPI_COMM_WORLD); 
			
			MPI_Send(&b_ll_corner, 1, MPI_INT, rank + nblocks - 1, 6, MPI_COMM_WORLD); 
			MPI_Send(&b_ul_corner, 1, MPI_INT, rank - nblocks - 1, 4, MPI_COMM_WORLD); 
			// printf("rank #%d, receving ..,\n", rank);
			MPI_Recv(left_col, rows, MPI_INT, rank - 1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(lower_row, cols, MPI_INT, rank + nblocks, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(upper_row, cols, MPI_INT, rank - nblocks, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			MPI_Recv(&ll_corner, 1, MPI_INT, rank + nblocks - 1, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&ul_corner, 1, MPI_INT, rank - nblocks - 1, 7, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			// printf("rank #%d, recevED!\n", rank);

			update_ghost_cells(padded_data, rows, cols, upper_row, lower_row, left_col, right_col, ul_corner, ur_corner, ll_corner, lr_corner); 
			update_grid_inner(padded_data, rows, cols);

		} else

		// inner blocks 
		{
			// printf("rank #%d, sending ..,\n", rank);

			MPI_Send(b_right_col, rows, MPI_INT, rank+1, 3, MPI_COMM_WORLD); 
			MPI_Send(b_left_col, rows, MPI_INT, rank-1, 2, MPI_COMM_WORLD); 
			MPI_Send(b_lower_row, cols, MPI_INT, rank + nblocks, 1, MPI_COMM_WORLD); 
			MPI_Send(b_upper_row, cols, MPI_INT, rank - nblocks, 0, MPI_COMM_WORLD); 
			
			MPI_Send(&b_ll_corner, 1, MPI_INT, rank + nblocks - 1, 6, MPI_COMM_WORLD); 
			MPI_Send(&b_ul_corner, 1, MPI_INT, rank - nblocks - 1, 4, MPI_COMM_WORLD); 
			MPI_Send(&b_lr_corner, 1, MPI_INT, rank + nblocks + 1, 7, MPI_COMM_WORLD); 
			MPI_Send(&b_ur_corner, 1, MPI_INT, rank - nblocks + 1, 5, MPI_COMM_WORLD); 
			// printf("rank #%d, receving ..,\n", rank);


			MPI_Recv(right_col, rows, MPI_INT, rank + 1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(left_col, rows, MPI_INT, rank - 1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(lower_row, cols, MPI_INT, rank + nblocks, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(upper_row, cols, MPI_INT, rank - nblocks, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			MPI_Recv(&ll_corner, 1, MPI_INT, rank + nblocks - 1, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&ul_corner, 1, MPI_INT, rank - nblocks - 1, 7, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&lr_corner, 1, MPI_INT, rank + nblocks + 1, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&ur_corner, 1, MPI_INT, rank - nblocks + 1, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			// printf("rank #%d, recevED!\n", rank);

			update_ghost_cells(padded_data, rows, cols, upper_row, lower_row, left_col, right_col, ul_corner, ur_corner, ll_corner, lr_corner); 
			update_grid_inner(padded_data, rows, cols);

		}


       	
		MPI_Barrier(MPI_COMM_WORLD);
	}


	// printf("=======%d=======\n",rank);
	// print_padded_grid(padded_data, rows, cols);

	int sentdata[rows * cols]; 

	for(int i = 1; i <= rows; i++){
		for(int j = 1; j <= cols; j++){
			sentdata[(i-1)*rows +(j-1)] = padded_data[i][j];  
		}
	}

	int *rbuf; 
	if(rank == 0){

		rbuf = new int[np* rows * cols];
		// printf("=======%d=======\n",rank);
		// print_padded_grid(&grid);
	}

	MPI_Gather(sentdata, rows * cols , MPI_INT, rbuf, rows  * cols , MPI_INT, 0, MPI_COMM_WORLD); 

	if(rank == 0){
		int which_block, which_elem, x, y; 
		int final_data[SIZE][SIZE];
		for (int i = 0; i < np * rows * cols; i++ ){

			which_block = i / (rows*cols); 

			// cout << which_block << endl;
			which_elem = i %  (rows*cols); 
			x = (which_block / nblocks) * rows + which_elem / cols ; 
			y = (which_block % nblocks) * cols + which_elem % rows ; 

			// cout << x << "," << y << endl;

			final_data[x][y] = rbuf[i];

		}

		for(int i = 0; i < SIZE; i++){
			for(int j = 0; j < SIZE; j++){
				if(final_data[i][j]==1)
					cout << '#';
				else
					cout << '.';

			}
			cout << endl;
		}
	}

	MPI_Finalize();
	return 0; 
}
