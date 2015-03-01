#include <assert.h>//for assert
#include <string> //for string(), +
#include <intrin.h>


#include "dualizer_OPT.h"
#include "my_memory.h"
#include "pool_stack.h"




using namespace std;

class Stack {
public:
	
	void push(const ui32* pool) {
		assert(pool != nullptr);
		pool_stack_.push_empty();
		My_Memory::MM_memcpy(pool_stack_.top(), pool, pool_stack_.element_size() * UI32_SIZE);
	}

	inline void update_j_next(ui32 j_next, ui32 offset) { 	
		*(pool_stack_.top() + offset) = j_next;
	}

	inline void pop() throw() { 
		pool_stack_.pop(); 
	}

	inline void copy_top(ui32* pool) throw() {
		assert(pool != nullptr);
		My_Memory::MM_memcpy(pool, pool_stack_.top(), pool_stack_.element_size() * UI32_SIZE);
	}

	inline bool empty() const throw() { return  pool_stack_.size() == 0; }

	inline int size() const throw() { return pool_stack_.size(); }

	Stack(ui32 pool_size, ui32 size = 16) : pool_stack_(pool_size) {
		pool_stack_.reserve(size);
	}

	~Stack() {}
private:

	Pool_Stack pool_stack_;
};

void Dualizer_OPT::update_covered_and_support_rows(ui32* rows, ui32* covered_rows,
	ui32* support_rows, ui32 j) const throw()
{
	const ui32* col_j = matrix_t_ + j * size_m();
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
		const ui32* col_j = matrix_t_ + j * size_m();

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
		const ui32* row1 = matrix_ + i1 * size_n();
		i2 = binary::find_next(rows, m(), 0);
		while (i2 < m()) {
			if (i2 != i1) {
				const ui32* row2 = matrix_ + i2 * size_n();

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
		const ui32* col_u = matrix_t_ + cov[u] * size_m();


		j = binary::find_next(cols, n(), 0);
		while (j < n()) {
			const ui32* col_j = matrix_t_ + j * size_m();
			
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

	ui32* const pool = static_cast<ui32*>(My_Memory::MM_malloc((3*size_m()+size_n()+1)*UI32_SIZE));
	ui32* dst = pool;

	My_Memory::MM_memset(dst, ~0, (size_n() + size_m())*UI32_SIZE);
	ui32* rows         = dst; dst += size_m();
	ui32* cols         = dst; dst += size_n();

	My_Memory::MM_memset(dst,  0, (2 * size_m() + 1)*UI32_SIZE);
	ui32* support_rows = dst; dst += size_m();
	ui32* covered_rows = dst; dst += size_m();
	ui32& j = *dst; dst += 1;
	
	Stack stack(3*size_m() + size_n() + 1, 16);
	stack.push(pool);

	bool up_to_date = true;

	while (!stack.empty()) {		
		if (!up_to_date)
			stack.copy_top(pool);
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

		binary::reset_le(cols, j);
		covering.append(j);
		update_covered_and_support_rows(rows, covered_rows, support_rows, j);

		++j;
		stack.update_j_next(j, &j - pool);

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

		stack.push(pool);
		up_to_date = true;
	}

	printf("Irreducible coverings: %d\n", n_coverings);

	My_Memory::MM_free(pool);
}

void Dualizer_OPT::init(const binary::Matrix& L, const char* file_name, const char* mode) {
	if (L.width() == 0 || L.height() == 0)
		throw std::runtime_error("Dualizer_OPT::init::empty matrix");

	if (file_name != nullptr) {
		p_file = fopen(file_name, mode);
		if (p_file == nullptr) {
			throw std::runtime_error(string("Dualizer_OPT::init::") + std::strerror(errno));
		}
	}

	matrix_ = const_cast<ui32*>(L.row(0));//it won't change!
	m_ = L.height();
	n_ = L.width();

	ui32* const pool = static_cast<ui32*>(My_Memory::MM_malloc((size_m() + size_n())*UI32_SIZE));
	ui32* dst = pool;

	My_Memory::MM_memset(dst, ~0, (size_n() + size_m())*UI32_SIZE);
	ui32* rows = dst; dst += size_m();
	ui32* cols = dst; dst += size_n();

	delete_le_rows(rows, cols);
	
	matrix_   = binary::submatrix(L.row(0), rows, m_, n_);
	matrix_t_ = binary::transpose(matrix_, m_, n_);

	My_Memory::MM_free(pool);
}

void Dualizer_OPT::clear() {
	if (p_file != nullptr) {
		fclose(p_file);
		p_file = nullptr;
	}		
	if (matrix_t_ != nullptr) {
		My_Memory::MM_free(matrix_);
		matrix_ = nullptr;
	}
	if (matrix_t_ != nullptr) {
		My_Memory::MM_free(matrix_t_);
		matrix_t_ = nullptr;
	}
	m_ = 0;
	n_ = 0;
	n_coverings = 0;
}

