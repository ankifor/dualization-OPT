#pragma once
#include <cstdio>
#include "my_int.h"
#include "stack_array.h"
#include "pool_stack.h"
#include "binary.h"

class Dualizer_OPT {
protected:

	class Covering {
	public:
		void reserve(ui32 size, ui32 width, bool reset_frequency = true);
		~Covering();
		void append(ui32 num);
		void remove_last(ui32 num = 1);
		ui32& top();
		void print(FILE* p_file, bool extra = false);
		ui32 operator[] (ui32 ind) const throw();		
		ui32 size() const throw();
		void print_freq(ui32 width);
		Stack_Array<ui32>& get_freq();
	private:
		Stack_Array<ui32> data_;
		Stack_Array<char> text_;
		Stack_Array<ui32> frequency_;
	};

	class Stack {
	public:
		void push();
		void update_j_next(ui32 j_next, const ui32* offset);
		void reset_cols(ui32 j, const ui32* offset);
		void pop() throw();
		void copy_top() throw();
		bool empty() const throw();
		int size() const throw();
		void reserve(ui32 pool_size, ui32 size, ui32* state);
		Stack();
		~Stack();
	private:
		Pool_Stack pool_stack_;
		ui32* state_;
	};

public:

	Dualizer_OPT() { My_Memory::MM_memset(this, 0, sizeof(Dualizer_OPT)); }
	~Dualizer_OPT() { clear(); }

	//preprocesses matrix, allocates memory, set matrix_, matrix_t_, m_, n_, etc
	void init(
		const binary::Matrix& L, 
		const char* file_name = nullptr, 
		const char* mode = "wb", 
		bool reset_frequency = true,
		const binary::Matrix* L_to_check = nullptr
		);
	void check_covering(const binary::Matrix& L_to_check);
	void reinit();
	void clear() throw();
	void run(ui32 j = ui32(~0));
	void run1();
	void print() {
		printf("%u %u %u\n", n_coverings, n_vertices, n_spare);
		if (p_file == nullptr)
			covering.print_freq(n());
	}
	Stack_Array<ui32>& get_freq() { return covering.get_freq(); }
	ui32 get_num() { return n_coverings;  }

protected:
	ui32 get_next_j() throw();
	ui32 get_next_i(ui32& sum) throw();

	void update_covered_and_support_rows(ui32 j) throw();
	char process_zero_and_unity_cols() throw();//returns (any_left != 0) | (any_children << 1)
	
	void delete_fobidden_cols()  throw();
	void delete_fobidden_cols1() throw();
	void delete_fobidden_cols2() throw();
	void delete_fobidden_cols3() throw();
	void delete_fobidden_cols4() throw();
	
	void delete_le_rows() throw();

	bool go_down(ui32 sum) throw();

private:

	Dualizer_OPT(Dualizer_OPT const&) {};
	void operator = (Dualizer_OPT const&) {};

	inline ui32 m() const throw() { return m_; }
	inline ui32 size32_m() const throw() { return binary::size(m_); }
	inline ui32 size64_m() const throw() { return binary::size64(m_); }
	inline ui32 mask32_m() const throw() { return binary::mask(m_); }
	inline ui64 mask64_m() const throw() { return binary::mask64(m_); }

	inline ui32 n() const throw() { return n_; }
	inline ui32 size32_n() const throw() { return binary::size(n_); }
	inline ui32 size64_n() const throw() { return binary::size64(n_); }
	inline ui32 mask32_n() const throw() { return binary::mask(n_); }
	inline ui64 mask64_n() const throw() { return binary::mask64(n_); }

	inline ui32 m1() const throw() { return m1_; }
	inline ui32 size32_m1() const throw() { return binary::size(m1_); }
private:
	ui32* pool_;
	Stack stack;

	ui32* matrix_;
	ui32* cols;
	ui32* rows;
	ui32* support_rows;
	ui32* p_j;
	ui32* p_i;
	ui32*p_depth;
	ui32* matrix_t_;
	ui32* covered_rows_;
	ui32* matrix_to_cover_t_;
	//ui32* cov;	
	Covering covering;


	ui32 m_;
	ui32 n_;
	ui32 m1_;
	ui32 n_coverings;
	ui32 n_vertices;
	ui32 n_spare;
	ui32 pool_size_;
	FILE* p_file;
};