#include <assert.h>//for assert
#include <string> //for string(), +

#include "dualizer_OPT.h"
#include "my_memory.h"




using namespace std;

class Stack {
public:
	struct Element {
		ui32* p;
		Element() : p(nullptr) {}
		~Element() { 
			if (p != nullptr) {
				My_Memory::MM_free(p);
				p = nullptr;
			}
		}
	};
	
	void push(ui32* rows, ui32* cols, 
		ui32* support_rows, ui32* covered_rows,
		ui32 h_last, ui32 j_next)
	{
		data_.push_empty();
		
		if (data_.top().p == nullptr) {
			data_.top().p = static_cast<ui32*>(My_Memory::MM_malloc(element_size * UI32_SIZE));
		}
		
		ui32* dst = data_.top().p;
		My_Memory::MM_memcpy(dst, rows, m_ * UI32_SIZE); dst += m_;
		My_Memory::MM_memcpy(dst, support_rows, m_ * UI32_SIZE); dst += m_;
		My_Memory::MM_memcpy(dst, covered_rows, m_ * UI32_SIZE); dst += m_;
		My_Memory::MM_memcpy(dst, cols, n_ * UI32_SIZE); dst += n_;
		*dst = h_last; dst += 1;
		*dst = j_next;
	}

	ui32& top_j_next() { return *(data_.top().p + 3 * m_ + n_ + 1); }

	void pop() throw() { data_.pop(); }

	void copy_top(ui32* rows, ui32* cols,
		ui32* support_rows, ui32* covered_rows,
		ui32& h_last, ui32& j_next) throw()
	{
		ui32* dst = data_.top().p;
		My_Memory::MM_memcpy(rows, dst, m_ * UI32_SIZE); dst += m_;
		My_Memory::MM_memcpy(support_rows, dst, m_ * UI32_SIZE); dst += m_;
		My_Memory::MM_memcpy(covered_rows, dst, m_ * UI32_SIZE); dst += m_;
		My_Memory::MM_memcpy(cols, dst, n_ * UI32_SIZE); dst += n_;
		h_last = *dst; dst += 1;
		j_next = *dst;
	}


	bool empty() const throw() { return  data_.size() == 0; }

	int size() const throw() { return data_.size(); }

	Stack(ui32 m, ui32 n, ui32 size = 16) { 
		data_.reserve(size); 
		m_ = m;
		n_ = n;
		element_size = 3 * m_ + n_ + 2;
	}

	~Stack() {}
private:

	Stack_Array<Element> data_;
	ui32 m_;
	ui32 n_;
	ui32 element_size;
};

void Dualizer_OPT::update_covered_and_support_rows(ui32* rows, ui32* covered_rows,
	ui32* support_rows, const ui32* col_j) const throw()
{
	for (ui32 ind = 0; ind < size_m(); ++ind) {
		support_rows[ind] = (~rows[ind] ^ col_j[ind]) & (rows[ind] | support_rows[ind]);
		rows[ind] &= ~col_j[ind];
		covered_rows[ind] |= col_j[ind];
	}
}

void Dualizer_OPT::delete_zero_cols(const ui32* rows, ui32* cols) const throw() {
	ui32* buf = static_cast<ui32*>(alloca(size_m()*UI32_SIZE));
	ui32 j = binary::find_next(cols, n(), 0);

	while (j < n()) {
		const ui32* col_j = L_t.row(j);

		for (ui32 ind = 0; ind < size_m(); ++ind) {
			buf[ind] = rows[ind] & col_j[ind];
		}

		if (!binary::any(buf, m()))
			binary::reset(cols, j);

		j = binary::find_next(cols, n(), j + 1);
	}

}

void Dualizer_OPT::delete_le_rows(ui32* rows, const ui32* cols) const throw() {
	if (binary::popcount(cols, n()) == 0)
		return;

	ui32* buf = static_cast<ui32*>(alloca(size_n()*UI32_SIZE));

	ui32 i1 = binary::find_next(rows, m(), 0);
	ui32 i2 = 0;

	while (i1 < m()) {
		const ui32* row1 = L.row(i1);
		i2 = binary::find_next(rows, m(), 0);
		while (i2 < m()) {
			if (i2 != i1) {
				const ui32* row2 = L.row(i2);

				for (ui32 ind = 0; ind < size_n(); ++ind) {
					buf[ind] = row1[ind] & ~row2[ind];
				}

				if (!binary::any(buf, n()))
					binary::reset(rows, i2);
			}
			i2 = binary::find_next(rows, m(), i2 + 1);
		}
		i1 = binary::find_next(rows, m(), i1 + 1);
	}

}

