#pragma once

#include <stdexcept>//for std::runtime_error
#include <assert.h>
#include "my_int.h"
#include "my_memory.h"

template <class TX>
class Stack_Array {
public:
	Stack_Array() : data_(nullptr), capacity_(0), size_(0) {}
	~Stack_Array() throw() { reserve(0); }

	void reserve(ui32 capacity) {
		if (capacity == 0) {//deallocate memory
			if (capacity_ > 0) {
				//calling destructors
				for (ui32 ind = 0; ind < capacity_; ++ind) {
					data_[ind].~TX();
				}
				My_Memory::MM_free(data_);
			}
			capacity_ = 0;
			data_ = nullptr;
		} else {
			if (capacity_ < capacity) {//reallocation
				TX* buf = data_;
				data_ = static_cast<TX*>(My_Memory::MM_malloc(capacity*sizeof(TX)));
				if (capacity_ > 0) {					
					My_Memory::MM_memcpy(data_, buf, capacity_*sizeof(TX));
					My_Memory::MM_free(buf);
				}
				//calling constructors
				for (ui32 ind = capacity_; ind < capacity; ++ind) {
					data_[ind] = TX();
				}
				buf = nullptr;
				capacity_ = capacity;
			}
		}
	}

	void resize_to_capacity() {
		size_ = capacity_;
	}

	ui32 size() const throw() { return size_; }

	void push_empty() {
		if (size_ >= capacity_) {
			ui32 capacity = capacity_ * 2 + (AllocateSpace + sizeof(TX) - 1) / sizeof(TX);
			reserve(capacity);
		}
		++size_;
	}
	
	void push(const TX& obj) {
		if (size_ >= capacity_) {
			ui32 capacity = capacity_ * 2 + (AllocateSpace + sizeof(TX) - 1) / sizeof(TX);
			reserve(capacity);
		}
		data_[size_] = obj;
		++size_;
	}
	
	void pop() throw() {
		assert(size_ > 0);
		//if (size_ == 0) {
		//	throw std::runtime_error("stack_array::pop::array is empty");
		//}
		--size_;
	}
	
	TX& top() throw() {
		assert(size_ > 0);
		//if (size_ == 0) {
		//	throw std::runtime_error("stack_array::pop::array is empty");
		//}
		return data_[size_ - 1];
	}

	TX& operator[] (ui32 ind) throw() {
		assert(ind < size_);
		return data_[ind];
	}

	const TX& operator[] (ui32 ind) const throw() {
		assert(ind < size_);
		return data_[ind];
	}
	
	const TX* get_data() const throw() {
		return data_;
	}

	TX* get_data() throw() {
		return data_;
	}
	
	// Define desired allocation size
	enum DefineSize {
		AllocateSpace = 1024// Minimum size, in bytes, of automatic re-allocation done by Push
	};

protected:
	TX* data_;
	ui32 capacity_;
	ui32 size_;

	Stack_Array(const Stack_Array&) {};
	void operator = (const Stack_Array&) {};
};
