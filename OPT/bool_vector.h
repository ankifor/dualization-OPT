#pragma once
#include "my_int.h"


class Bool_Vector {
public:

	ui32 find_next(ui32 bit) const throw();
	ui32 popcount() const throw();

	ui32 at(ui32 bit) const throw();
	void set(ui32 bit) throw();
	void reset(ui32 bit) throw();
	void setall() throw();
	void resetall() throw();
	ui32& operator[] (ui32 ind) throw() { return data_[ind]; }

	ui32 size() const throw() { return size_from_bitsize(bitsize_); }
	ui32 bitsize() const throw() { return bitsize_; }
	ui32 mask() const throw() { return ALL >> (BITS - (bitsize_ & MASK)); }

	void copy(const Bool_Vector& src);
	void assign(ui32* data, ui32 bitsz);//nocopy

	Bool_Vector(ui32 bitsz = 0) : data_(nullptr), bitsize_(0), capacity_(0) { reserve(bitsz); }
	Bool_Vector(ui32* data, ui32 bitsz) throw() : data_(data), bitsize_(bitsz), capacity_(0) {} //nocopy
	~Bool_Vector() { reserve(0); }

protected:

	void reserve(ui32 bitsz);
	static ui32 size_from_bitsize(ui32 bitsz) throw() { return (bitsz + BITS - 1) >> LOG2BIT; }

protected:

	ui32* data_;
	ui32 bitsize_;
	ui32 capacity_;//not owner iff capacity = 0
	//bool owner;
};

