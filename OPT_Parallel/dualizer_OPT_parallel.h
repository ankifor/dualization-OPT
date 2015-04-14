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

	void beta_scheme(const char* text_a, const char* text_b);
	void stripe_scheme(const char* text_u, const char* text_times);

	void distribute_tasks();
	
	void set_file_out(char* src);

	void run();

	void reduce();

	void print();

private:
	Dualizer_OPT_Parallel(const Dualizer_OPT_Parallel &);
	Dualizer_OPT_Parallel& operator=(const Dualizer_OPT_Parallel&);
	ui32 n() { return L.width(); }

	Dualizer_OPT solver;
	binary::Matrix L;
	Stack_Array<double> task_size;
	Stack_Array<ui32> task_performer;
	Stack_Array<ui32> frequency;
	char* file_out;
	ui32 rank;
	ui32 world_size;
	ui32 n_coverings;
	double wtime_begin;
	double wtime_scheme;
};