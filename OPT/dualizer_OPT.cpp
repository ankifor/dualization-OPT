#include <assert.h>//for assert
#include <string> //for string(), +
#include <intrin.h>


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
	
	void push(const ui32* pool, ui32 h_last, ui32 j_next) {
		data_.push_empty();
		
		if (data_.top().p == nullptr) {
			data_.top().p = static_cast<ui32*>(My_Memory::MM_malloc(element_size_ * UI32_SIZE));
		}
		
		ui32* dst = data_.top().p;
		My_Memory::MM_memcpy(dst, pool, pool_size_ * UI32_SIZE); dst += pool_size_;
		*dst = h_last; dst += 1;
		*dst = j_next;
	}

	void update_j_next(ui32 j_next) { 	
		ui32* dst = data_.top().p + pool_size_ + 1;
		*dst = j_next;
	}

	void pop() throw() { data_.pop(); }

	void copy_top(ui32* pool, ui32& h_last, ui32& j_next) throw() {
		ui32* src = data_.top().p;
		My_Memory::MM_memcpy(pool, src, pool_size_ * UI32_SIZE); src += pool_size_;
		h_last = *src; src += 1;
		j_next = *src;
	}


	bool empty() const throw() { return  data_.size() == 0; }

	int size() const throw() { return data_.size(); }

	Stack(ui32 pool_size, ui32 size = 16) { 
		data_.reserve(size); 
		element_size_ = pool_size + 2;
		pool_size_ = pool_size;
	}

	~Stack() {}
private:

	Stack_Array<Element> data_;
	ui32 pool_size_;
	ui32 element_size_;
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
	ui32* buf = static_cast<ui32*>(alloca(size_n()*UI32_SIZE));

	ui32 u = 0;
	ui32 j = 0;

	while (u < cov.size()) {
		const ui32* col_u = L_t.row(cov[u]);


		j = binary::find_next(cols, n(), 0);
		while (j < n()) {
			const ui32* col_j = L_t.row(j);
			
			for (ui32 ind = 0; ind < size_m(); ++ind) {
				buf[ind] = col_j[ind] | ~ (col_u[ind] & support_rows[ind]);
			}

			if (binary::all(buf, m()))
				binary::reset(cols, j);

			j = binary::find_next(cols, n(), j + 1);
		}
		++u;
	}

}

//ui32 ind = bit >> UI32_LOG2BIT;
//ui32 offset = bit & UI32_MASK;
//ui32 buf = (p[ind] >> offset) << offset;
//ui32 sz = size(bitsize);
//
//while (ind < sz) {
//	offset = _tzcnt_u32(buf);//UI32_BITS==32
//	if (offset == UI32_BITS) {
//		++ind;
//		buf = p[ind];
//	} else {
//		break;
//	}
//}
//return (ind << UI32_LOG2BIT) + offset;

void Dualizer_OPT::run() {
	Covering covering;
	covering.reserve(20);

	ui32* pool = static_cast<ui32*>(My_Memory::MM_malloc((3*size_m()+size_n())*UI32_SIZE));
	ui32* dst = pool;

	My_Memory::MM_memset(dst, ~0, (size_n() + size_m())*UI32_SIZE);
	ui32* rows         = dst; dst += size_m();
	ui32* cols         = dst; dst += size_n();

	My_Memory::MM_memset(dst, 0, 2 * size_m()*UI32_SIZE);
	ui32* support_rows = dst; dst += size_m();
	ui32* covered_rows = dst; dst += size_m();

	//My_Memory::MM_memset(rows, ~0, size_m()*UI32_SIZE);
	//My_Memory::MM_memset(cols, ~0, size_n()*UI32_SIZE);
	//My_Memory::MM_memset(support_rows, 0, size_m()*UI32_SIZE);
	//My_Memory::MM_memset(covered_rows, 0, size_m()*UI32_SIZE);
	
	Stack stack(3*size_m() + size_n(), 16);
	stack.push(pool, 0, 0);

	ui32 h = 0;
	ui32 j = 0;
	bool up_to_date = true;

	while (!stack.empty()) {		
		if (!up_to_date)
			stack.copy_top(pool, h, j);
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

		stack.update_j_next(j+1);
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

		stack.push(pool, j, 0);
		j = 0;
		up_to_date = true;
	}

	printf("Irreducible coverings: %d\n", n_coverings);

	My_Memory::MM_free(pool);
}

void Dualizer_OPT::init(const binary::Matrix& L0, const char* file_name, const char* mode) {
	if (file_name != nullptr) {
		p_file = fopen(file_name, mode);
		if (p_file == nullptr) {
			throw std::runtime_error(string("Dualizer_OPT::init::") + std::strerror(errno));
		}
	}
	L = L0;

	ui32* pool = static_cast<ui32*>(My_Memory::MM_malloc((size_m() + size_n())*UI32_SIZE));
	ui32* dst = pool;

	ui32* rows = dst; dst += size_m();
	ui32* cols = dst; dst += size_n();

	My_Memory::MM_memset(rows, ~0, (size_n() + size_m())*UI32_SIZE);

	delete_le_rows(rows, cols);
	L.submatrix(rows);
	L_t.transpose(L);

	My_Memory::MM_free(pool);
}

void Dualizer_OPT::clear() {
	if (p_file != nullptr)
		fclose(p_file);
}

