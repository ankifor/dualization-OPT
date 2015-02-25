#include <assert.h>//for assert
#include <string> //for string(), +

#include "my_int.h"
#include "my_memory.h"
#include "stack_array.h"
#include "bool_matrix.h"

#include "dualizer_OPT.h"

using namespace std;

class Stack {
public:
	struct Element {
		Bool_Vector rows;
		Bool_Vector cols;
		Bool_Vector support_rows;
		Bool_Vector covered_rows;
		ui32 h_last;
		ui32 j_next;
	};
	
	void push(Bool_Vector& rows, Bool_Vector& cols, 
		Bool_Vector& support_rows, Bool_Vector& covered_rows,
		ui32 h_last, ui32 j_next)
	{
		data_.push_empty();
		Element& tmp = data_.top();
		tmp.rows.copy(rows);
		tmp.cols.copy(cols);
		tmp.support_rows.copy(support_rows);
		tmp.covered_rows.copy(covered_rows);
		tmp.h_last = h_last;
		tmp.j_next = j_next;
	}

	void pop() { data_.pop(); }

	void copy_top(Bool_Vector& rows, Bool_Vector& cols,
		Bool_Vector& support_rows, Bool_Vector& covered_rows,
		ui32& h_last, ui32& j_next) 
	{
		Element& tmp = data_.top();
		rows.copy(tmp.rows);
		cols.copy(tmp.cols);
		support_rows.copy(tmp.support_rows);
		covered_rows.copy(tmp.covered_rows);
		h_last = tmp.h_last;
		j_next = tmp.j_next;
	}

	void update_j_next(ui32 j_next) { data_.top().j_next = j_next; }
	bool empty() { return  data_.size() == 0; }
	int size() { return data_.size(); }
	Stack(ui32 size = 16) { data_.reserve(size); }
	~Stack() {}
private:
	Stack_Array<Element> data_;
};

static void update_covered_and_support_rows(Bool_Vector& rows, Bool_Vector& covered_rows, 
	Bool_Vector& support_rows, const Bool_Vector& col_j) throw()
{
	for (ui32 ind = 0; ind < rows.size(); ++ind) {
		support_rows[ind] = (~rows[ind] ^ col_j[ind]) & (rows[ind] | support_rows[ind]);
		rows[ind] &= ~col_j[ind];
		covered_rows[ind] |= col_j[ind];
	}
}

void Dualizer_OPT::delete_zero_cols(const Bool_Vector& rows, Bool_Vector& cols) const throw() {
	Bool_Vector buf;
	buf.assign(static_cast<ui32*>(alloca(rows.size()*UI32_SIZE)), rows.bitsize());

	for (ui32 j = cols.find_next(0); j < cols.bitsize(); j = cols.find_next(j + 1)) {
		const Bool_Vector& col_j = L_t.row(j);

		for (ui32 ind = 0; ind < buf.size(); ++ind) {
			buf[ind] = rows[ind] & col_j[ind];
		}

		if (!buf.any())
			cols.reset(j);
	}
}

void Dualizer_OPT::delete_le_rows(Bool_Vector& rows, const Bool_Vector& cols) const throw() {
	if (cols.popcount() == 0)
		return;

	Bool_Vector buf;
	buf.assign(static_cast<ui32*>(alloca(cols.size()*UI32_SIZE)), cols.bitsize());

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

void Dualizer_OPT::delete_fobidden_cols(const Bool_Vector& support_rows,
	Bool_Vector& cols, const Bool_Vector& cov) const throw()
{
	Bool_Vector buf1;
	buf1.assign(static_cast<ui32*>(alloca(support_rows.size()*UI32_SIZE)), support_rows.bitsize());

	Bool_Vector buf2;
	buf2.assign(static_cast<ui32*>(alloca(support_rows.size()*UI32_SIZE)), support_rows.bitsize());

	for (ui32 u = cov.find_next(0); u < cov.bitsize(); u = cov.find_next(u + 1)) {
		const Bool_Vector& col_u = L_t.row(u);

		for (ui32 ind = 0; ind < support_rows.size(); ++ind) {
			buf1[ind] = col_u[ind] & support_rows[ind];
		}

		for (ui32 j = cols.find_next(0); j < cols.bitsize(); j = cols.find_next(j + 1)) {
			const Bool_Vector& col_j = L_t.row(j);

			for (ui32 ind = 0; ind < support_rows.size(); ++ind) {
				buf2[ind] = col_j[ind] | ~buf1[ind];
			}
			if (buf2.all()) {
				cols.reset(j);
			}
		}
	}
}

void Dualizer_OPT::run() {
	Bool_Vector rows;
	rows.reserve(L.height());
	rows.setall();

	Bool_Vector cols;
	cols.reserve(L.width());
	cols.setall();
	
	Bool_Vector covering;
	covering.reserve(L.width());
	covering.resetall();

	Bool_Vector support_rows;
	support_rows.reserve(L.width());
	support_rows.resetall();

	Bool_Vector covered_rows;
	covered_rows.reserve(L.height());
	covered_rows.resetall();

	Stack stack;
	stack.push(rows, cols, support_rows, covered_rows, 0, 0);

	ui32 h = 0;
	ui32 j = 0;
	while (!stack.empty()) {		
		stack.copy_top(rows, cols, support_rows, covered_rows, h, j);
		j = cols.find_next(j);

		if (j >= cols.bitsize()) {
			//all children are finished
			stack.pop();
			if (stack.size() > 0)
				covering.reset(h);
			continue;
		}

		stack.update_j_next(j + 1);
		cols.resetupto(j);		
		covering.set(j);
		update_covered_and_support_rows(rows, covered_rows, support_rows, L_t.row(j));

		if (!rows.any()) {
			//leaf, it might be false positive
			if (covered_rows.all()) {
				//irreducible covering (true positive)
				print_covering(covering);
				++n_coverings;
			}
			covering.reset(j);
			continue;
		}

		delete_fobidden_cols(support_rows, cols, covering);
		delete_le_rows(rows, cols);
		delete_zero_cols(rows, cols);

		stack.push(rows, cols, support_rows, covered_rows, j, 0);
	}

	printf("irreducible coverings: %d\n", n_coverings);
}

void Dualizer_OPT::init(const Bool_Matrix& L0, const char* file_name, const char* mode) {
	if (file_name != nullptr) {
		p_file = fopen(file_name, mode);
		if (p_file == nullptr) {
			throw std::runtime_error(string("Dualizer_OPT::init::") + std::strerror(errno));
		}
	}
	L = L0;

	Bool_Vector rows;
	rows.reserve(L.height());
	rows.setall();

	Bool_Vector cols;
	cols.reserve(L.width());
	cols.setall();

	delete_le_rows(rows, cols);
	L.submatrix(rows);
	L_t.transpose(L);
}

void Dualizer_OPT::clear() {
	if (p_file != nullptr)
		fclose(p_file);
}

void Dualizer_OPT::print_covering(const Bool_Vector& cov) {
	//non-opitmized
	bool any = false;
	for (ui32 j = cov.find_next(0); j < cov.bitsize(); j = cov.find_next(j + 1)) {
		fprintf(p_file, "%d ", j);
		any = true;
	}
	if (any)
		fputc('\n', p_file);
}