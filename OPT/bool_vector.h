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
	void resetupto(ui32 bit) throw();
	ui32& operator[] (ui32 ind) throw() { return data_[ind]; }
	const ui32& operator[] (ui32 ind) const throw() { return data_[ind]; }

	ui32 size() const throw() { return size_from_bitsize(bitsize_); }
	ui32 bitsize() const throw() { return bitsize_; }
	ui32 mask() const throw() { return UI32_ALL >> (UI32_BITS - (bitsize_ & UI32_MASK)); }

	void copy(const Bool_Vector& src);
	void assign(ui32* data, ui32 bitsz);//nocopy
	void make_mask(ui32 bitsz);

	Bool_Vector(ui32 bitsz = 0) : data_(nullptr), bitsize_(0), capacity_(0) { reserve(bitsz); }
	Bool_Vector(ui32* data, ui32 bitsz) throw() : data_(data), bitsize_(bitsz), capacity_(0) {} //nocopy
	~Bool_Vector() { reserve(0); }

protected:

	void reserve(ui32 bitsz);
	static ui32 size_from_bitsize(ui32 bitsz) throw() { return (bitsz + UI32_BITS - 1) >> UI32_LOG2BIT; }

protected:

	ui32* data_;
	ui32 bitsize_;
	ui32 capacity_;
};

