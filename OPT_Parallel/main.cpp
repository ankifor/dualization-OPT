#include <mpi.h>
#include <cstdio>
#include <iostream>
#include <cmath>

#include "dualizer_OPT_Parallel.h"
#include "stack_array.h"

//static double LogGamma(double x);
//
//static double Gamma(double x) {
//	assert(x > 0.0);
//	// Split the function domain into three intervals:
//	// (0, 0.001), [0.001, 12), and (12, infinity)
//
//	///////////////////////////////////////////////////////////////////////////
//	// First interval: (0, 0.001)
//	//
//	// For small x, 1/Gamma(x) has power series x + gamma x^2  - ...
//	// So in this range, 1/Gamma(x) = x + gamma x^2 with error on the order of x^3.
//	// The relative error over this interval is less than 6e-7.
//	const double gamma = 0.577215664901532860606512090; // Euler's gamma constant
//	if (x < 0.001)
//		return 1.0 / (x*(1.0 + gamma*x));
//	///////////////////////////////////////////////////////////////////////////
//	// Second interval: [0.001, 12)
//	if (x < 12.0) {
//		// The algorithm directly approximates gamma over (1,2) and uses
//		// reduction identities to reduce other arguments to this interval.
//		double y = x;
//		int n = 0;
//		bool arg_was_less_than_one = (y < 1.0);
//		// Add or subtract integers as necessary to bring y into (1,2)
//		// Will correct for this below
//		if (arg_was_less_than_one) {
//			y += 1.0;
//		} else {
//			n = static_cast<int> (floor(y)) - 1;  // will use n later
//			y -= n;
//		}
//		// numerator coefficients for approximation over the interval (1,2)
//		static const double p[] = {
//			-1.71618513886549492533811E+0,
//			2.47656508055759199108314E+1,
//			-3.79804256470945635097577E+2,
//			6.29331155312818442661052E+2,
//			8.66966202790413211295064E+2,
//			-3.14512729688483675254357E+4,
//			-3.61444134186911729807069E+4,
//			6.64561438202405440627855E+4
//		};
//		// denominator coefficients for approximation over the interval (1,2)
//		static const double q[] = {
//			-3.08402300119738975254353E+1,
//			3.15350626979604161529144E+2,
//			-1.01515636749021914166146E+3,
//			-3.10777167157231109440444E+3,
//			2.25381184209801510330112E+4,
//			4.75584627752788110767815E+3,
//			-1.34659959864969306392456E+5,
//			-1.15132259675553483497211E+5
//		};
//		double num = 0.0;
//		double den = 1.0;
//		int i;
//		double z = y - 1;
//		for (i = 0; i < 8; i++) {
//			num = (num + p[i])*z;
//			den = den*z + q[i];
//		}
//		double result = num / den + 1.0;
//		// Apply correction if argument was not initially in (1,2)
//		if (arg_was_less_than_one) {
//			// Use identity gamma(z) = gamma(z+1)/z
//			// The variable "result" now holds gamma of the original y + 1
//			// Thus we use y-1 to get back the orginal y.
//			result /= (y - 1.0);
//		} else {
//			// Use the identity gamma(z+n) = z*(z+1)* ... *(z+n-1)*gamma(z)
//			for (i = 0; i < n; i++)
//				result *= y++;
//		}
//		return result;
//	}
//	///////////////////////////////////////////////////////////////////////////
//	// Third interval: [12, infinity)
//	if (x > 171.624) {
//		// Correct answer too large to display. Force +infinity.
//		double temp = DBL_MAX;
//		return temp*2.0;
//	}
//	return exp(LogGamma(x));
//}
//
//static double LogGamma(double x) {
//	assert(x > 0.0);
//	if (x < 12.0) {
//		return log(fabs(Gamma(x)));
//	}
//	// Abramowitz and Stegun 6.1.41
//	// Asymptotic series should be good to at least 11 or 12 figures
//	// For error analysis, see Whittiker and Watson
//	// A Course in Modern Analysis (1927), page 252
//	static const double c[8] = {
//		1.0 / 12.0,
//		-1.0 / 360.0,
//		1.0 / 1260.0,
//		-1.0 / 1680.0,
//		1.0 / 1188.0,
//		-691.0 / 360360.0,
//		1.0 / 156.0,
//		-3617.0 / 122400.0
//	};
//	double z = 1.0 / (x*x);
//	double sum = c[7];
//	for (int i = 6; i >= 0; i--) {
//		sum *= z;
//		sum += c[i];
//	}
//	double series = sum / x;
//
//	static const double halfLogTwoPi = 0.91893853320467274178032973640562;
//	double logGamma = (x - 0.5)*log(x) - x + halfLogTwoPi + series;
//	return logGamma;
//}

