#include <assert.h>//for assert
#include <string> //for string(), +
#include <cerrno>

#include "dualizer_OPT.h"
#include "my_memory.h"
#include "pool_stack.h"

#if defined(_MSC_VER)
#include <intrin.h>//for __popcnt64, _tzcnt_u64
#define popcnt32 __popcnt
#define tzcnt32 _tzcnt_u32
#elif defined(__IBMC__) || defined(__IBMCPP__)
#include <builtins.h>
#define popcnt32 __popcnt4
#define tzcnt32 __cnttz4
#endif

using namespace std;

//Dualizer_OPT::Covering

void Dualizer_OPT::Covering::reserve(ui32 size, ui32 width, bool reset_frequency) {
	data_.reserve(size);
	text_.reserve(size * 4);
	if (frequency_.size() != width) {
		frequency_.resize(width);
		reset_frequency = true;
	}
	if (reset_frequency)
		My_Memory::MM_memset(frequency_.get_data(), 0, width*UI32_SIZE);
}

Dualizer_OPT::Covering::~Covering() {}

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

void Dualizer_OPT::Covering::remove_last(ui32 num) {
	assert(data_.size() > 0 && num > 0);
	for (ui32 i = 0; i < num; ++i) {
		data_.pop();
		do {
			text_.pop();
		} while (text_.size() > 0 && text_.top() != ' ');
	}
}

ui32& Dualizer_OPT::Covering::top() { 
	return data_.top(); 
}

