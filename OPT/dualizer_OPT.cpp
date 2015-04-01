#include <assert.h>//for assert
#include <string> //for string(), +
#include <intrin.h>
#include <cstdlib>//for rand
#include <ctime>//for time

#include "dualizer_OPT.h"
#include "my_memory.h"
#include "pool_stack.h"

//extern "C" {
//	inline int __fastcall find_next_asm(ui32* arr, ui32 bitsize, ui32 bit);
//	void __fastcall dfc2_internal(ui64* buf, ui64 const* cols, ui32 const* ru, ui32 const* mat, ui32 m, ui32 n);
//}

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

	inline void reset_cols(ui32 j,  ui32 offset) {
		binary::reset((pool_stack_.top() + offset), j);
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

void Dualizer_OPT::update_covered_and_support_rows(ui32 j) throw() {
	const ui32* col_j = matrix_t_ + j * size_m();
	for (ui32 ind = 0; ind < size_m(); ++ind) {
		support_rows[ind] = (~rows[ind] ^ col_j[ind]) & (rows[ind] | support_rows[ind]);
		rows[ind] &= ~col_j[ind];
		//covered_rows[ind] |= col_j[ind];
	}
}

//void Dualizer_OPT::delete_zero_cols() throw() {
//	ui32 j = binary::find_next(cols, n(), 0);
//
//	while (j < n()) {
//		const ui64* col_j = RE_64(matrix_t_ + j * size_m());
//		ui64 buf = 0;
//		ui32 ind = 0;
//		while (ind < size64_m()) {
//			buf |= RE_64(rows)[ind] & col_j[ind];
//			++ind;
//		}
//		buf |= RE_64(rows)[ind] & col_j[ind] & mask64_m();
//
//		if (buf == 0)
//			binary::reset(cols, j);
//			
//
//		j = binary::find_next(cols, n(), j + 1);
//	}
//
//}

void Dualizer_OPT::delete_zero_cols() throw() {
	ui32* buf = static_cast<ui32*>(alloca(size_n()*UI32_SIZE));
	My_Memory::MM_memset(buf, 0, size_n()*UI32_SIZE);

	ui32 i = binary::find_next(rows, m(), 0);
	while (i < m()) {
		const ui32* row_i = matrix_ + i * size_n();
		for (ui32 ind = 0; ind < size_n(); ++ind) {
			buf[ind] |= row_i[ind];
		}
		i = binary::find_next(rows, m(), i+1);
	}

	for (ui32 ind = 0; ind < size_n(); ++ind) {
		cols[ind] &= buf[ind];
	}

}

void Dualizer_OPT::delete_le_rows() throw() {
	ui32 i1 = binary::find_next(rows, m(), 0);
	ui32 i2 = 0;

	while (i1 < m()) {
		const ui64* row1 = RE_64(matrix_ + i1 * size_n());
		i2 = binary::find_next(rows, m(), i1 + 1);
		while (i2 < m()) {
			
			const ui64* row2 = RE_64(matrix_ + i2 * size_n());
			ui64 buf1 = 0;
			ui64 buf2 = 0;
			ui32 ind = 0;
			while (ind < size64_n() - 1) {
				buf1 |=  row1[ind] & ~row2[ind] & RE_64(cols)[ind];
				buf2 |= ~row1[ind] &  row2[ind] & RE_64(cols)[ind];
				++ind;
			}
			buf1 |=  row1[ind] & ~row2[ind] & RE_64(cols)[ind] & mask64_n();
			buf2 |= ~row1[ind] &  row2[ind] & RE_64(cols)[ind] & mask64_n();

			if (buf1 == 0) {
				binary::reset(rows, i2);
			} else if (buf2 == 0) {
				binary::reset(rows, i1);
				break;
			}
			
			i2 = binary::find_next(rows, m(), i2 + 1);
		}
		i1 = binary::find_next(rows, m(), i1 + 1);
	}

}

void Dualizer_OPT::delete_fobidden_cols() throw() {
	ui32 cols_count = binary::popcount(cols, n());
	ui32 cov_count = covering.size();

	if (cols_count < cov_count) {
		delete_fobidden_cols3();
	} else {
		delete_fobidden_cols2();
	}

}

//void Dualizer_OPT::delete_fobidden_cols1() throw() {
//	ui32 u = 0;
//	ui32 ind = 0;
//
//	const_cast<ui32*>(support_rows)[size_m() - 1] &= mask_m();
//
//	while (u < covering.size()) {
//		const ui32* col_u = matrix_t_ + covering[u] * size_m();
//
//		ui32 j = binary::find_next(cols, n(), 0);
//		while (j < n()) {
//			const ui32* col_j = matrix_t_ + j * size_m();
//
//			ui32 buf = UI32_ALL;
//			ind = 0;
//			while (ind < size_m()) {
//				buf &= col_j[ind] | ~(col_u[ind] & support_rows[ind]);
//				++ind;
//			}
//
//			if (buf == UI32_ALL)
//				binary::reset(cols, j);
//
//			j = binary::find_next(cols, n(), j + 1);
//		}
//		++u;
//	}
//
//}

void Dualizer_OPT::delete_fobidden_cols2() throw() {
	ui64* buf = static_cast<ui64*>(alloca(size64_n()*UI64_SIZE));
	ui32* ru  = static_cast<ui32*>(alloca(size_m()  *UI32_SIZE));

	ui32 u = 0;
	ui32 i1, i2 = 0;
	ui32 size64_n_ = size64_n();
	ui32 ind = 0;
	ui64 const*  row_i1 = nullptr;
	My_Memory::MM_memset(buf, ~0, size64_n()*UI64_SIZE);

	while (u < covering.size()) {
		
		ui32 const* col_u = matrix_t_ + covering[u] * size_m();
		ind = 0;
		do {
			ru[ind] = support_rows[ind] & col_u[ind];			
			++ind;
		} while (ind < size_m());
		
		
		i1 = binary::find_next(ru, m(), 0);
		while (i1 < m()) {					
			row_i1 = RE_64(matrix_ + i1 * size_n());//how to avoid multiplication?			
			ind = 0;
			do {
				buf[ind] &= row_i1[ind];
				++ind;
			} while (ind < size64_n_);		
			i1 = binary::find_next(ru, m(), i1 + 1);
		} 

		ind = 0;
		do {
			cols[ind] &= ~RE_32(buf)[ind];
			RE_32(buf)[ind] = ~0;
			++ind;
		} while (ind < size_n());

		++u;
	}

}

void Dualizer_OPT::delete_fobidden_cols3() throw() {
	ui64* buf = static_cast<ui64*>(alloca(size64_n()*UI64_SIZE));
	ui32* rj  = static_cast<ui32*>(alloca(size_m()  *UI32_SIZE));

	ui32 i = 0;
	ui64 const* row_i = nullptr;
	ui32 j = 0;
	ui32 ind = 0;
	ui32 size64_n_ = size64_n();
	
	RE_32(cov)[size_n() - 1] &= mask_n();
	My_Memory::MM_memcpy(buf, cov, size_n()*UI32_SIZE);

	j = binary::find_next(cols, n(), 0);
	while (j < n()) {
		const ui32* col_j = matrix_t_ + j * size_m();

		for (ui32 ind = 0; ind < size_m(); ++ind) {
			rj[ind] = support_rows[ind] & ~col_j[ind];
		}

		i = binary::find_next(rj, m(), 0);		
		while (i < m()) {
			row_i = RE_64(matrix_ + i*size_n());
			ind = 0;
			do {
				buf[ind] &= ~row_i[ind];
				++ind;
			} while (ind < size64_n_);
			i = binary::find_next(rj, m(), i+1);
		}

		//any
		ui64 buf1 = 0;	
		ind = 0;
		do {
			buf1 |= RE_32(buf)[ind];
			RE_32(buf)[ind] = cov[ind];
			++ind;
		} while (ind < size_n());

		if (buf1 != 0) {
			binary::reset(cols, j);
		}

		j = binary::find_next(cols, n(), j + 1);
	}

}

//char Dualizer_OPT::create_search_set(ui32* set) throw() {
//	ui32* unobserved = static_cast<ui32*>(alloca(size_m()*UI32_SIZE));
//	ui32 buf = 0;
//	ui32 ind = 0;
//	for (ind = 0; ind < size_m() - 1; ++ind) {
//		unobserved[ind] = ~(covered_rows[ind] | rows[ind]);
//		buf |= unobserved[ind];
//	}
//	unobserved[ind] = ~(covered_rows[ind] | rows[ind]) & mask_m();
//	buf |= unobserved[ind];
//	
//	if (buf == 0) {
//		return 0;
//	}
//
//	My_Memory::MM_memset(set, 0, size_n()*UI32_SIZE);
//	cols[size_n() - 1] &= mask_n();
//	ui32 i = binary::find_next(unobserved, m(), 0);
//	
//	while (i < m()) {
//		ui32* row_i = RE_32(matrix_ + i * size_n());
//		for (ind = 0; ind < size_m(); ++ind) {
//			set[ind] |= row_i[ind];
//		}
//		i = binary::find_next(unobserved, m(), i+1);
//	}
//	for (ind = 0; ind < size_m(); ++ind) {
//		set[ind] &= cols[ind];
//	}
//	return 1;
//
//}


void Dualizer_OPT::run() {
	
	covering.reserve(20, n());

	//current tree node may be described by 5 variables:
	//rows, cols, support_rows, covered_rows
	//they are stored in variable pool for efficiency
	ui32* const state = rows;

	My_Memory::MM_memset(rows, ~0, (size_n() + size_m())*UI32_SIZE);//rows and cols
	My_Memory::MM_memset(support_rows,  0, (size_m() + 1)*UI32_SIZE);//support_rows and covered_rows
	My_Memory::MM_memset(cov, 0, size_n()*UI32_SIZE);//cov
	
	//stack stores the whole tree node
	Stack stack(3*size_m() + size_n() + 1, 16);
	stack.push(state);

	//helps to avoid double copying
	bool up_to_date = true;
	//in-depth tree search
	while (!stack.empty()) {		
		if (!up_to_date)
			stack.copy_top(state);			
		
		*p_j = binary::find_next(cols, n(), 0);

		//any children left?
		if (*p_j >= n()) {
			//all children are finished, go up
			stack.pop();
			if (stack.size() > 0) {
				binary::reset(cov, covering.top());
				covering.remove_last();				
			}
			up_to_date = false;
			continue;
		}

		//binary::reset_le(cols, *p_j);
		stack.reset_cols(*p_j, cols - state);
		binary::reset(cols, *p_j);

		covering.append(*p_j);
		binary::set(cov, *p_j);

		update_covered_and_support_rows(*p_j);

		++*p_j;
		//modify last processed child number
		stack.update_j_next(*p_j, p_j - state);

		//leaf?
		if (!binary::any(rows, m())) {
			covering.print(p_file);
			++n_coverings;
			//go up in the tree
			binary::reset(cov, covering.top());
			covering.remove_last();			
			up_to_date = false;
			continue;
		}

		//prepare child
		delete_fobidden_cols();
		if (binary::any(cols, n())) {
			delete_le_rows();
			delete_zero_cols();
		}
		stack.push(state);
		up_to_date = true;
		
	}
	print();
}

void Dualizer_OPT::init(const binary::Matrix& L, const char* file_name, const char* mode) {
	srand(time(0));
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

	ui32* tmp = static_cast<ui32*>(My_Memory::MM_malloc((size_m() + size_n())*UI32_SIZE));
	ui32* dst = tmp;
	My_Memory::MM_memset(dst, ~0, (size_n() + size_m())*UI32_SIZE);
	rows = dst; dst += size_m();
	cols = dst; dst += size_n();

	delete_le_rows();
	
	ui32 m = binary::popcount(rows, m_);
	ui32 pool_size =
		m * binary::size(n_) + //matrix_
		n_* binary::size(m ) + //matrix_t_
		binary::size(m_)     + //rows
		binary::size(n_)     + //cols
		binary::size(m_)     + //support_rows
		//binary::size(m_)     + //covered_rows
		1                    + //p_j
		binary::size(n_);      //cov
	
	if (pool_ != nullptr) {
		My_Memory::MM_free(tmp);
		throw runtime_error("Dualizer_OPT::init::pool_ not null");
	}

	pool_ = static_cast<ui32*>(My_Memory::MM_malloc(pool_size*UI32_SIZE));
	dst = pool_; //-V519
	matrix_ = dst; dst += m * binary::size(n_);

	binary::submatrix(matrix_, L.row(0), rows, m_, n_);
	m_ = m;

	matrix_t_ = dst; dst += n_* binary::size(m);
	binary::transpose(matrix_t_, matrix_, m_, n_);

	rows = dst; dst += size_m();
	cols = dst; dst += size_n();
	support_rows = dst; dst += size_m();
	//covered_rows = dst; dst += size_m();
	p_j = dst; dst += 1;
	cov = dst; dst += size_n();

	My_Memory::MM_free(tmp);

}

void Dualizer_OPT::clear() {
	if (p_file != nullptr) {
		fclose(p_file);
	}		
	if (pool_ != nullptr) {
		My_Memory::MM_free(pool_);
	}
	covering.~Covering();
	My_Memory::MM_memset(this, 0, sizeof(Dualizer_OPT));
}

