#include "dualizer_OPT.h"

#include <intrin.h>
#include <ammintrin.h>
#include <assert.h>
#include "DynamicArray.h"
#include "my_int.h"
#include "bool_matrix.h"
#include "my_memory.h"

#define BUF_MEMORY_SIZE 256

using namespace std;


//static const char ZERO_MEMORY[BUF_MEMORY_SIZE] = {0};
//static const char ONES_MEMORY[BUF_MEMORY_SIZE] = {~0};

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
		data_.Push_Empty();
		Element& tmp = top();
		tmp.rows.copy(rows);
		tmp.cols.copy(cols);
		tmp.support_rows.copy(support_rows);
		tmp.covered_rows.copy(covered_rows);
		tmp.h_last = h_last;
		tmp.j_last = j_last;
	}
	void pop() { data_.Pop(); }
	Element& top() { return data_.Top(); }
	bool empty() { return  data_.GetNum() == 0; }
	Stack(ui32 size = 16) { data_.Reserve(size); }
	~Stack() {}
private:
	DynamicArray<Element> data_;
};

static void update_one_sums(Bool_Vector& one_sums, const Bool_Vector& row_i, const Bool_Vector& rows) {
	for (ui32 ind = 0; ind < one_sums.size(); ++ind) {
		one_sums[ind] = (rows[ind] ^ row_i[ind]) & (rows[ind] | one_sums[ind]);
	}
}

void Dualizer_OPT::delete_le_rows(Bool_Vector& rows, const Bool_Vector& cols) const {
	if (cols.popcount() == 0)
		return;

	Bool_Vector buf;
	buf.assign(static_cast<ui32*>(My_Memory::MM_alloca(cols.size()*UI32_SIZE)), cols.bitsize());

	for (ui32 i1 = rows.find_next(0); i1 < rows.bitsize(); i1 = rows.find_next(i1 + 1)) {
		const Bool_Vector& row1 = L.row(i1);
		for (ui32 i2 = rows.find_next(0); i2 < rows.bitsize(); i2 = rows.find_next(i2 + 1)) {
			if (i2 == i1)
				continue;
			const Bool_Vector& row2 = L.row(i2);

			for (ui32 ind = 0; ind < row1.size(); ++ind) {
				buf[ind] = row1[ind] & ~row2[ind];
			}

			if (!buf.any())
				rows.reset(i2);
		}
	}
}

void Dualizer_OPT::delete_covered_rows(Bool_Vector& rows, const Bool_Vector& col_j) const {
	for (ui32 ind = 0; ind < rows.size(); ++ind) {
		rows[ind] = rows[ind] & ~col_j[ind];
	}
}

//void Dualizer_OPT::delete_zero_cols(const Bool_Vector& rows, Bool_Vector& cols, const Bool_Vector& mask) const {
//	for (ui32 j = cols.find_next(0); j < cols.bitsize(); j = cols.find_next(j + 1)) {
//		if (!any(L_t.row(j), mask))
//			cols.reset(j);
//	}
//}

void Dualizer_OPT::delete_fobidden_cols(const Bool_Vector& one_sums,
	Bool_Vector& cols, const Bool_Vector& cov) const
{
	Bool_Vector buf;
	buf.assign(static_cast<ui32*>(My_Memory::MM_alloca(one_sums.size()*UI32_SIZE)), one_sums.bitsize());

	for (ui32 u = cov.find_next(0); u < cov.bitsize(); u = cov.find_next(u + 1)) {
		const Bool_Vector& col_u = L_t.row(u);

		for (ui32 j = cols.find_next(0); j < cols.bitsize(); j = cols.find_next(j + 1)) {
			const Bool_Vector& col_j = L_t.row(j);

			for (ui32 ind = 0; ind < one_sums.size(); ++ind) {
				buf[ind] = ~col_j[ind] & col_u[ind] & one_sums[ind];
			}
			if (!buf.all()) {
				cols.reset(j);
			}
		}
	}
}

/*




void Dualizer_OPT::run() {
	Bool_Vector rows(1, L.height(), true);
	Bool_Vector cols(1, L.width() , true);



}

void Dualizer_OPT::init_masks() {
	ui32 ind = 0;
	mask_rows_ = static_cast<ui32*>(My_Memory::MM_malloc(L.row32()*UI32_SIZE));
	mask_cols_ = static_cast<ui32*>(My_Memory::MM_malloc(L_t.row32()*UI32_SIZE));
	if (mask_rows_ == nullptr || mask_cols_ == nullptr)
		throw std::runtime_error("Dualizer_OPT::run::Allocation memory problem");
	
	memset(mask_rows_, -1, L.row32()*UI32_SIZE);
	ind = L.width() & UI32_MASK;
	//mask_rows_[L.row32() - 1] = _bzhi_u32(-1, );

	memset(mask_cols_, -1, L_t.row32()*UI32_SIZE);
	ind = L_t.width() & UI32_MASK;
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