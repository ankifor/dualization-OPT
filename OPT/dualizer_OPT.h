#pragma once
#include <cstdio>
#include "my_int.h"
#include "bool_vector.h"
#include "bool_matrix.h"

class Dualizer_OPT {
public:
	Dualizer_OPT();
	~Dualizer_OPT() throw();
	void init(const Bool_Matrix& L0, const char* file_name);
	void clear() throw();
	void run();
protected:
	bool check_covering(const Bool_Vector& covered_rows) const throw();
	void delete_ge_rows(Bool_Vector& rows, const Bool_Vector& cols) const throw();
	void delete_covered_rows(Bool_Vector& rows, ui32 j) const throw();
	void delete_fobidden_cols(const Bool_Vector& one_sums, 
		Bool_Vector& cols, const Bool_Vector& cov) const throw();
	void delete_zero_cols(const Bool_Vector& rows, Bool_Vector& cols) const throw();
	void init_masks();
	
protected:
	Dualizer_OPT(Dualizer_OPT const&) {};
	void operator = (Dualizer_OPT const&) {};
	Bool_Matrix L;
	Bool_Matrix L_t;
	ui32* mask_rows_;
	ui32* mask_cols_;
	FILE* p_file;
};