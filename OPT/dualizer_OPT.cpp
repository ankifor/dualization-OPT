#include <assert.h>//for assert
#include <string> //for string(), +
#include <intrin.h>


#include "dualizer_OPT.h"
#include "my_memory.h"
#include "pool_stack.h"

//#define __ZF ((ui32) ((__readeflags() >> 6) & 1))


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

void Dualizer_OPT::update_covered_and_support_rows(ui32 j) throw() {
	const ui32* col_j = matrix_t_ + j * size_m();
	for (ui32 ind = 0; ind < size_m(); ++ind) {
		support_rows[ind] = (~rows[ind] ^ col_j[ind]) & (rows[ind] | support_rows[ind]);
		rows[ind] &= ~col_j[ind];
		covered_rows[ind] |= col_j[ind];
	}
}

void Dualizer_OPT::delete_zero_cols() throw() {
	ui32 j = binary::find_next(cols, n(), 0);

	while (j < n()) {
		const ui32* col_j = matrix_t_ + j * size_m();
		ui32 buf = 0;
		ui32 ind = 0;
		while (ind < size_m()) {
			buf |= rows[ind] & col_j[ind];
			++ind;
		}
		buf |= rows[ind] & col_j[ind] & mask_m();

		if (buf == 0)
			binary::reset(cols, j);
			

		j = binary::find_next(cols, n(), j + 1);
	}

}

//void Dualizer_OPT::delete_zero_cols() throw() {
//	ui32* buf = static_cast<ui32*>(alloca(size_n()*UI32_SIZE));
//	My_Memory::MM_memset(buf, 0, size_n()*UI32_SIZE);
//
//	ui32 i = binary::find_next(rows, m(), 0);
//	while (i < m()) {
//		const ui32* row_i = matrix_ + i * size_n();
//		for (ui32 ind = 0; ind < size_n(); ++ind) {
//			buf[ind] |= row_i[ind];
//		}
//		i = binary::find_next(rows, m(), i+1);
//	}
//
//	for (ui32 ind = 0; ind < size_n(); ++ind) {
//		cols[ind] &= buf[ind];
//	}
//
//}

