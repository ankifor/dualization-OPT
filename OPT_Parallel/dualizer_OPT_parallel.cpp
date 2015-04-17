#include <cmath>
#include <cstdio>
#include "dualizer_OPT_parallel.h"


Dualizer_OPT_Parallel::Dualizer_OPT_Parallel():
	rank(0), world_size(0), n_coverings(0), file_out(nullptr), 
	wtime_begin(0.0), wtime_scheme(0.0)
{
	MPI_Comm_size(MPI_COMM_WORLD, (int*) &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, (int*) &rank);
	if (rank == 0) {
		wtime_begin = MPI_Wtime();
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
//#pragma warning(suppress: 6001)
	MPI_Bcast(sz, 2, MPI_INT, 0, MPI_COMM_WORLD);
	if (rank != 0) {
		L.reserve(sz[0], sz[1]);
	}
	MPI_Bcast(L.row(0), L.size32(), MPI_INT, 0, MPI_COMM_WORLD);
	solver.init(L);//reserve memory
}

void Dualizer_OPT_Parallel::beta_scheme(const char* text_a, const char* text_b) {
	if (rank == 0) {
		double a = atof(text_a);
		double b = atof(text_b);
		if (a < 0.01 || b < 1.01) {
			throw std::runtime_error("beta_scheme::invalid parameters");
		}
		task_size.resize(n());
		double tmp = exp(-lgamma(n()) + lgamma(a) + lgamma(n() - 1 + b));
		task_size[0] = tmp;
		for (ui32 j = 1; j < n(); ++j) {
			tmp *= double(n() - j - 1) / double(j + 1) * (double(j) + a) / (double(n() - j - 2) + b);
			task_size[j] = tmp;
		}
		wtime_scheme = MPI_Wtime() - wtime_begin;
	}
}

void Dualizer_OPT_Parallel::stripe_scheme(const char* text_u, const char* text_times) {
	ui32 u = atoi(text_u);
	ui32 times = atoi(text_times);
	if (u >= n() || times == 0) {
		throw std::runtime_error("Dualizer_OPT_Parallel::stripe_scheme::invalid paramtres");
	}
	binary::Matrix L_u;//stripe-submatrix
	//Stack_Array<ui32> frequency;
	//frequency.resize(n());
	if (rank == 0) {
		task_size.resize(n());
		My_Memory::MM_memset(task_size.get_data(), 0, n() * UI32_SIZE);
	}

	for (ui32 i = rank; i < times; i += world_size) {
		L_u.random_stripe(L, u);
		L_u.delete_le_rows();
		solver.init(L_u, nullptr, nullptr, false);
		solver.run();
		//const Stack_Array<ui32>& freq0 = solver.get_freq();
		//for (ui32 j = 0; j < n(); ++j) {
		//	frequency[j] += freq0[j];
		//}
	}
	MPI_Reduce(solver.get_freq().get_data(), task_size.get_data(), n(), MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	if (rank == 0) {
		wtime_scheme = MPI_Wtime() - wtime_begin;
	}
}

void Dualizer_OPT_Parallel::distribute_tasks() {
	task_performer.resize(n());
	if (rank == 0) {
		Stack_Array<double> expected_load;
		expected_load.resize(world_size);
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
	MPI_Bcast(task_performer.get_data(), task_performer.size(), MPI_INT, 0, MPI_COMM_WORLD);
}

void Dualizer_OPT_Parallel::reduce() {
	n_coverings = 0;
	ui32 n_cov = solver.get_num();
	printf("before reduce: %d %d\n", rank, n_cov);
	//MPI_Reduce(&n_cov, &n_coverings, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	frequency.resize(n());
	MPI_Reduce(solver.get_freq().get_data(), frequency.get_data(), n(), MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	for (ui32 j = 0; j < frequency.size(); ++j) {
		n_coverings += frequency[j];
	}
}

void Dualizer_OPT_Parallel::run() {
	solver.init(L, file_out);
	for (ui32 j = 0; j < L.width(); ++j) {
		if (task_performer[j] == rank) {
			solver.reinit();
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
		printf("%d, %f, %f sec\n", n_coverings, wtime_scheme, MPI_Wtime() - wtime_begin);
		for (ui32 j = 0; j < frequency.size(); ++j) {
			printf("%d ", frequency[j]);
		}
		printf("\n");
	}
}