#pragma once

#include "DynamicArray.h"
#include "my_int.h"

//matrix elements are stored by rows
class Bool_Matrix {
public:
	ui32 find_next(ui32 i) const throw();
	ui32 popcount() const throw();
	void copy_and_transpose(const Bool_Matrix& src);
	//taking and changing elements
	char at(ui32 i, ui32 j) const throw();
	char at(ui32 j) const throw();//when m_==1
	void set(ui32 i, ui32 j) throw();
	void set(ui32 j) throw();//when m_==1
	void reset(ui32 i, ui32 j) throw();
	void reset(ui32 j) throw();//when m_==1
	ui32* row(ui32 i) throw();
	const ui32* row(ui32 i) const throw();
	//stats
	ui32 size32() const { return sz32_; }
	ui32 width() const { return n_; }
	ui32 height() const { return m_;	}
	//reading data
	void read(const DynamicArray<char>& data, ui32 m, ui32 n);
	void read(FILE* pFile);	
	void read(const char* file_name);
	//printing data
	void print(const std::string& file_name, const char* mode = "w") const;
	void print(FILE* pFile) const;
	//copy, swap
	void copy(const Bool_Matrix& src);
	void swap(Bool_Matrix& src) throw();
	//generate random matrix with P(a[i,j]=1)=d
	void random(ui32 m, ui32 n, float d = 0.5, unsigned seed = 0);
	//constructing
	void init(ui32 m, ui32 n, char value = 0);//value should be either 0 or 0xFF
	void clear() throw();
	Bool_Matrix(ui32 m, ui32 n);
	Bool_Matrix(ui32 n);
	Bool_Matrix(const Bool_Matrix& src);
	Bool_Matrix(const Bool_Matrix& src, const Bool_Matrix& rows);
	Bool_Matrix& operator=(const Bool_Matrix& src);
	Bool_Matrix() throw();
	~Bool_Matrix() throw();
protected:
	ui32* data_;  
	ui32 n_;//matrix width
	ui32 m_;//matrix hight
	ui32 sz32_;//size in ui32
	ui32 n32_;//ui32 in a row
};
