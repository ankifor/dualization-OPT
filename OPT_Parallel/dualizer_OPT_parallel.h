#pragma once

#include <mpi.h>
#include "binary.h"
#include "stack_array.h"
#include "dualizer_OPT.h"

class Dualizer_OPT_Parallel {
public:
	Dualizer_OPT_Parallel();
	~Dualizer_OPT_Parallel();
	
	void read_matrix(const char* filename);
	void beta_scheme(double a, double b);
	void distribute_tasks();
	void reduce();
	void set_file_out(char* src);
	void run();
	void print();

private:
	Dualizer_OPT_Parallel(const Dualizer_OPT_Parallel &);
	Dualizer_OPT_Parallel& operator=(const Dualizer_OPT_Parallel&);
	ui32 n() { return L.width(); }

	Dualizer_OPT solver;
	binary::Matrix L;
	Stack_Array<double> task_size;
	Stack_Array<ui32> task_performer;
	char* file_out;
	ui32 rank;
	ui32 world_size;
	ui32 n_coverings;
	double wtime;
};