#include "dualizer_OPT.h"

#include <intrin.h>
#include <ammintrin.h>
#include "DynamicArray.h"
#include "my_int.h"
#include "bool_matrix.h"
/*
using namespace std;

class Stack {
public:
	struct Element {
		Bool_Vector rows;
		Bool_Vector cols;
		Bool_Vector support_rows;
		Bool_Vector covered_rows;
		ui32 h_last;
		ui32 j_last;
	};
	void push(Bool_Vector& rows, Bool_Vector& cols, 
		Bool_Vector& support_rows, Bool_Vector& covered_rows,
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

static char any(const ui32* p, ui32 n32, const ui32* mask) {
	ui32 buf = 0;
	for (ui32 ind = 0; ind < n32; ++ind) {
		buf |= p[ind] & mask[ind];
	}
	return buf;
}

//nall(x)= any(~x)
static char nall(const ui32* p, ui32 n32, const ui32* mask) {
	ui32 buf = 0;
	for (ui32 ind = 0; ind < n32; ++ind) {
		buf |= ~p[ind] & mask[ind];
	}
	return buf;
}

static bool ge(const ui32* p1, const ui32* p2, const ui32* mask, ui32 n32) {
	ui32* buf = static_cast<ui32*>(alloca(n32*SIZE));
	ui32 i = 0;
	for (i = 0; i < n32 - 1; ++i) {
		buf[i] |= p1[i] & ~p2[i] & mask[i];
	}
	buf[i] = p1[i] & ~p2[i] & mask[i] & (1 << (n32 & MASK));
	return buf == 0;
}

static void reset(ui32* p, ui32 j, ui32 n32) {
	//reset bits 1..j
	ui32 k = j >> LOG2BIT;	
	if (k < n32) {
		memset(p, 0, k*SIZE);
		ui32 mask = ALL << (j & MASK);
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

void Dualizer_OPT::delete_ge_rows(Bool_Vector& rows, const Bool_Vector& cols) const {
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

void Dualizer_OPT::delete_covered_rows(Bool_Vector& rows, ui32 j) const {
	ui32* x = rows.row(0);
	const ui32* y = L_t.row(j);
	for (ui32 ind = 0; ind < rows.size32(); ++ind) {
		x[ind] = x[ind] & ~y[ind];
	}
}

bool Dualizer_OPT::check_covering(const Bool_Vector& covered_rows) const {
	return nall(covered_rows.row(0), covered_rows.size32());
}

void Dualizer_OPT::delete_zero_cols(const Bool_Vector& rows, Bool_Vector& cols) const {
	for (ui32 j = cols.find_next(0); j < cols.width(); j = cols.find_next(j + 1)) {
		if (!any(L_t.row(j), rows.size32()))
			cols.reset(j);
	}
}

void Dualizer_OPT::delete_fobidden_cols(const Bool_Vector& one_sums, 
	Bool_Vector& cols, const Bool_Vector& cov) const 
{
	const ui32* p = one_sums.row(0);
	const ui32* pj = nullptr;
	const ui32* pu = nullptr;
	//ui32 buf = 0;
	ui32 ind = 0;
	ui32* buf = static_cast<ui32*>(alloca(one_sums.size32()*SIZE));
	for (ui32 u = cov.find_next(0); u < cov.width(); u = cov.find_next(u + 1)) {
		pu = L_t.row(u);
		for (ui32 j = cols.find_next(0); j < cols.width(); j = cols.find_next(j + 1)) {
			pj = L_t.row(j);			
			for (ind = 0; ind < one_sums.size32(); ++ind) {
				buf[ind] = ~pj[ind] & pu[ind] & p[ind];
			}
			if (!all(buf, one_sums.size32())) {
				cols.reset(j);
			}
		}
	}
}

void Dualizer_OPT::run() {
	Bool_Vector rows(1, L.height(), true);
	Bool_Vector cols(1, L.width() , true);



}

void Dualizer_OPT::init_masks() {
	ui32 ind = 0;
	mask_rows_ = static_cast<ui32*>(malloc(L.row32()*SIZE));
	mask_cols_ = static_cast<ui32*>(malloc(L_t.row32()*SIZE));
	if (mask_rows_ == nullptr || mask_cols_ == nullptr)
		throw std::runtime_error("Dualizer_OPT::run::Allocation memory problem");
	
	memset(mask_rows_, -1, L.row32()*SIZE);
	ind = L.width() & MASK;
	//mask_rows_[L.row32() - 1] = _bzhi_u32(-1, );

	memset(mask_cols_, -1, L_t.row32()*SIZE);
	ind = L_t.width() & MASK;
	//mask_rows_[L_t.row32() - 1] = 0;
}

Dualizer_OPT::Dualizer_OPT() : mask_cols_(nullptr), mask_rows_(nullptr) {
}

Dualizer_OPT::~Dualizer_OPT() {
	clear();
}

void Dualizer_OPT::init(const Bool_Matrix& L0, const char* file_name) {
	p_file = fopen(file_name, "r");
	if (p_file == nullptr) {
		throw std::runtime_error(string("Dualizer_OPT::init::") + std::strerror(errno));
	}
	L = L0;
}

void Dualizer_OPT::clear() {
	fclose(p_file);
	if (mask_cols_ != nullptr) {
		free(mask_cols_);
		mask_cols_ = nullptr;
	}
	if (mask_rows_ != nullptr) {
		free(mask_rows_);
		mask_rows_ = nullptr;
	}
}
*/