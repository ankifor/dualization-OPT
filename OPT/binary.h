#pragma once

#include "my_int.h"

namespace binary {

	inline ui32 size(ui32 bitsize) { return (bitsize + UI32_BITS - 1) >> UI32_LOG2BIT; }
	inline ui32 size64(ui32 bitsize) { return (bitsize + UI64_BITS - 1) >> UI64_LOG2BIT; }
	inline ui32 mask(ui32 bitsize) { return UI32_ALL >> (UI32_BITS - bitsize & UI32_MASK); }
	inline ui64 mask64(ui32 bitsize) { return UI64_ALL >> (UI64_BITS - bitsize & UI64_MASK); }
	ui32 popcount(const ui32* p, ui32 bitsize);
	ui32 find_next(const ui32* p, ui32 bitsize, ui32 bit);
	ui32 find_prev(const ui32* p, ui32 bitsize, ui32 bit);
	//ui32 find_first(const ui32* p, ui32& ind);
	bool any(const ui32* p, ui32 bitsize);
	bool all(const ui32* p, ui32 bitsize);
	char at(const ui32* p, ui32 bit);
	void set(ui32* p, ui32 bit);
	void reset(ui32* p, ui32 bit);
	void reset_le(ui32* p, ui32 bit);


	void submatrix(ui32* dst, ui32* src, const ui32* rows, ui32 m, ui32 n);
	void transpose(ui32* dst, const ui32* src, ui32 m, ui32 n);

	//matrix elements are stored by rows
	class Matrix {
	public:
		//taking and changing elements
		char at(ui32 i, ui32 j) const throw() { return binary::at(row(i), j); }
		void set(ui32 i, ui32 j) throw() { binary::set(row(i), j); }
		void reset(ui32 i, ui32 j) throw() { binary::reset(row(i), j); }
		ui32* row(ui32 i) throw() { return data_ + i*row_size(); }
		const ui32* row(ui32 i) const throw() { return data_ + i*row_size(); }

		//stats
		inline ui32 row_size() const { return size(n_); }
		inline ui32 size32() const { return m_ * size(n_); }
		inline ui32 width() const { return n_; }
		inline ui32 height() const { return m_; }

		//service functions
		void copy(const Matrix& src);
		//void swap(Matrix& src) throw();
		//void transpose(const Matrix& src);
		//void submatrix(const ui32* rows);
		void random(ui32 m, ui32 n, float d);//generate random matrix with P(a[i,j]=1)=d

		//constructors
		Matrix(ui32 m = 0, ui32 n = 0) : data_(nullptr), n_(0), m_(0) { reserve(m, n); }
		void reserve(ui32 m, ui32 n);
		Matrix(const Matrix& src) : data_(nullptr), n_(0), m_(0) { copy(src); }
		//Bool_Matrix(const Bool_Matrix& src, const Bool_Vector& rows);
		Matrix& operator=(const Matrix& src) { copy(src); return *this; }
		~Matrix() { reserve(0, 0); }

		//io functions
		void read(FILE* pFile);
		void read(const char* file_name);
		void print_bm(FILE* pFile) const;
		void print_0x(FILE* pFile) const;
		void print_hg(FILE* pFile) const;
		void print(const char* file_name, const char* mode = "bm") const;
		//special functions
		Matrix& delete_le_rows();
		Matrix& random_stripe(const Matrix& src, ui32 height);
	protected:
		ui32* data_;
		ui32 n_;//matrix width
		ui32 m_;//matrix hight
		ui32 capacity_;//storage capacity
	};


}