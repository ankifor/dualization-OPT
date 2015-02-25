#pragma once
#include <cstdio>
#include "my_int.h"
#include "bool_vector.h"
#include "bool_matrix.h"

class Dualizer_OPT {

public:

	Dualizer_OPT() throw() : p_file(nullptr), n_coverings(0) {}
	~Dualizer_OPT() throw() { clear(); }

	void init(const Bool_Matrix& L0, const char* file_name = nullptr, const char* mode = "w");
	void clear() throw();
	void run();

protected:

	void delete_zero_cols(const Bool_Vector& rows, Bool_Vector& cols) const throw();
	void delete_le_rows(Bool_Vector& rows, const Bool_Vector& cols) const throw();
	void delete_fobidden_cols(const Bool_Vector& one_sums, 
		Bool_Vector& cols, const Bool_Vector& cov) const throw();

	void print_covering(const Bool_Vector& covering);
	
private:

	Dualizer_OPT(Dualizer_OPT const&) {};
	void operator = (Dualizer_OPT const&) {};

protected:

	Bool_Matrix L;
	Bool_Matrix L_t;
	ui32 n_coverings;
	FILE* p_file;
};