void Dualizer_OPT::Covering::print(FILE* p_file, bool extra) {
	if (p_file == nullptr) {
		++frequency_[data_[0]];
	} else {
		assert(text_.size() > 0);
		if (!extra) {
			text_.top() = '\n';
			//
			//fputs(text_.get_data(), p_file);
			fwrite(text_.get_data(), 1, text_.size(), p_file);
			//text_.pop();
			text_.top() = ' ';
		} else {
			text_.top() = 'x';
			text_.push('\n');
			fwrite(text_.get_data(), 1, text_.size(), p_file);
			text_.pop();
			text_.top() = ' ';
		}
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

Stack_Array<ui32>& Dualizer_OPT::Covering::get_freq() {
	return frequency_;
}

//Dualizer_OPT::Stack

void Dualizer_OPT::Stack::push() {
	assert(state_ != nullptr);
	pool_stack_.push_empty();
	My_Memory::MM_memcpy(pool_stack_.top(), state_, pool_stack_.element_size() * UI32_SIZE);
}

void Dualizer_OPT::Stack::update_j_next(ui32 j_next, const ui32* offset) {
	*(pool_stack_.top() + ui32(offset - state_)) = j_next;
}

void Dualizer_OPT::Stack::reset_cols(ui32 j, const ui32* offset) {
	binary::reset((pool_stack_.top() + ui32(offset - state_)), j);
}

void Dualizer_OPT::Stack::pop() throw() {
	pool_stack_.pop();
}

void Dualizer_OPT::Stack::copy_top() throw() {
	assert(state_ != nullptr);
	My_Memory::MM_memcpy(state_, pool_stack_.top(), pool_stack_.element_size() * UI32_SIZE);
}

bool Dualizer_OPT::Stack::empty() const throw() { 
	return  pool_stack_.size() == 0; 
}

int Dualizer_OPT::Stack::size() const throw() { 
	return pool_stack_.size(); 
}

Dualizer_OPT::Stack::Stack(): state_(nullptr) {}

void Dualizer_OPT::Stack::reserve(ui32 pool_size, ui32 size, ui32* state) {
	pool_stack_.set_element_size(pool_size);
	pool_stack_.reserve(size);
	state_ = state;
}

Dualizer_OPT::Stack::~Stack() {}

//Dualizer_OPT

void Dualizer_OPT::update_covered_and_support_rows(ui32 j) throw() {
	const ui32* col_j = matrix_t_ + j * size32_m();
	ui32 ind = 0;
	do {
		//support_rows[ind] = (~rows[ind] ^ col_j[ind]) & (rows[ind] | support_rows[ind]);
		support_rows[ind] ^= (rows[ind] ^ support_rows[ind]) & col_j[ind];
		rows[ind] &= ~col_j[ind];
		++ind;
	} while (ind < size32_m());
	support_rows[size32_m() - 1] &= mask32_m();
}

char Dualizer_OPT::process_zero_and_unity_cols() throw() {
	ui32* buf_zero = static_cast<ui32*>(alloca(size32_n() * UI32_SIZE));
	ui32* buf_unit = static_cast<ui32*>(alloca(size32_n()*UI32_SIZE));

	assert((cols[size32_n() - 1] & ~mask32_n()) == 0);

	My_Memory::MM_memset(buf_zero, 0, size32_n()*UI32_SIZE);
	My_Memory::MM_memcpy(buf_unit, cols, size32_n() * UI32_SIZE);

	ui32 i = binary::find_next(rows, m(), 0);
	assert(i < m());

	do {
		ui32 ind = 0;
		ui32 const* row_i = matrix_ + i * size32_n();
		do {
			buf_unit[ind] &= row_i[ind];
			buf_zero[ind] |= row_i[ind];
			++ind;
		} while (ind < size32_n());
		i = binary::find_next(rows, m(), i + 1);
	} while (i < m());
	
	bool any_children = false;
	ui32 j = binary::find_next(buf_unit, n(), 0);
	while (j < n()) {
		any_children = true;
		covering.append(j);
		covering.print(p_file);
		++n_coverings;
		covering.remove_last();
		binary::reset(cols, j);
		j = binary::find_next(buf_unit, n(), j + 1);
	}

	ui32 any_left = false;
	{
		ui32 ind = 0;
		do {
			cols[ind] &= buf_zero[ind];
			any_left |= cols[ind];
			++ind;
		} while (ind < size32_n());
	}
	return (any_left != 0) | (any_children << 1);
}

//void Dualizer_OPT::delete_zero_cols() throw() {
//	ui32* buf = static_cast<ui32*>(alloca(size32_n()*UI32_SIZE));
//	My_Memory::MM_memset(buf, 0, size32_n()*UI32_SIZE);
//
//	ui32 ind = 0;
//	ui32 i = binary::find_next(rows, m(), 0);
//	while (i < m()) {
//		const ui32* row_i = matrix_ + i * size32_n();
//		ind = 0;
//		do {
//			buf[ind] |= row_i[ind];
//			++ind;
//		} while (ind < size32_n());
//		i = binary::find_next(rows, m(), i+1);
//	}
//
//	for (ui32 ind = 0; ind < size32_n(); ++ind) {
//		cols[ind] &= buf[ind];
//	}
//
//}

void Dualizer_OPT::delete_le_rows() throw() {
	ui32 i1 = binary::find_next(rows, m(), 0);
	ui32 i2 = 0;
	ui32 size32_n_ = size32_n();
	//cols[size32_n_ - 1] &= mask32_n();
	assert((cols[size32_n() - 1] & ~mask32_n()) == 0);

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
			} while ( ind < size32_n_ );


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

//void Dualizer_OPT::process_unity_cols() throw() {
//	//ui32 rows_count = binary::popcount(rows, m());
//	//if (rows_count == 1) {
//	//	process_unity_cols1();
//	//} else if (rows_count > 1) {
//		process_unity_cols2();
//	//}
//}
//
//void Dualizer_OPT::process_unity_cols2() throw() {
//	ui32* buf = static_cast<ui32*>(alloca(size32_n() * UI32_SIZE));
//	My_Memory::MM_memcpy(buf, cols, size32_n() * UI32_SIZE);
//
//	ui32 i = binary::find_next(rows, m(), 0);
//	if (i == m()) {
//		return;
//	}
//	
//	do {
//		ui32 ind = 0;
//		ui32 const* row_i = matrix_ + i * size32_n();
//		do {
//			buf[ind] &= row_i[ind];
//			++ind;
//		} while (ind < size32_n());
//		i = binary::find_next(rows, m(), i+1);
//	} while (i < m());
//
//	ui32 j = binary::find_next(buf, n(), 0);
//	while (j < n()) {
//		covering.append(j);
//		covering.print(p_file);
//		++n_coverings;
//		covering.remove_last();
//		binary::reset(cols, j);
//		j = binary::find_next(buf, n(), j + 1);
//	}
//
//}

void Dualizer_OPT::delete_fobidden_cols() throw() {
	ui32 cols_count = binary::popcount(cols, n());
	ui32 support_count = binary::popcount(support_rows, m());
	ui32 cov_count = covering.size();
	ui32 n1 = cov_count * cols_count * 5 * m();
	ui32 n2 = cov_count * (2 * m() + 3 * n()) + support_count * 2 * n();
	//if (cols_count < cov_count) {
	//if (cols_count * m() > 2 * n()) {
	if (n2 < n1) {
		delete_fobidden_cols2(); 		
	} else if(n1 > 0) {
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

	assert((support_rows[size32_m() - 1] & ~mask32_m()) == 0);
	//support_rows[size32_m() - 1] &= mask32_m();

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

	cols[size32_n() - 1] &= mask32_n();
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

ui32 Dualizer_OPT::get_next_j() throw() {
	return binary::find_next(cols, n(), *p_j);

	//ui32 const* row_i = matrix_ + *p_i * size32_n();
	//
	//ui32 ind = *p_j >> UI32_LOG2BIT;
	//ui32 offset = *p_j & UI32_MASK;
	//
	//ui32 buf = ((row_i[ind] & cols[ind]) >> offset) << offset;
	//if (buf == 0) {
	//	do {
	//		++ind;
	//		buf = row_i[ind] & cols[ind];
	//	} while ((ind < size32_n()) & (buf == 0));
	//}
	//offset = tzcnt32(buf);
	//return (ind << UI32_LOG2BIT) + offset;
}

ui32 Dualizer_OPT::get_next_i(ui32& sum0) throw() {
	sum0 = binary::popcount(cols, n());
	return 0;
	//ui32 i_min = 0;
	//ui32 sum_min = n() + 1;
	//cols[size32_n() - 1] &= mask32_n();
	//
	//ui32 i = binary::find_next(rows, m(), 0);
	//while (i < m()) {
	//	ui32 const* row_i = matrix_ + i * size32_n();
	//	ui32 sum = 0;
	//	ui32 ind = 0;
	//	do {
	//		sum += popcnt32(row_i[ind] & cols[ind]);
	//		++ind;
	//	} while (ind < size32_n());
	//	if (sum < sum_min) {
	//		i_min = i;
	//		sum_min = sum;
	//	}
	//	i = binary::find_next(rows, m(), i + 1);
	//}
	//sum0 = sum_min;
	//return i_min;
}

void Dualizer_OPT::run(ui32 j) {
	//current tree node may be described by 5 variables:
	//rows, cols, support_rows, covered_rows
	//they are stored in variable pool for efficiency
	if (j != ui32(~0)) {
		binary::reset_le(cols, j);
		binary::set(cols, j);
	} else {
		ui32 sum = 0;
		*p_i = get_next_i(sum);
		if (sum == 0) {
			return;
		}
	}
	stack.push();
	//helps to avoid double copying while descent pushing
	char up_to_date = true;
	bool do_not_pop = false;
	
	//in-depth tree search
	while (!stack.empty()) {		

		if (!up_to_date)
			stack.copy_top();		

		bool parallel = (stack.size() == 1) & (j != ui32(~0));
		bool go_up = false;

		if (parallel) {
			*p_j = j;
			go_up = !binary::at(cols, j);
		} else if (!do_not_pop) {
			if (stack.size() > 1) {
				*p_j = get_next_j();
			} else {
				*p_j = binary::find_next(cols, n(), *p_j);
			}
			go_up = (*p_j >= n());
		} else {
			go_up = true;
		}
		//any children left?
		if (go_up) {
			//all children are finished, go up
			if (!do_not_pop) {
				stack.pop();
			}// else {
				//covering.print(p_file, true);
			//}
			do_not_pop = false;
			if (stack.size() > 0) {
				//binary::reset(cov, covering.top());
				covering.remove_last();				
			}
			up_to_date = false;
			continue;
		}
		++n_vertices;
		//reset j-th column
		stack.reset_cols(*p_j, cols);
		binary::reset(cols, *p_j);
		//append j to the covering
		covering.append(*p_j);
		//binary::set(cov, *p_j);
		//modify last processed child number	
		stack.update_j_next(*p_j + 1, p_j);
		//prepare child
		update_covered_and_support_rows(*p_j);
		delete_fobidden_cols();

		bool any = binary::any(cols, n());
		do_not_pop = !any;
		
		if (any) {
			//delete_le_rows();
			//delete_zero_cols();
		  //process_unity_cols();
			any = process_zero_and_unity_cols();
			do_not_pop = !any;
			//save current state
			if (!do_not_pop) {
				ui32 sum = 0;
				*p_i = get_next_i(sum);// binary::find_next(rows, m(), 0);
				//if (sum == 0) {
				//	do_not_pop = true;
				//} else {
					stack.push();
				//}				
			}
		}
		n_spare += do_not_pop;
		up_to_date = true;	
		*p_j = 0;
	}
	n_vertices += n_coverings;
}

bool Dualizer_OPT::go_down(ui32 sum) throw() {
	*p_j = 0;
	*p_j = get_next_j();
	assert(*p_j < n());
	binary::reset(cols, *p_j);
	++*p_depth;
	if (sum > 1)
		stack.push();
	return true;
}

void Dualizer_OPT::run1() {
	//this is a depth-first-search (DFS) without adding all children at once
	//no extra copying carried out when performing DFS
	//current tree node may be described by 7 variables:
	//covering, rows, cols, support_rows, covered_rows, i, j
	//where i is a row with minimal sum and j is a number of next column to add to covering
	//they are stored in variable pool for efficiency
	bool any_compliant = true;
	bool any_left = true;
	bool any_children = false;
	ui32 sum = 0;
	delete_le_rows();
	if (any_compliant) {
		char tmp = process_zero_and_unity_cols();
		any_left = tmp & 1;
		any_children = (tmp & 2) >> 1;
	}
	if (any_left)
		*p_i = get_next_i(sum);
	if (!any_left || sum == 0)
		return;

	bool go_down_ind = go_down(sum);
	

	while (true) {
		++n_vertices;
		ui32 j1 = get_next_j();
		if (j1 < n()) {
			stack.update_j_next(j1, p_j);
			stack.reset_cols(j1, cols);
		} else if (!go_down_ind) {
			stack.pop();
		}
		covering.append(*p_j);
		update_covered_and_support_rows(*p_j);
		delete_fobidden_cols();

		any_compliant = binary::any(cols, n());
		any_left = any_compliant;
		any_children = false;		

		if (any_compliant) {
			delete_le_rows();
			char tmp = process_zero_and_unity_cols();
			any_left = tmp & 1;
			any_children = (tmp & 2) >> 1;
		}

		sum = 1;
		if (any_left)
			*p_i = get_next_i(sum);

		if (!any_left || sum == 0) {
			//go up
			ui32 depth = *p_depth;
			if (stack.empty())
				break;
			stack.copy_top();
			covering.remove_last(depth - *p_depth + 1);
			n_spare += !any_compliant | !any_children;
			go_down_ind = false;
			continue;
		}
		go_down_ind = go_down(sum);		
	}

	n_vertices += n_coverings;
}




void Dualizer_OPT::init(
	const binary::Matrix& L, 
	const char* file_name, 
	const char* mode, 
	bool reset_frequency, 
	const binary::Matrix* L_to_check) 
{
	//do not process empty matrices
	if (L.width() == 0 || L.height() == 0) {
		throw std::runtime_error("Dualizer_OPT::init::empty matrix");
	}
	//open file if necessary
	if (file_name != nullptr) {
		if (p_file != nullptr) {
			fclose(p_file);
			p_file = nullptr;
		}
		p_file = fopen(file_name, mode);
		if (p_file == nullptr) {
			throw std::runtime_error(string("Dualizer_OPT::init::") + std::strerror(errno));
		}
	}
	ui32 m_new = L.height();
	ui32 m1_new = 0;
	ui32 n_new = L.width();
	ui32 pool_size =
		m_new * binary::size(n_new) + //matrix_
		binary::size(n_new) + //cols
		binary::size(m_new) + //rows				
		binary::size(m_new) + //support_rows
		1 + //p_j
		1 + //p_i
		1 + //p_depth
		n_new* binary::size(m_new); //matrix_t_
		//binary::size(n_new); //cov
	if (L_to_check != nullptr) {
		m1_new = L_to_check->height();
		if (L_to_check->width() != n_new)
			throw std::runtime_error("Dualizer_OPT::init::invalid L_to_check");
		pool_size += binary::size(m1_new) * (n_new + 1);//covered_rows_, matrix_to_cover_t_;
	}

	if (pool_size > pool_size_) {
		pool_size_ = pool_size;
		if (pool_ != nullptr) {
			My_Memory::MM_free(pool_);
			pool_ = nullptr;
		}
		pool_ = SC_32(My_Memory::MM_malloc(pool_size_ * UI32_SIZE));
	}
	n_coverings = 0;
	//prepare pool_: a place to store all data
	m_  = m_new;
	n_  = n_new;
	m1_ = m1_new;
	
	ui32* dst = pool_; //-V519	
	//matrix
	{
		matrix_ = dst; dst += m() * size32_n();
		My_Memory::MM_memcpy(matrix_, L.row(0), m() * size32_n() * UI32_SIZE);
	}
	//state variables
	{
		ui32* dst0 = dst;
		cols         = dst; dst += size32_n();
		rows         = dst; dst += size32_m();
		support_rows = dst; dst += size32_m();
		p_j          = dst; dst += 1;
		p_i          = dst; dst += 1;
		p_depth      = dst; dst += 1;
		stack.reserve(ui32(dst - dst0), 16, dst0);
	}
	//transposed matrix
	{
		matrix_t_ = dst; dst += n_* size32_m();
		binary::transpose(matrix_t_, matrix_, m_, n_);
	}	
	

	//covering
	{
		//cov
		//cov = dst; dst += size32_n();	
		covering.reserve(20, n(), reset_frequency);
	}	
	//matrix_to_cover_t_ and covered_rows_ 
	if (m1() > 0) {
		covered_rows_      = dst; dst += size32_m1();
		matrix_to_cover_t_ = dst; dst += size32_m1() * n();
		My_Memory::MM_memcpy(matrix_to_cover_t_, L_to_check->row(0), size32_m1() * n() * UI32_SIZE);
	}

	assert(ui32(dst - pool_) == pool_size_);
	reinit();
}

void Dualizer_OPT::reinit() {
	My_Memory::MM_memset(cols, ~0, size32_n() * UI32_SIZE);
	cols[size32_n() - 1] &= mask32_n();
	My_Memory::MM_memset(rows, ~0, size32_m() * UI32_SIZE);
	My_Memory::MM_memset(support_rows, 0, size32_m() * UI32_SIZE);
	support_rows[size32_m() - 1] &= mask32_m();
	*p_j = 0;
	*p_i = 0;
	*p_depth = 0;
	if (m1() > 0) {
		My_Memory::MM_memset(covered_rows_, 0, size32_m1() * UI32_SIZE);
	}
	//My_Memory::MM_memset(cov, 0, size32_n()*UI32_SIZE);
}

void Dualizer_OPT::clear() throw() {
	if (p_file != nullptr)
		fclose(p_file);
	if (pool_ != nullptr)
		My_Memory::MM_free(pool_);
	covering.~Covering();
	stack.~Stack();
	My_Memory::MM_memset(this, 0, sizeof(Dualizer_OPT));
}
