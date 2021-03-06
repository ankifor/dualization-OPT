#pragma once

#include <cmath>
#include "my_int.h"
#include "my_memory.h"

class Pool_Stack {
public:
	Pool_Stack() {
		start_ = nullptr;
		top_ = nullptr;
		element_size_ = 0; 
		capacity_ = 0;
	}

	void set_element_size(ui32 element_size) {
		//recalculate capacity
		if (element_size_ != 0 && element_size != element_size_) {
			double capacity1 = capacity_;
			double sz1 = element_size_;
			double sz2 = element_size;
			capacity1 = capacity1 * sz1 * double(UI32_SIZE) / (sz2 * double(UI32_SIZE));
			capacity_ = ui32(floor(capacity1));
		}
		element_size_ = element_size;
	}

	~Pool_Stack() throw() { reserve(0); }

	void reserve(ui32 capacity) {
		if (capacity == 0) {//deallocate memory
			if (capacity_ > 0) {
				My_Memory::MM_free(start_);
			}
			capacity_ = 0;
			start_ = nullptr;
			top_ = nullptr;
		} else {
			if (capacity_ < capacity) {//reallocation
				ui32* buf = start_;
				start_ = static_cast<ui32*>(My_Memory::MM_malloc(capacity*element_size_*UI32_SIZE));
				if (capacity_ > 0) {
					My_Memory::MM_memcpy(start_, buf, capacity_*element_size_*UI32_SIZE);
					top_ = start_ + (top_ - buf);
					My_Memory::MM_free(buf);
				} else {
					top_ = start_;
				}
				buf = nullptr;
				My_Memory::MM_memset(start_ + capacity_*element_size_, 0,
					(capacity - capacity_)*element_size_*UI32_SIZE);
				capacity_ = capacity;
			}
		}
	}

	ui32 size() const throw() {
		assert(start_ <= top_);
		return ui32((top_ - start_ + element_size_ - 1) / element_size_);
	}

	ui32 element_size() const throw() {
		return element_size_;
	}

	void push_empty() {
		if (size() + 1 >= capacity_) {
			ui32 capacity = capacity_ * 2 + (AllocateSpace + element_size_ - 1) / element_size_;
			reserve(capacity);
		}
		top_ += element_size_;
	}

	void pop() throw() {
		assert(start_ <= top_);
		top_ -= element_size_;
	}

	ui32* top() throw() {
		assert(start_ <= top_);
		return top_;
	}

	// Define desired allocation size
	enum DefineSize {
		AllocateSpace = 1024// Minimum size, in bytes, of automatic re-allocation done by Push
	};

protected:
	Pool_Stack(const Pool_Stack&) {};
	void operator = (const Pool_Stack&) {};

private:
	ui32* start_;
	ui32* top_;
	ui32 element_size_;
	ui32 capacity_;
};