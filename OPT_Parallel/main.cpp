#include <mpi.h>
#include <cstdio>
#include <vector>
#include <iostream>

#include "dualizer_OPT.h"

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

static void calculate_task_size(std::vector<ui32>& task_size, ui32 n) {
	task_size.resize(n);
	for (ui32 j = 0; j < n; ++j) {
		task_size[j] = 1;
	}
}

static void distibute_tasks(const std::vector<ui32>& task_size, std::vector<ui32>& task_performer, ui32 rank, ui32 world_size) {
	ui32 n = task_size.size();
	task_performer.resize(n, 0);
	for (ui32 j = rank; j < n; j += world_size) {
		task_performer[j] = rank;
	}
}


int main(int argc, char** argv) {
	std::vector<ui32> task_size;
	std::vector<ui32> task_performer;
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
		//calculate task_size
		calculate_task_size(task_size, L.width());
		//distribute tasks
		distibute_tasks(task_size, task_performer, rank, world_size);
		//run
		solver.init(L, argv[2]);
		for (ui32 j = 0; j < L.width(); ++j) {
			if (task_performer[j] == rank) {
				solver.run(j);
			}
		}
		//reduce
		//int MPI_Reduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
		ui32 n_cov = solver.get_num();
		ui32 sum = 0;
		MPI_Reduce(&n_cov, &sum, 1, MPI_INT32_T, MPI_SUM, 0, MPI_COMM_WORLD);
		//time
		if (rank == 0) {
			wtime = MPI_Wtime() - wtime;
			printf("%d, %f\n", n_cov, wtime);
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