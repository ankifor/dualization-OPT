#include <cmath>
#include <cstdio>
#include "dualizer_OPT_parallel.h"


Dualizer_OPT_Parallel::Dualizer_OPT_Parallel():
rank(0), world_size(0), n_coverings(0), task_size(), task_performer(), solver(), L(), file_out(nullptr), wtime(0.0)
{
	MPI_Comm_size(MPI_COMM_WORLD, (int*) &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, (int*) &rank);
	if (rank == 0) {
		wtime = MPI_Wtime();
	}
}

Dualizer_OPT_Parallel::~Dualizer_OPT_Parallel() {
	if (file_out != nullptr) {
		My_Memory::MM_free(file_out);
	}
}

void Dualizer_OPT_Parallel::read_matrix(const char* filename) {
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

void Dualizer_OPT_Parallel::beta_scheme(double a, double b) {
	if (rank == 0) {
		if (a < 0.01 || b < 1.01) {
			throw std::runtime_error("beta_scheme::invalid parameters");
		}
		task_size.reserve(n());
		task_size.resize_to_capacity();
		double tmp = exp(-lgamma(n()) + lgamma(a) + lgamma(n() - 1 + b));
		task_size[0] = tmp;
		for (ui32 j = 1; j < n(); ++j) {
			tmp *= double(n() - j - 1) / double(j + 1) * (double(j) + a) / (double(n() - j - 2) + b);
			task_size[j] = tmp;
		}
	}
}

void Dualizer_OPT_Parallel::distribute_tasks() {
	task_performer.reserve(n());
	task_performer.resize_to_capacity();
	if (rank == 0) {
		Stack_Array<double> expected_load;
		expected_load.reserve(world_size);
		expected_load.resize_to_capacity();
		My_Memory::MM_memset(expected_load.get_data(), 0, world_size * sizeof(double));
		for (ui32 j = 0; j < n(); ++j) {
			ui32 k_min = 0;
			double val_min = 1e100;
			for (ui32 k = 0; k < world_size; ++k) {
				double val = expected_load[k];
				if (val < val_min) {
					val_min = val;
					k_min = k;
				}
			}
			task_performer[j] = k_min;
			expected_load[k_min] += task_size[j];
		}
	}
	MPI_Bcast(task_performer.get_data(), task_performer.size(), MPI_INT32_T, 0, MPI_COMM_WORLD);
}

void Dualizer_OPT_Parallel::reduce() {
	n_coverings = 0;
	ui32 n_cov = solver.get_num();
	printf("before reduce: %d %d\n", rank, n_cov);
	MPI_Reduce(&n_cov, &n_coverings, 1, MPI_INT32_T, MPI_SUM, 0, MPI_COMM_WORLD);
}

void Dualizer_OPT_Parallel::run() {
	for (ui32 j = 0; j < L.width(); ++j) {
		if (task_performer[j] == rank) {
			solver.init(L, file_out);
			solver.run(j);
		}
	}
}

void Dualizer_OPT_Parallel::set_file_out(char* src) {
	if (src != nullptr && strcmp(src, "NUL") != 0) {
		file_out = static_cast<char*>(My_Memory::MM_malloc(256));
		sprintf(file_out, "%s%d", src, rank);
	}
}

void Dualizer_OPT_Parallel::print() {
	if (rank == 0) {
		wtime = MPI_Wtime() - wtime;
		printf("%d, %f sec\n", n_coverings, wtime);
	}
}