void Dualizer_OPT::delete_fobidden_cols(const ui32* support_rows,
	ui32* cols, const Covering& cov) const throw()
{
	ui32* buf1 = static_cast<ui32*>(alloca(size_n()*UI32_SIZE));
	ui32* buf2 = static_cast<ui32*>(alloca(size_n()*UI32_SIZE));

	ui32 u = 0;
	ui32 j = 0;

	while (u < cov.size()) {
		const ui32* col_u = L_t.row(cov[u]);
		
		for (ui32 ind = 0; ind < size_m(); ++ind) {
			buf1[ind] = col_u[ind] & support_rows[ind];
		}

		j = binary::find_next(cols, n(), 0);
		while (j < n()) {
			const ui32* col_j = L_t.row(j);
			
			for (ui32 ind = 0; ind < size_m(); ++ind) {
				buf2[ind] = col_j[ind] | ~buf1[ind];
			}

			if (binary::all(buf2, m()))
				binary::reset(cols, j);

			j = binary::find_next(cols, n(), j + 1);
		}
		++u;
	}

}

void Dualizer_OPT::run() {
	Covering covering;
	covering.reserve(20);

	ui32* rows = static_cast<ui32*>(My_Memory::MM_malloc(size_m()*UI32_SIZE));
	My_Memory::MM_memset(rows, ~0, size_m()*UI32_SIZE);

	ui32* support_rows = static_cast<ui32*>(My_Memory::MM_malloc(size_m()*UI32_SIZE));
	My_Memory::MM_memset(support_rows, 0, size_m()*UI32_SIZE);

	ui32* covered_rows = static_cast<ui32*>(My_Memory::MM_malloc(size_m()*UI32_SIZE));
	My_Memory::MM_memset(covered_rows, 0, size_m()*UI32_SIZE);

	ui32* cols = static_cast<ui32*>(My_Memory::MM_malloc(size_n()*UI32_SIZE));
	My_Memory::MM_memset(cols, ~0, size_n()*UI32_SIZE);

	Stack stack(m(), n());
	stack.push(rows, cols, support_rows, covered_rows, 0, 0);

	ui32 h = 0;
	ui32 j = 0;
	bool up_to_date = false;

	while (!stack.empty()) {		
		if (!up_to_date)
			stack.copy_top(rows, cols, support_rows, covered_rows, h, j);
		j = binary::find_next(cols, n(), j);

		if (j >= n()) {
			//all children are finished
			stack.pop();
			if (stack.size() > 0) {
				covering.remove_last();
			}
			up_to_date = false;
			continue;
		}

		stack.top_j_next() = j + 1;
		binary::reset_le(cols, j);
		covering.append(j);
		update_covered_and_support_rows(rows, covered_rows, support_rows, L_t.row(j));

		if (!binary::any(rows, m())) {
			//leaf, it might be false positive
			if (binary::all(covered_rows, m())) {
				//irreducible covering (true positive)
				covering.print(p_file);
				++n_coverings;
			}
			covering.remove_last();
			up_to_date = false;
			continue;
		}

		delete_fobidden_cols(support_rows, cols, covering);
		delete_le_rows(rows, cols);
		delete_zero_cols(rows, cols);

		stack.push(rows, cols, support_rows, covered_rows, j, 0);
		j = 0;
		up_to_date = true;
	}

	printf("Irreducible coverings: %d\n", n_coverings);

	My_Memory::MM_free(rows);
	My_Memory::MM_free(support_rows);
	My_Memory::MM_free(covered_rows);
	My_Memory::MM_free(cols);
}

void Dualizer_OPT::init(const binary::Matrix& L0, const char* file_name, const char* mode) {
	if (file_name != nullptr) {
		p_file = fopen(file_name, mode);
		if (p_file == nullptr) {
			throw std::runtime_error(string("Dualizer_OPT::init::") + std::strerror(errno));
		}
	}
	L = L0;

	ui32* rows = static_cast<ui32*>(My_Memory::MM_malloc(size_m()*UI32_SIZE));
	My_Memory::MM_memset(rows, ~0, size_m()*UI32_SIZE);

	ui32* cols = static_cast<ui32*>(My_Memory::MM_malloc(size_n()*UI32_SIZE));
	My_Memory::MM_memset(cols, ~0, size_n()*UI32_SIZE);

	delete_le_rows(rows, cols);
	L.submatrix(rows);
	L_t.transpose(L);

	My_Memory::MM_free(rows);
	My_Memory::MM_free(cols);
}

void Dualizer_OPT::clear() {
	if (p_file != nullptr)
		fclose(p_file);
}