//static void read_matrix(binary::Matrix& L, const char* filename, ui32 rank) {
//	ui32 sz[2];
//	if (rank == 0) {
//		L.read(filename);
//		L.delete_le_rows();
//		sz[0] = L.height();
//		sz[1] = L.width();
//	}
//	MPI_Bcast(sz, 2, MPI_INT32_T, 0, MPI_COMM_WORLD);
//	if (rank != 0) {
//		L.reserve(sz[0], sz[1]);
//	}
//	MPI_Bcast(L.row(0), L.size32(), MPI_INT32_T, 0, MPI_COMM_WORLD);
//}
//
//static void beta_scheme(Stack_Array<double>& task_size, ui32 n, double a, double b, ui32 rank) {
//	//performed by rank==0
//	if (a < 0.01 || b < 1.01) {
//		throw std::runtime_error("beta_scheme::invalid parameters");
//	}
//	task_size.reserve(n);
//	task_size.resize_to_capacity();
//	double tmp = exp(-lgamma(n) + lgamma(a) + lgamma(n - 1 + b));
//	task_size[0] = tmp;
//	for (ui32 j = 1; j < n; ++j) {
//		tmp *= double(n - j - 1) / double(j + 1) * (double(j) + a) / (double(n - j - 2) + b);
//		task_size[j] = tmp;
//	}
//}
//
//static void distribute_tasks(Stack_Array<ui32>& task_performer, 
//	const Stack_Array<double>& task_size, ui32 n, ui32 rank, ui32 world_size) 
//{
//	task_performer.reserve(n);
//	task_performer.resize_to_capacity();
//	if (rank == 0) {
//		Stack_Array<double> expected_load;
//		expected_load.reserve(world_size);
//		expected_load.resize_to_capacity();
//		My_Memory::MM_memset(expected_load.get_data(), 0, world_size * sizeof(double));
//		for (ui32 j = 0; j < n; ++j) {
//			ui32 k_min = 0;
//			double val_min = 1e100;
//			for (ui32 k = 0; k < world_size; ++k) {
//				double val = expected_load[k];
//				if (val < val_min) {
//					val_min = val;
//					k_min = k;
//				}
//			}
//			task_performer[j] = k_min;
//			expected_load[k_min] += task_size[j];
//		}
//
//		//ui32 performer = 0;
//		//for (ui32 j = 0; j < n; ++j) {
//		//	task_performer[j] = performer;
//		//	performer = (performer >= world_size - 1 ? 0 : performer + 1);
//		//}
//		
//		//for (ui32 j = 0; j < n; ++j) {
//		//	printf("%d ", task_performer[j]);
//		//}
//		//printf("\n");
//		//
//		//for (ui32 k = 0; k < world_size; ++k) {
//		//	printf("%f ", expected_load[k]);
//		//}
//		//printf("\n");
//		//fflush(stdout);
//	}
//	MPI_Bcast(task_performer.get_data(), task_performer.size(), MPI_INT32_T, 0, MPI_COMM_WORLD);
//}
//
//ui32 reduce_solver(Dualizer_OPT& solver, ui32 rank) {
//	ui32 sum = 0;
//	ui32 n_cov = solver.get_num();
//	printf("before reduce: %d %d\n", rank, n_cov);
//	MPI_Reduce(&n_cov, &sum, 1, MPI_INT32_T, MPI_SUM, 0, MPI_COMM_WORLD);	
//	return sum;
//}

//int main(int argc, char** argv) {	
//	// Initialize the MPI environment
//	MPI_Init(&argc, &argv);	
//
//	Stack_Array<ui32> task_performer;
//	Stack_Array<double> task_size;
//	int world_size = 0;
//	int rank = 0;
//
//
//	
//	binary::Matrix L;
//	
//	try {
//		double wtime = 0.0;
//		if (rank == 0) {
//			wtime = MPI_Wtime();
//		}
//		read_matrix(L, argv[1], rank);
//
//		MPI_Barrier(MPI_COMM_WORLD);
//		//calculate tasks
//		double a = atof(argv[3]);
//		double b = atof(argv[4]);
//		if (rank == 0) {			
//			beta_scheme(task_size, L.width(), a, b, rank);
//		}
//		//distribute tasks
//		MPI_Barrier(MPI_COMM_WORLD);
//		distribute_tasks(task_performer, task_size, L.width(), rank, world_size);
//		MPI_Barrier(MPI_COMM_WORLD);
//		//prepare file_out
//		char* file_out = nullptr;
//		if (argv[2] != nullptr && strcmp(argv[2], "NUL") != 0) {
//			file_out = static_cast<char*>(alloca(128));
//			sprintf(file_out, "%s%d", argv[2], rank);
//		}
//		//run dualizer
//		Dualizer_OPT solver;
//		for (ui32 j = 0; j < L.width(); ++j) {
//			if (task_performer[j] == rank) {
//				solver.init(L, file_out);
//				solver.run(j);
//			}
//		}
//		//reduce
//		ui32 sum = reduce_solver(solver, rank);
//		//time		
//		if (rank == 0) {
//			wtime = MPI_Wtime() - wtime;
//			printf("%d, %f sec\n", sum, wtime);
//		}
//	} catch (std::runtime_error& rte) {
//		std::cout << rte.what() << " sec" << std::endl;
//		MPI_Abort(MPI_COMM_WORLD, -1);
//	} catch (...) {
//		std::cout << "Unknown error" << std::endl;
//		MPI_Abort(MPI_COMM_WORLD, -1);
//	}
//	// Finalize the MPI environment.
//	MPI_Finalize();
//}

int main(int argc, char** argv) {
	// Initialize the MPI environment
	MPI_Init(&argc, &argv);

	Dualizer_OPT_Parallel solver;
	try {		
		solver.read_matrix(argv[1]);

		int rank = 0;
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);

		double a = 0.0;
		double b = 0.0;

		if (rank == 0) {
			a = atof(argv[3]);
			b = atof(argv[4]);
		}
		solver.beta_scheme(a, b);
		solver.distribute_tasks();
		solver.set_file_out(argv[2]);
		solver.run();
		solver.reduce();
		solver.print();
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