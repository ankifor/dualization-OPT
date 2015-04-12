#include <mpi.h>
#include <cstdio>
#include <vector>
#include <iostream>

#include "dualizer_OPT.h"
#include "stack_array.h"

static void read_matrix(binary::Matrix& L, const char* filename, ui32 rank) {
	ui32 sz[2];
	if (rank == 0) {
		L.read(filename);
		L.delete_le_rows();
		sz[0] = L.height();
		sz[1] = L.width();
	}
	MPI_Bcast(sz, 2, MPI_INT32_T, 0, MPI_COMM_WORLD);
	if (rank != 0) {
		L.reserve(sz[0], sz[1]);
	}
	MPI_Bcast(L.row(0), L.size32(), MPI_INT32_T, 0, MPI_COMM_WORLD);
}

static void calculate_task_size(Stack_Array<ui32>& task_size, ui32 n) {
	task_size.reserve(n);
	task_size.resize_to_capacity();
	My_Memory::MM_memset(task_size.get_data(), 0, n * UI32_SIZE);
	for (ui32 j = 0; j < n; ++j) {
		task_size[j] = 1;
	}
}

static void distribute_tasks(Stack_Array<ui32>& task_performer, ui32 n, ui32 rank, ui32 world_size) {
	task_performer.reserve(n);
	task_performer.resize_to_capacity();
	if (rank == 0) {
		Stack_Array<ui32> task_size;
		calculate_task_size(task_size, n);

		ui32 performer = 0;
		for (ui32 j = 0; j < n; ++j) {
			task_performer[j] = performer;
			performer = (performer >= world_size - 1 ? 0 : performer + 1);
		}
	}
	MPI_Bcast(task_performer.get_data(), task_performer.size(), MPI_INT32_T, 0, MPI_COMM_WORLD);
}


int main(int argc, char** argv) {	
	Stack_Array<ui32> task_performer;
	int world_size = 0;
	int rank = 0;
	// Initialize the MPI environment
	MPI_Init(&argc, &argv);	
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	binary::Matrix L;
	Dualizer_OPT solver;
	try {
		double wtime = 0.0;
		if (rank == 0) {
			wtime = MPI_Wtime();
		}
		read_matrix(L, argv[1], rank);
		//distribute tasks
		distribute_tasks(task_performer, L.width(), rank, world_size);
		//run
		char const* file_out = argv[2];		
		for (ui32 j = 0; j < L.width(); ++j) {
			if (task_performer[j] == rank) {
				solver.init(L, file_out);
				solver.run(j);
			}
		}
		//reduce
		ui32 sum = 0;
		{
			ui32 n_cov = solver.get_num();		
			//printf("before reduce: %d %d\n", rank, n_cov);
			MPI_Reduce(&n_cov, &sum, 1, MPI_INT32_T, MPI_SUM, 0, MPI_COMM_WORLD);
		}
		//time		
		if (rank == 0) {
			wtime = MPI_Wtime() - wtime;
			printf("%d, %f sec\n", sum, wtime);
		}
	} catch (std::runtime_error& rte) {
		std::cout << rte.what() << " sec" << std::endl;
		MPI_Abort(MPI_COMM_WORLD, -1);
	} catch (...) {
		std::cout << "Unknown error" << std::endl;
		MPI_Abort(MPI_COMM_WORLD, -1);
	}
	// Finalize the MPI environment.
	MPI_Finalize();
}