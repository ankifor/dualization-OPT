#include <stdexcept>//for runtime_error
#include <intrin.h>//for _tzcnt_u32, __popcnt, _bittest, _bittestandset, _bittestandreset
#include <assert.h>
#include "bool_vector.h"
#include "my_memory.h"


ui32 Bool_Vector::find_next(ui32 bit) const {
	assert(size_ > 0);
	if (bit >= bitsize_)
		return bitsize_;
	ui32 ind = bit >> UI32_LOG2BIT;
	ui32 offset = bit & UI32_MASK;
	ui32 buf = (data_[ind] >> offset) << offset;

	while (ind < size_) {
		offset = _tzcnt_u32(buf);//UI32_BITS==32
		if (offset == UI32_BITS) {
			++ind;
			buf = data_[ind];
		} else {
			break;
		}
	}
	return (ind << UI32_LOG2BIT) + offset;
}

ui32 Bool_Vector::popcount() const {
	assert(size_ > 0);
	ui32 sum = 0;
	ui32 sz = size();

	for (ui32 ind = 0; ind < size_ - 1; ++ind) {
		sum += __popcnt(data_[ind]);
	}
	sum += __popcnt(data_[size_-1] & last_mask_);
	return sum;
}

bool Bool_Vector::any() const {
	bool res = false;
	assert(bitsize_ > 0);
	res = (*data_ != 0) || (My_Memory::MM_memcmp(data_, data_ + 1, size_ - UI32_SIZE) != 0);
	return res;
}

bool Bool_Vector::all() const {
	bool res = false;
	assert(size_ > 0);
	data_[size_ - 1] |= ~last_mask_;
	res = (*data_ != ~0) || (My_Memory::MM_memcmp(data_, data_ + 1, size_ - UI32_SIZE) != 0);
	data_[size_ - 1] &= last_mask_;
	return !res;
}

ui32 Bool_Vector::at(ui32 bit) const {
	ui32 ind = (bit >> UI32_LOG2BIT);
	return _bittest(reinterpret_cast<const long*>(data_ + ind), bit & UI32_MASK);
}

void Bool_Vector::set(ui32 bit) {
	ui32 ind = (bit >> UI32_LOG2BIT);
	_bittestandset(reinterpret_cast<long*>(data_ + ind), bit & UI32_MASK);
}

void Bool_Vector::reset(ui32 bit) {
	ui32 ind = (bit >> UI32_LOG2BIT);
	_bittestandreset(reinterpret_cast<long*>(data_ + ind), bit & UI32_MASK);
}

void Bool_Vector::setall() {
	assert(size_ > 0);
	My_Memory::MM_memset(data_, -1, size_*UI32_SIZE);
	reset_irrelevant_bits();
}

void Bool_Vector::resetall() {
	assert(size_ > 0);
	My_Memory::MM_memset(data_, 0, size_*UI32_SIZE);
}

void Bool_Vector::resetupto(ui32 bit) {
	if (bit >= bitsize_)
		bit = bitsize_;
	ui32 k = bit >> UI32_LOG2BIT;
	My_Memory::MM_memset(data_, 0, k*UI32_SIZE);
	ui32 offset = bit & UI32_MASK;
	if (offset > 0) {
		ui32 mask = UI32_ALL << offset;
		data_[k] &= mask;
	}
}

void Bool_Vector::reset_irrelevant_bits() {
	assert(size_ > 0);
	data_[size_ - 1] &= last_mask_;
}

void Bool_Vector::copy(const Bool_Vector& src) {
	init_stats_(src.bitsize_);
	reserve_();
	memcpy(data_, src.data_, size_*UI32_SIZE);
}

void Bool_Vector::assign(ui32* data, ui32 bitsz) {
	init_stats_(bitsz);
	reserve_();
	data_ = data;
}

void Bool_Vector::init_stats_(ui32 bitsz) {
	bitsize_ = bitsz;
	if (bitsize_ != 0) {
		size_ = ((bitsize_ - 1) >> UI32_LOG2BIT) + 1;
		last_mask_ = ~(UI32_ALL << (bitsize_ & UI32_MASK));
	} else {
		last_mask_ = ~0;
		size_ = 0;
	}
}

void Bool_Vector::reserve_() {
	if (size_ == 0) {//deallocate memory
		if (capacity_ > 0)
			My_Memory::MM_free(data_);
		capacity_ = 0;
		data_ = nullptr;
	} else {
		if (capacity_ < size_) {//reallocation
			if (capacity_ > 0)
				My_Memory::MM_free(data_);
			data_ = static_cast<ui32*>(My_Memory::MM_malloc(size_*UI32_SIZE));
			capacity_ = size_;
		}
	}
}