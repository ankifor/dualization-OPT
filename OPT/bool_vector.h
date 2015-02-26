#pragma once
#include "my_int.h"


class Bool_Vector {
public:

	ui32 find_next(ui32 bit) const throw();
	ui32 popcount() const throw();
	bool any() const throw();
	bool all() const throw();

	ui32 at(ui32 bit) const throw();
	void set(ui32 bit) throw();
	void reset(ui32 bit) throw();
	void setall() throw();
	void resetall() throw();
	void resetupto(ui32 bit) throw();
	//void reset_irrelevant_bits() throw();

	inline ui32& operator[] (ui32 ind) throw() { return data_[ind]; }
	inline const ui32& operator[] (ui32 ind) const throw() { return data_[ind]; }
	inline const void* get_data() const throw() { return data_; }

	inline ui32 size() const throw() { return size_; }
	inline ui32 bitsize() const throw() { return bitsize_; }
	inline ui32 mask() const throw() { return last_mask_; }

	void copy(const Bool_Vector& src);
	void assign(ui32* data, ui32 bitsz);//nocopy
	void reserve(ui32 bitsz) { init_stats_(bitsz); reserve_(size_); }

	Bool_Vector() : data_(nullptr), capacity_(0) { init_stats_(0); reserve_(0); }
	Bool_Vector(ui32* data, ui32 bitsz) : data_(nullptr), capacity_(0) { assign(data, bitsz); }
	~Bool_Vector() { init_stats_(0);  reserve_(0); }

protected:

	void init_stats_(ui32 bitsz) throw();//modifies bitsize_, size_, last_, mask_
	void reserve_(ui32 capacity);	
	//static ui32 size_from_bitsize(ui32 bitsz) throw() { return (bitsz + UI32_BITS - 1) >> UI32_LOG2BIT; }

protected:

	ui32* data_;
	ui32 bitsize_;
	ui32 size_;
	ui32 last_mask_;
	ui32 capacity_;
};

