#include "dualizer_OPT.h"

#include <intrin.h>
#include <ammintrin.h>
#include "DynamicArray.h"
#include "my_int.h"
#include "bool_matrix.h"

using namespace std;

class Stack {
public:
	struct Element {
		Bool_Matrix rows;
		Bool_Matrix cols;
		Bool_Matrix support_rows;
		Bool_Matrix covered_rows;
		ui32 h_last;
		ui32 j_last;
	};
	void push(Bool_Matrix& rows, Bool_Matrix& cols, 
		Bool_Matrix& support_rows, Bool_Matrix& covered_rows,
		ui32 h_last, ui32 j_last) 
	{
		++ind_;
		if (ind_ >= sz_) {
			data_.Push(Element());
			++sz_;
		}
		Element& tmp = top();
		tmp.rows = rows;
		tmp.cols = cols;
		tmp.support_rows = support_rows;
		tmp.covered_rows = covered_rows;
		tmp.h_last = h_last;
		tmp.j_last = j_last;
	}
	void pop() {
		data_.Pop();
	}
	Element& top() {
		return data_.Top();
	}
	bool empty() {
		return  data_.GetNum() == 0;
	}
	Stack(ui32 size = 0) : ind_(0), sz_(0) {
		data_.Reserve(size);
	}
	~Stack() {}
private:
	DynamicArray<Element> data_;
	ui32 ind_;
	ui32 sz_;
};

static bool ge(const ui32* p1, const ui32* p2, const ui32* mask, ui32 n32) {
	ui32 buf = 0;
	for (ui32 i = 0; i < n32; ++i) {
		buf |= p1[i] & ~p2[i] & mask[i];
	}
	return buf == 0;
}

static bool all(const ui32* p, ui32 n32) {
	ui32 buf = ALL;
	for (ui32 ind = 0; ind < n32; ++ind) {
		buf &= p[ind];
	}
	return buf == ALL;
}

static bool any(const ui32* p, ui32 n32) {
	ui32 buf = 0;
	for (ui32 ind = 0; ind < n32; ++ind) {
		buf |= p[ind];
	}
	return buf != 0;
}

static void reset(ui32* p, ui32 n32, ui32 j) {
	//reset bits 1..j
	ui32 k = j >> LOG2BIT;	
	if (k < n32) {
		memset(p, 0, k*SIZE);
		ui32 mask = ALL << ((j & MASK) + 1);
		p[k + 1] &= mask;
	} else {
		memset(p, 0, n32*SIZE);
	}


}

static void update_one_sums(ui32* p_one_sums, const ui32* pj, const ui32* p_rows0, ui32 n32) {
	for (ui32 ind = 0; ind < n32; ++ind) {
		p_one_sums[ind] = (p_rows0[ind] ^ pj[ind]) & (p_rows0[ind] | p_one_sums[ind]);
	}
}

void Dualizer_OPT::delete_ge_rows(Bool_Matrix& rows, const Bool_Matrix& cols) const {
	if (cols.popcount() == 0)
		return;
	for (ui32 i1 = rows.find_next(0); i1 < rows.width(); i1 = rows.find_next(i1 + 1)) {
		for (ui32 i2 = rows.find_next(0); i2 < rows.width(); i2 = rows.find_next(i2 + 1)) {
			if (i2 == i1)
				continue;
			if (ge(L.row(i1), L.row(i2), rows.row(0), rows.size32())) {
				rows.reset(i2);
			}
		}
	}
}

void Dualizer_OPT::delete_covered_rows(Bool_Matrix& rows, ui32 j) const {
	ui32* x = rows.row(0);
	const ui32* y = L_t.row(j);
	for (ui32 ind = 0; ind < rows.size32(); ++ind) {
		x[ind] = x[ind] & ~y[ind];
	}
}

bool Dualizer_OPT::check_covering(const Bool_Matrix& covered_rows) const {
	return all(covered_rows.row(0), covered_rows.size32());
}

void Dualizer_OPT::delete_zero_cols(const Bool_Matrix& rows, Bool_Matrix& cols) const {
	for (ui32 j = cols.find_next(0); j < cols.width(); j = cols.find_next(j + 1)) {
		if (!any(L_t.row(j), rows.size32()))
			cols.reset(j);
	}
}

void Dualizer_OPT::delete_fobidden_cols(const Bool_Matrix& one_sums, 
	Bool_Matrix& cols, const Bool_Matrix& cov) const 
{
	const ui32* p = one_sums.row(0);
	const ui32* pj = nullptr;
	const ui32* pu = nullptr;
	ui32 buf = 0;
	for (ui32 u = cov.find_next(0); u < cov.width(); u = cov.find_next(u + 1)) {
		pu = L_t.row(u);
		for (ui32 j = cols.find_next(0); j < cols.width(); j = cols.find_next(j + 1)) {
			pj = L_t.row(j);
			buf = 0;
			for (ui32 ind = 0; ind < one_sums.size32(); ++ind) {
				buf |= ~pj[ind] & pu[ind] & p[ind];
			}
			if (buf ^ 1) {
				cols.reset(j);
			}
		}
	}
}