#include <assert.h>//for assert
#include <string> //for string(), +
#include <intrin.h>
//#include <cstdlib>//for rand
//#include <ctime>//for time

#include "dualizer_OPT.h"
#include "my_memory.h"
#include "pool_stack.h"

//extern "C" {
//	inline int __fastcall find_next_asm(ui32* arr, ui32 bitsize, ui32 bit);
//	void __fastcall dfc2_internal(ui64* buf, ui64 const* cols, ui32 const* ru, ui32 const* mat, ui32 m, ui32 n);
//}

//static void divu10(ui32& q, ui32& r) {
//	ui32 n = q;
//	q = (q >> 1) + (q >> 2);
//	q = q + (q >> 4);
//	q = q + (q >> 8);
//	q = q + (q >> 16);
//	q = q >> 3;
//	r = n - q * 10;
//	q = q + (r > 9); //-V602
//	r = n - q * 10;
//}

using namespace std;

void Dualizer_OPT::Covering::reserve(ui32 size, ui32 width) {
	data_.reserve(size);
	text_.reserve(size * 4);
	frequency_ = (ui32*) My_Memory::MM_malloc(width*UI32_SIZE);
	My_Memory::MM_memset(frequency_, 0, width*UI32_SIZE);
}

Dualizer_OPT::Covering::~Covering() {
	if (frequency_ != nullptr) {
		My_Memory::MM_free(frequency_);
	}
}

void Dualizer_OPT::Covering::append(ui32 q) {
	assert(q <= 99999);

	data_.push(q);

	char buf[5];
	ui32 len = 0;
	ui32 r = 0;

	do {
		//divu10(q, r);
		buf[len] = '0' + q % 10 ;
		q = q / 10;
		++len;
	} while (q > 0);

	do {
		--len;
		text_.push(buf[len]);
	} while (len > 0);

	text_.push(' ');
}

void Dualizer_OPT::Covering::remove_last() {
	assert(data_.size() > 0);
	data_.pop();
	do {
		text_.pop();
	} while (text_.size() > 0 && text_.top() != ' ');
}

ui32& Dualizer_OPT::Covering::top() { 
	return data_.top(); 
}

void Dualizer_OPT::Covering::print(FILE* p_file) {
	if (p_file == nullptr) {
		++frequency_[data_[0]];
	} else {
		assert(text_.size() > 0);
		text_.top() = '\n';
		//text_.push('\0');
		//fputs(text_.get_data(), p_file);
		fwrite(text_.get_data(), 1, text_.size(), p_file);
		//text_.pop();
		text_.top() = ' ';
	}
}

ui32 Dualizer_OPT::Covering::operator[] (ui32 ind) const throw() { 
	return data_[ind]; 
}

ui32 Dualizer_OPT::Covering::size() const throw() { 
	return data_.size(); 
}

void Dualizer_OPT::Covering::print_freq(ui32 width) {
	for (ui32 i = 0; i < width; ++i) {
		printf("%d ", frequency_[i]);
	}
	printf("\n");
}

ui32* Dualizer_OPT::Covering::get_freq() {
	return frequency_;
}

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
	const ui32* col_j = matrix_t_ + j * size32_m();
	ui32 ind = 0;
	do {
		//support_rows[ind] = (~rows[ind] ^ col_j[ind]) & (rows[ind] | support_rows[ind]);
		support_rows[ind] ^= (rows[ind] ^ support_rows[ind]) & col_j[ind];
		rows[ind] &= ~col_j[ind];
		++ind;
	} while (ind < size32_m());
}

//void Dualizer_OPT::delete_zero_cols() throw() {
//	ui32 n1 = binary::popcount(cols, n()) * m();
//	ui32 n2 = binary::popcount(rows, m()) * n();
//	
//	if (n1 < n2) {
//		delete_zero_cols1();
//	} else {
//		delete_zero_cols2();
//	}
//
//}

//void Dualizer_OPT::delete_zero_cols1() throw() {
//	rows[size32_m() - 1] &= mask32_m();
//	ui32 j = binary::find_next(cols, n(), 0);
//
//	while (j < n()) {
//		ui32 const* col_j = matrix_t_ + j * size32_m();
//		ui32 buf = 0;
//		ui32 ind = 0;
//		do {
//			buf |= rows[ind] & col_j[ind];
//			++ind;
//		} while (ind < size32_m());
//
//		if (buf == 0)
//			binary::reset(cols, j);			
//
//		j = binary::find_next(cols, n(), j + 1);
//	}
//
//}

