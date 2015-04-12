#pragma once
#include <cstdio>
#include "my_int.h"
#include "stack_array.h"
#include "binary.h"

class Dualizer_OPT {
protected:

	class Covering {
	public:
		void reserve(ui32 size, ui32 width);
		~Covering();
		void append(ui32 num);
		void remove_last();
		ui32& top();
		void print(FILE* p_file);
		ui32 operator[] (ui32 ind) const throw();		
		ui32 size() const throw();
		void print_freq(ui32 width);
		ui32* get_freq();
	private:
		Stack_Array<ui32> data_;
		Stack_Array<char> text_;
		ui32* frequency_ = nullptr;
	};

public:

	Dualizer_OPT() { My_Memory::MM_memset(this, 0, sizeof(Dualizer_OPT)); }
	~Dualizer_OPT() { clear(); }

	//preprocesses matrix, allocates memory, set matrix_, matrix_t_, m_, n_, etc
	void init(const binary::Matrix& L, const char* file_name = nullptr, const char* mode = "wb");
	void clear() throw();
	void run();
	void print() {
		printf("%d\n", n_coverings);
		if (p_file == nullptr)
			covering.print_freq(n());
	}
	ui32* get_freq() { return covering.get_freq(); }

protected:

	void update_covered_and_support_rows(ui32 j) throw();

	void delete_zero_cols() throw();
	void delete_zero_cols1() throw();
	void delete_zero_cols2() throw();
	
	void delete_fobidden_cols()  throw();
	void delete_fobidden_cols1() throw();
	void delete_fobidden_cols2() throw();
	void delete_fobidden_cols3() throw();
	void delete_fobidden_cols4() throw();
	
	void delete_le_rows() throw();

	//char create_search_set(ui32* set) throw();

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

private:
	ui32* pool_;

	ui32* matrix_;
	ui32* cols;
	ui32* rows;
	ui32* support_rows;
	ui32* p_j;
	ui32* matrix_t_;
	ui32* cov;	
	Covering covering;

	ui32 m_;
	ui32 n_;
	ui32 n_coverings;
	FILE* p_file;
	char* file_buffer_;
};