#pragma once

#include "my_int.h"

namespace binary {

	inline ui32 size(ui32 bitsize) { return (bitsize + UI32_BITS - 1) >> UI32_LOG2BIT; }
	inline ui32 mask(ui32 bitsize) { return UI32_ALL >> (UI32_BITS - bitsize & UI32_MASK); }
	ui32 popcount(const ui32* p, ui32 bitsize);
	ui32 find_next(const ui32* p, ui32 bitsize, ui32 bit);
	bool any(const ui32* p, ui32 bitsize);
	bool all(const ui32* p, ui32 bitsize);
	void set(ui32* p, ui32 bit);
	void reset(ui32* p, ui32 bit);
	void reset_le(ui32* p, ui32 bit);


	//matrix elements are stored by rows
	class Matrix {
	public:
		//taking and changing elements
		char at(ui32 i, ui32 j) const throw();
		void set(ui32 i, ui32 j) throw();
		void set(ui32 j) throw();//when m_==1
		void reset(ui32 i, ui32 j) throw();
		void reset(ui32 j) throw();//when m_==1
		ui32* row(ui32 i) throw();
		const ui32* row(ui32 i) const throw();

		//stats
		ui32 row_size() const { return size_from_bitsize(n_); }
		ui32 width() const { return n_; }
		ui32 height() const { return m_; }

		//service functions
		void copy(const Matrix& src);
		//void swap(Matrix& src) throw();
		void transpose(const Matrix& src);
		void submatrix(const ui32* rows);
		//void random(ui32 m, ui32 n, float d = 0.5, unsigned seed = 0);//generate random matrix with P(a[i,j]=1)=d

		//constructors
		Matrix(ui32 m = 0, ui32 n = 0) : data_(nullptr), n_(0), m_(0) { reserve(m, n); }
		Matrix(const Matrix& src) : data_(nullptr), n_(0), m_(0) { copy(src); }
		//Bool_Matrix(const Bool_Matrix& src, const Bool_Vector& rows);
		Matrix& operator=(const Matrix& src) { copy(src); return *this; }
		~Matrix() { reserve(0, 0); }

		//io functions
		void read(FILE* pFile);
		void read(const char* file_name);
		void print(FILE* pFile) const;
		void print(const char* file_name, const char* mode = "w") const;
		

	protected:
		void reserve(ui32 m, ui32 n);
		static ui32 size_from_bitsize(ui32 bitsz) throw() { return (bitsz + UI32_BITS - 1) >> UI32_LOG2BIT; }
	protected:
		ui32* data_;
		ui32 n_;//matrix width
		ui32 m_;//matrix hight
		ui32 capacity_;//storage capacity
	};

}