void Dualizer_OPT::delete_zero_cols() throw() {
	ui32* buf = static_cast<ui32*>(alloca(size32_n()*UI32_SIZE));
	My_Memory::MM_memset(buf, 0, size32_n()*UI32_SIZE);

	ui32 ind = 0;
	ui32 i = binary::find_next(rows, m(), 0);
	while (i < m()) {
		const ui32* row_i = matrix_ + i * size32_n();
		ind = 0;
		do {
			buf[ind] |= row_i[ind];
			++ind;
		} while (ind < size32_n());
		i = binary::find_next(rows, m(), i+1);
	}

	for (ui32 ind = 0; ind < size32_n(); ++ind) {
		cols[ind] &= buf[ind];
	}

}

void Dualizer_OPT::delete_le_rows() throw() {
	ui32 i1 = binary::find_next(rows, m(), 0);
	ui32 i2 = 0;
	ui32 size32_n_ = size32_n();
	cols[size32_n_ - 1] &= mask32_n();

	while (i1 < m()) {
		const ui32* row1 = matrix_ + i1 * size32_n_;
		i2 = binary::find_next(rows, m(), i1 + 1);
		while (i2 < m()) {
			
			const ui32* row2 = matrix_ + i2 * size32_n_;
			ui32 buf1 = 0;
			ui32 buf2 = 0;
			ui32 ind = 0;
			do {
				buf1 |=  row1[ind] & ~row2[ind] & cols[ind];
				buf2 |= ~row1[ind] &  row2[ind] & cols[ind];
				++ind;
			} while (ind < size32_n_);

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
	//if (cols_count < cov_count) {
	if (cols_count * m() > 2 * n()) {
		delete_fobidden_cols2(); 		
	} else if(cols_count > 0) {
		delete_fobidden_cols1();
	}
}

void Dualizer_OPT::delete_fobidden_cols1() throw() {
	//this function is efficient, when popcount(cols) is low
	ui32 u = 0;
	ui32 j = 0;
	ui32 ind = 0;
	ui32 buf = 0;
	ui32 const* col_u = nullptr;
	ui32 const* col_j = nullptr;
	support_rows[size32_m() - 1] &= mask32_m();

	j = binary::find_next(cols, n(), 0);
	do {
		col_j = matrix_t_ + j * size32_m();		
		u = 0;
		while (u < covering.size()) {
			col_u = matrix_t_ + covering[u] * size32_m();
			buf = 0;
			ind = 0;
			do {
				buf |= ~col_j[ind] & support_rows[ind] & col_u[ind];
				++ind;
			} while (ind < size32_m());

			if (buf == 0) {
				binary::reset(cols, j);
				break;
			}
			++u;
		}
		j = binary::find_next(cols, n(), j+1);
	} while (j < n());

}

void Dualizer_OPT::delete_fobidden_cols2() throw() {
	ui32* buf = static_cast<ui32*>(alloca(size32_n() * UI32_SIZE));
	ui32* ru  = static_cast<ui32*>(alloca(size32_m() * UI32_SIZE));

	ui32 u = 0;//for covering
	ui32 i = 0;//for ru
	ui32 size32_n_ = size32_n();
	ui32 ind = 0;//for vector indexing
	ui32 const* row_i      = nullptr;
	ui32 const* col_u      = nullptr;
	My_Memory::MM_memset(buf, ~0, size32_n_ * UI32_SIZE);

	while (u < covering.size()) {
		
		col_u = matrix_t_ + covering[u] * size32_m();
		ind = 0;
		do {
			ru[ind] = support_rows[ind] & col_u[ind];			
			++ind;
		} while (ind < size32_m());

		i = binary::find_next(ru, m(), 0);
		
		while (i < m()) {		
			row_i = matrix_ + i * size32_n();
			ind = 0;			
			do {
				buf[ind] &= row_i[ind];
				++ind;
			} while (ind < size32_n_);		

			i = binary::find_next(ru, m(), i+1);
		} 

		ind = 0;
		do {
			cols[ind] &= ~buf[ind];
			buf[ind] = ui32(~0);
			++ind;
		} while (ind < size32_n_);

		++u;
	}

}

//void Dualizer_OPT::delete_fobidden_cols3() throw() {
//	//popcount(cols) == 1
//	ui32* buf = static_cast<ui32*>(alloca(size32_n()*UI32_SIZE));
//	ui32* rj  = static_cast<ui32*>(alloca(size32_m()  *UI32_SIZE));
//
//	ui32 i = 0;
//	ui32 const* row_i = nullptr;
//	ui32 j = 0;
//	ui32 ind = 0;
//	ui32 size32_n_ = size32_n();
//	
//	cov[size32_n() - 1] &= mask32_n();
//	My_Memory::MM_memcpy(buf, cov, size32_n()*UI32_SIZE);
//
//	j = binary::find_next(cols, n(), 0);
//	const ui32* col_j = matrix_t_ + j * size32_m();
//
//	for (ui32 ind = 0; ind < size32_m(); ++ind) {
//		rj[ind] = support_rows[ind] & ~col_j[ind];
//	}
//
//	i = binary::find_next(rj, m(), 0);		
//	while (i < m()) {
//		row_i = RE_32(matrix_ + i*size32_n());
//		ind = 0;
//		do {
//			buf[ind] &= ~row_i[ind];
//			++ind;
//		} while (ind < size32_n_);
//		i = binary::find_next(rj, m(), i+1);
//	}
//
//	//any
//	ui32 buf1 = 0;	
//	ind = 0;
//	do {
//		buf1 |= buf[ind];
//		++ind;
//	} while (ind < size32_n());
//
//	if (buf1 != 0) {
//		binary::reset(cols, j);
//	}
//}

//char Dualizer_OPT::create_search_set(ui32* set) throw() {
//	ui32* unobserved = static_cast<ui32*>(alloca(size32_m()*UI32_SIZE));
//	ui32 buf = 0;
//	ui32 ind = 0;
//	for (ind = 0; ind < size32_m() - 1; ++ind) {
//		unobserved[ind] = ~(covered_rows[ind] | rows[ind]);
//		buf |= unobserved[ind];
//	}
//	unobserved[ind] = ~(covered_rows[ind] | rows[ind]) & mask32_m();
//	buf |= unobserved[ind];
//	
//	if (buf == 0) {
//		return 0;
//	}
//
//	My_Memory::MM_memset(set, 0, size32_n()*UI32_SIZE);
//	cols[size32_n() - 1] &= mask32_n();
//	ui32 i = binary::find_next(unobserved, m(), 0);
//	
//	while (i < m()) {
//		ui32* row_i = RE_32(matrix_ + i * size32_n());
//		for (ind = 0; ind < size32_m(); ++ind) {
//			set[ind] |= row_i[ind];
//		}
//		i = binary::find_next(unobserved, m(), i+1);
//	}
//	for (ind = 0; ind < size32_m(); ++ind) {
//		set[ind] &= cols[ind];
//	}
//	return 1;
//
//}

//void Dualizer_OPT::delete_fobidden_cols4() throw() {
//	ui32 size_cov = covering.size();
//	ui32* support_i = static_cast<ui32*>(alloca(size32_n() * UI32_SIZE));
//	ui32* buf = static_cast<ui32*>(alloca(size_cov * size32_n() * UI32_SIZE));
//	My_Memory::MM_memset(buf, ~0, size_cov * size32_n() * UI32_SIZE);
//
//	ui32 ind = 0;
//	ui32 u = 0;
//	ui32 num = 0;
//	ui32 i = binary::find_next(support_rows, m(), 0);
//	ui32* buf_u = nullptr;
//	while (i < m()) {
//		ui32 const* row_i = matrix_ + i * size32_n();
//		ind = 0;
//		do {
//			support_i[ind] = row_i[ind] & cov[ind];
//			++ind;
//		} while (ind < size32_n());
//		//find column for which i-th row is support-row
//		num = binary::find_next(support_i, n(), 0);
//		//restore number of that column in covering
//		buf_u = buf;
//		u = 0;
//		while (covering[u] != num) {
//			++u;
//			buf_u += size32_n();
//		}
//
//		ind = 0;
//		do {
//			buf_u[ind] &= row_i[ind];
//			++ind;
//		} while (ind < size32_n());
//		
//		i = binary::find_next(support_rows, m(), i + 1);
//	}
//	//update cols
//	u = 0;
//	buf_u = buf;
//	do {		
//		ind = 0;
//		do {
//			cols[ind] &= ~buf_u[ind];
//			++ind;
//		} while (ind < size32_n());
//		buf_u += size32_n();
//		++u;
//	} while (u < covering.size());
//
//}

void Dualizer_OPT::run() {
	covering.reserve(20, n());
	//current tree node may be described by 5 variables:
	//rows, cols, support_rows, covered_rows
	//they are stored in variable pool for efficiency
	ui32* const state = cols;//cols, rows, support_rows, p_j	
	//stack stores the whole tree node
	Stack stack(2*size32_m() + size32_n() + 1, 16);
	stack.push(state);
	//helps to avoid double copying while descent pushing
	char up_to_date = true;
	bool do_not_pop = false;
	//in-depth tree search
	while (!stack.empty()) {		
		if (!up_to_date)
			stack.copy_top(state);
		*p_j = binary::find_next(cols, n(), 0);
		//any children left?
		if (*p_j >= n()) {
			//all children are finished, go up
			if (!do_not_pop) {
				stack.pop();
			} else {
				do_not_pop = false;
			}
			if (stack.size() > 0) {
				binary::reset(cov, covering.top());
				covering.remove_last();				
			}
			up_to_date = false;
			continue;
		}
		//reset j-th column
		stack.reset_cols(*p_j, cols - state);
		binary::reset(cols, *p_j);
		//append j to the covering
		covering.append(*p_j);
		binary::set(cov, *p_j);
		//update state
		update_covered_and_support_rows(*p_j);
		//modify last processed child number
		++*p_j;		
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
			//save current state
			stack.push(state);
		} else {
			do_not_pop = true;
		}
		
		
		up_to_date = true;		
	}
	print();
}

void Dualizer_OPT::init(const binary::Matrix& L, const char* file_name, const char* mode) {
	ui32* dst = nullptr;
	//different checks
	if (L.width() == 0 || L.height() == 0)
		throw std::runtime_error("Dualizer_OPT::init::empty matrix");

	if (pool_ != nullptr)
		throw std::runtime_error("Dualizer_OPT::init::pool_ not null");

	if (file_name != nullptr) {
		p_file = fopen(file_name, mode);
		if (p_file == nullptr)
			throw std::runtime_error(string("Dualizer_OPT::init::") + std::strerror(errno));
		//file_buffer_ = static_cast<char*>(My_Memory::MM_malloc(1024));
		//setbuf(p_file, file_buffer_);
	}
	//prepare pool_: a place to store all data
	m_ = L.height();
	n_ = L.width();	
	ui32 pool_size  =
		m_ * size32_n() + //matrix_
		size32_n()      + //cols
		size32_m()      + //rows				
		size32_m()      + //support_rows
		1               + //p_j
		n_* size32_m()  + //matrix_t_
		size32_n()      ; //cov
	pool_ = SC_32(My_Memory::MM_malloc(pool_size * UI32_SIZE));
	////prepare data for delete_le_rows
	//dst = pool_ + m_ * size32_n();
	//My_Memory::MM_memset(dst, ~0, (size32_n() + size32_m()) * UI32_SIZE);
	//cols = dst; dst += size32_n();
	//rows = dst; dst += size32_m();	
	//matrix_ = const_cast<ui32*>(L.row(0));//data is not changed
	//delete_le_rows
	//delete_le_rows();		
	//prepare data for run()
	dst = pool_; //-V519
	//matrix_
	//binary::submatrix(dst, L.row(0), rows, m_, n_);
	My_Memory::MM_memcpy(dst, L.row(0), m() * size32_n() * UI32_SIZE);
	//m_ = binary::popcount(rows, m_);
	matrix_ = dst; dst += m() * size32_n(); 	
	//cols and rows
	My_Memory::MM_memset(dst, ~0, (size32_n() + size32_m()) * UI32_SIZE);
	cols = dst; dst += size32_n();
	rows = dst; dst += size32_m();	
	//support_rows and p_j
	My_Memory::MM_memset(dst, 0, (size32_m() + 1) * UI32_SIZE);
	support_rows = dst; dst += size32_m();
	p_j          = dst; dst += 1;	
	//matrix_t_
	binary::transpose(dst, matrix_, m_, n_);
	matrix_t_ = dst; dst += n_* size32_m();	
	//cov
	My_Memory::MM_memset(dst, 0, size32_n()*UI32_SIZE);
	cov = dst; dst += size32_n();	
}

void Dualizer_OPT::clear() {
	if (p_file != nullptr)
		fclose(p_file);
	if (pool_ != nullptr)
		My_Memory::MM_free(pool_);
	if (file_buffer_ != nullptr)
		My_Memory::MM_free(file_buffer_);
	covering.~Covering();
	My_Memory::MM_memset(this, 0, sizeof(Dualizer_OPT));
}