void Dualizer_OPT::delete_le_rows() throw() {
	if (binary::popcount(cols, n()) == 0)
		return;

	ui32 i1 = binary::find_next(rows, m(), 0);
	ui32 i2 = 0;

	while (i1 < m()) {
		const ui32* row1 = matrix_ + i1 * size_n();
		i2 = binary::find_next(rows, m(), i1 + 1);
		while (i2 < m()) {
			
			const ui32* row2 = matrix_ + i2 * size_n();
			ui32 buf1 = 0;
			ui32 buf2 = 0;
			ui32 ind = 0;
			while (ind < size_n() - 1) {
				buf1 |=  row1[ind] & ~row2[ind];
				buf2 |= ~row1[ind] &  row2[ind];
				++ind;
			}
			buf1 |=  row1[ind] & ~row2[ind] & mask_n();
			buf2 |= ~row1[ind] &  row2[ind] & mask_n();

			//rows[i2 >> UI32_LOG2BIT] ^= binary::test_zero(buf) << (i2 & UI32_MASK);
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

	//ui32 n1 = binary::popcount(cols, n()) * covering.size();
	//
	//ui32 sz = binary::size64(m());
	//ui64 buf = reinterpret_cast<ui64*>(support_rows)[binary::size64(m()) - 1];
	//reinterpret_cast<ui64*>(support_rows)[sz - 1] &= binary::mask64(m());


	//ui32 n2 = 0;
	//for (ui32 u = 0; u < covering.size(); ++u) {
	//	const ui32* col_u = matrix_t_ + covering[u] * size_m();
	//	for (ui32 ind = 0; ind < sz; ++ind) {
	//		//n2 += __popcnt(support_rows[ind] & col_u[ind]);
	//		n2 += __popcnt64(
	//			reinterpret_cast<ui64*>(support_rows)[ind] &
	//			reinterpret_cast<const ui64*>(col_u)[ind]
	//			);
	//	}
	//}
	//
	//ui32 n3 = 0;
	//for (ui32 j = binary::find_next(cols, n(), 0); j < n(); j = binary::find_next(cols, n(), j + 1)) {
	//	const ui32* col_j = matrix_t_ + j * size_m();
	//	for (ui32 ind = 0; ind < sz; ++ind) {
	//		//n3 += __popcnt(support_rows[ind] & ~col_j[ind]);
	//		n3 += __popcnt64(
	//			reinterpret_cast<ui64*>(support_rows)[ind] &
	//			~reinterpret_cast<const ui64*>(col_j)[ind]
	//			);
	//	}
	//}

	//reinterpret_cast<ui64*>(support_rows)[sz - 1] = buf;

	//if (n1 < n2) {
	//	if (n1 < n3) {
	//		delete_fobidden_cols1();
	//	} else {
	//		delete_fobidden_cols3();
	//	}
	//} else {
	//	if (n2 < n3) {
	//		delete_fobidden_cols2();
	//	} else {
	//		delete_fobidden_cols3();
	//	}
	//}

	ui32 cols_count = binary::popcount(cols, n());
	ui32 cov_count = covering.size();

	if (cols_count < cov_count) {
		delete_fobidden_cols3();
	} else {
		delete_fobidden_cols2();
	}

}

void Dualizer_OPT::delete_fobidden_cols1() throw() {
	ui32 u = 0;
	ui32 ind = 0;

	const_cast<ui32*>(support_rows)[size_m() - 1] &= mask_m();

	while (u < covering.size()) {
		const ui32* col_u = matrix_t_ + covering[u] * size_m();

		ui32 j = binary::find_next(cols, n(), 0);
		while (j < n()) {
			tmp1_++;
			const ui32* col_j = matrix_t_ + j * size_m();

			ui32 buf = UI32_ALL;
			ind = 0;
			while (ind < size_m()) {
				buf &= col_j[ind] | ~(col_u[ind] & support_rows[ind]);
				++ind;
			}

			if (buf == UI32_ALL)
				binary::reset(cols, j);

			j = binary::find_next(cols, n(), j + 1);
		}
		++u;
	}

}

void Dualizer_OPT::delete_fobidden_cols2() throw() {
	ui32* buf = static_cast<ui32*>(alloca(size_n()*UI32_SIZE));
	ui32* ru = static_cast<ui32*>(alloca(size_m()*UI32_SIZE));

	ui32 u = 0;
	ui32 i = 0;

	while (u < covering.size()) {
		My_Memory::MM_memset(buf, ~0, size_n()*UI32_SIZE);
		const ui32* col_u = matrix_t_ + covering[u] * size_m();

		for (ui32 ind = 0; ind < size_m(); ++ind) {
			ru[ind] = support_rows[ind] & col_u[ind];
		}

		i = binary::find_next(ru, m(), 0);
		while (i < m()) {
			tmp2_++;
			const ui32* row_i = matrix_ + i * size_n();

			for (ui32 ind = 0; ind < size_n(); ++ind) {
				buf[ind] &= row_i[ind];
			}

			i = binary::find_next(ru, m(), i + 1);
		}

		for (ui32 ind = 0; ind < size_n(); ++ind) {
			cols[ind] &= ~buf[ind];
		}

		++u;
	}

}

void Dualizer_OPT::delete_fobidden_cols3() throw() {
	ui32* buf = static_cast<ui32*>(alloca(size_n()*UI32_SIZE));
	ui32* rj = static_cast<ui32*>(alloca(size_m()*UI32_SIZE));

	ui32 i = 0;
	ui32 j = 0;

	j = binary::find_next(cols, n(), 0);
	while (j < n()) {
		//buf = cov
		My_Memory::MM_memcpy(buf, cov, size_n()*UI32_SIZE);

		const ui32* col_j = matrix_t_ + j * size_m();

		for (ui32 ind = 0; ind < size_m(); ++ind) {
			rj[ind] = support_rows[ind] & ~col_j[ind];
		}

		i = binary::find_next(rj, m(), 0);
		while (i < m()) {
			const ui32* row_i = matrix_ + i*size_n();
			tmp3_++;
			for (ui32 ind = 0; ind < size_n(); ++ind) {
				buf[ind] &= ~row_i[ind];
			}			

			i = binary::find_next(rj, m(), i + 1);
		}

		//any
		ui32 buf1 = 0;
		buf[size_n() - 1] &= mask_n();
		for (ui32 ind = 0; ind < size_n(); ++ind) {
			buf1 |= buf[ind];
		}
		if (buf1 != 0)
			binary::reset(cols, j);

		j = binary::find_next(cols, n(), j + 1);
	}

}

void Dualizer_OPT::run() {
	
	covering.reserve(20);

	//current tree node may be described by 5 variables:
	//rows, cols, support_rows, covered_rows
	//they are stored in variable pool for efficiency
	ui32* const state = rows;

	My_Memory::MM_memset(rows, ~0, (size_n() + size_m())*UI32_SIZE);//rows and cols
	My_Memory::MM_memset(support_rows,  0, (2 * size_m() + 1)*UI32_SIZE);//support_rows and covered_rows
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
		*p_j = binary::find_next(cols, n(), *p_j);

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

		binary::reset_le(cols, *p_j);
		covering.append(*p_j);
		binary::set(cov, *p_j);

		update_covered_and_support_rows(*p_j);

		++*p_j;
		//modify last processed child number
		stack.update_j_next(*p_j, p_j - state);

		//leaf?
		if (!binary::any(rows, m())) {
			//false positive?
			if (binary::all(covered_rows, m())) {
				//irreducible covering (true positive)
				covering.print(p_file);
				++n_coverings;
			}
			//go up in the tree
			binary::reset(cov, covering.top());
			covering.remove_last();			
			up_to_date = false;
			continue;
		}

		//prepare child
		delete_fobidden_cols();
		delete_le_rows();
		delete_zero_cols();

		stack.push(state);
		up_to_date = true;
	}

	printf("Irreducible coverings: %d\ntmp1_: %d\ntmp2_: %d\ntmp3_: %d\n", n_coverings, tmp1_, tmp2_, tmp3_);
	//printf("Irreducible coverings: %d\n", n_coverings);
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
	size_m_ = binary::size(m_);
	size_n_ = binary::size(n_);

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
		binary::size(m_)     + //covered_rows
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
	size_m_ = binary::size(m_);

	matrix_t_ = dst; dst += n_* binary::size(m);
	binary::transpose(matrix_t_, matrix_, m_, n_);

	rows = dst; dst += size_m();
	cols = dst; dst += size_n();
	support_rows = dst; dst += size_m();
	covered_rows = dst; dst += size_m();
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

