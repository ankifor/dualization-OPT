#pragma once
#include "my_int.h"
#include "bool_matrix.h"

class Dualizer_OPT {
public:

protected:
	bool check_covering(const Bool_Matrix& covered_rows) const throw();
	void delete_ge_rows(Bool_Matrix& rows, const Bool_Matrix& cols) const throw();
	void delete_covered_rows(Bool_Matrix& rows, ui32 j) const throw();
	void delete_fobidden_cols(const Bool_Matrix& one_sums, 
		Bool_Matrix& cols, const Bool_Matrix& cov) const throw();
	void delete_zero_cols(const Bool_Matrix& rows, Bool_Matrix& cols) const throw();
	
	
protected:
	Bool_Matrix L;
	Bool_Matrix L_t;

};