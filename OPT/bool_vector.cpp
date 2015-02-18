#include <malloc.h>//for malloc
#include <stdexcept>//for runtime_error
#include <intrin.h>//for _tzcnt_u32, __popcnt, _bittest, _bittestandset, _bittestandreset
#include "bool_vector.h"


ui32 Bool_Vector::find_next(ui32 bit) const {
	ui32 ind = bit >> LOG2BIT;
	ui32 offset = bit & MASK;
	ui32 buf = (data_[ind] >> offset) << offset;
	ui32 sz = size();

	while (ind < sz) {
		offset = _tzcnt_u32(buf);//BITS==32
		if (offset == BITS) {
			++ind;
			buf = data_[ind];
		} else {
			break;
		}
	}
	return (ind << LOG2BIT) + offset;
}

ui32 Bool_Vector::popcount() const {
	ui32 sum = 0;
	ui32 ind = 0;
	ui32 sz = size();

	for (ind = 0; ind + 1 < sz; ++ind) {
		sum += __popcnt(data_[ind]);
	}
	sum += __popcnt(data_[ind] & mask());
	return sum;
}

ui32 Bool_Vector::at(ui32 bit) const {
	ui32 ind = (bit >> LOG2BIT);
	return _bittest(reinterpret_cast<const long*>(data_ + ind), bit & MASK);
}

void Bool_Vector::set(ui32 bit) {
	ui32 ind = (bit >> LOG2BIT);
	_bittestandset(reinterpret_cast<long*>(data_ + ind), bit & MASK);
}

void Bool_Vector::reset(ui32 bit) {
	ui32 ind = (bit >> LOG2BIT);
	_bittestandreset(reinterpret_cast<long*>(data_ + ind), bit & MASK);
}

void Bool_Vector::setall() {
	memset(data_, -1, size()*SIZE);
}

void Bool_Vector::resetall() {
	memset(data_, 0, size()*SIZE);
}

void Bool_Vector::copy(const Bool_Vector& src) {
	reserve(src.bitsize_);
	memcpy(data_, src.data_, size()*SIZE);
}

void Bool_Vector::assign(ui32* data, ui32 bitsz) {
	reserve(0);
	data_ = data;
	bitsize_ = bitsz;
}

void Bool_Vector::reserve(ui32 bitsz) {
	ui32 sz = size_from_bitsize(bitsz);
	if (sz == 0) {//deallocate memory
		if (capacity_ > 0 && data_ != nullptr)
			free(data_);
		capacity_ = 0;
		data_ = nullptr;
	} else {
		if (capacity_ < sz) {//reallocation
			if (capacity_ > 0 && data_ != nullptr)
				free(data_);
			data_ = static_cast<ui32*>(malloc(sz*SIZE));
			capacity_ = sz;
			if (data_ == nullptr)
				throw std::runtime_error("Bool_Vector::reserve::Memory allocation problem");
		}
	}
	bitsize_ = bitsz;
}