#pragma once

//#include "DynamicArray.h"
#include "bool_vector.h"
#include "my_int.h"

//matrix elements are stored by rows
class Bool_Matrix {
public:	
	//taking and changing elements
	char at(ui32 i, ui32 j) const throw();
	void set(ui32 i, ui32 j) throw();
	void set(ui32 j) throw();//when m_==1
	void reset(ui32 i, ui32 j) throw();
	void reset(ui32 j) throw();//when m_==1
	Bool_Vector row(ui32 i) throw();
	const Bool_Vector row(ui32 i) const throw();

	//stats
	ui32 row_size() const { return size_from_bitsize(n_); }
	ui32 width() const { return n_; }
	ui32 height() const { return m_;	}

	//service functions
	void copy(const Bool_Matrix& src);
	void swap(Bool_Matrix& src) throw();
	void transpose(const Bool_Matrix& src);
	void submatrix(const Bool_Vector& rows);
	void random(ui32 m, ui32 n, float d = 0.5, unsigned seed = 0);//generate random matrix with P(a[i,j]=1)=d

	//constructors
	Bool_Matrix(ui32 m = 0, ui32 n = 0) : data_(nullptr), n_(0), m_(0) { reserve(m, n); }
	Bool_Matrix(const Bool_Matrix& src) : data_(nullptr), n_(0), m_(0) { copy(src); }
	//Bool_Matrix(const Bool_Matrix& src, const Bool_Vector& rows);
	Bool_Matrix& operator=(const Bool_Matrix& src) { copy(src); return *this; }
	~Bool_Matrix() { reserve(0,0); }

	//io functions
	//void read(const DynamicArray<char>& data, ui32 m, ui32 n);
	void read(FILE* pFile);
	void read(const char* file_name);
	void print(const char* file_name, const char* mode = "w") const;
	void print(FILE* pFile) const;

protected:
	void reserve(ui32 m, ui32 n);
	static ui32 size_from_bitsize(ui32 bitsz) throw() { return (bitsz + UI32_BITS - 1) >> UI32_LOG2BIT; }
protected:
	ui32* data_;  
	ui32 n_;//matrix width
	ui32 m_;//matrix hight
	ui32 capacity_;//storage capacity
};
