#pragma once

#include <memory.h>//for memcpy and memset
#include <stdexcept>//for std::runtime_error

// Class DynamicArray makes a dynamic array which can grow as new data are added
template <typename TX>
class DynamicArray {
public:
	DynamicArray();
	~DynamicArray() throw();
	void Reserve(int num);// Allocate buffer for num objects
	void SetNum(int num);// Set the number of valid entries. New entries will be zero
	int GetNum() { return NumEntries; };// Get number of objects stored
	int GetMaxNum() { return MaxNum; };// Get number of objects that can be stored without re-allocating memory
	void Push_Empty();// Add empty object to end of array. Return its index
	void Push(TX const & obj);// Add object to end of array. Return its index
	void Pop();
	TX& Top();
	TX& operator[] (int i);
	const TX& operator[] (int i) const;
	// Define desired allocation size
	enum DefineSize {
		AllocateSpace = 1024// Minimum size, in bytes, of automatic re-allocation done by Push
	};
private:
	TX* Buffer;// Buffer containing data
	TX* OldBuffer;// Old buffer before re-allocation
	int MaxNum;// Maximum number of objects that buffer can contain
	void ReAllocate(int num);// Allocate new memory buffer, leave OldBuffer intact
	DynamicArray(DynamicArray const&) {};// Make private copy constructor to prevent copying
	void operator = (DynamicArray const&) {};// Make private assignment operator to prevent copying
protected:
	int NumEntries;// Number of objects stored
	//void Error(int e, int n);// Make fatal error message
};

template <typename TX>
DynamicArray<TX>::DynamicArray() :
Buffer(nullptr), OldBuffer(nullptr), MaxNum(0), NumEntries(0) {}

template <typename TX>
DynamicArray<TX>::~DynamicArray() throw() {
	Reserve(0);
}

template <typename TX>
void DynamicArray<TX>::Reserve(int num) {
	// Setting num > current MaxNum will allocate a larger buffer and move all data to the new buffer.
	// Setting num <= current MaxNum will do nothing. The buffer will only grow, not shrink.
	// Setting num = 0 will discard all data and de-allocate the buffer.
	if (num <= MaxNum) {
		if (num <= 0) {
			if (num < 0) {
				throw std::runtime_error("DynamicArray::Reserve::Index out of range");
			}
			// num = 0. Discard data and de-allocate buffer
			if (Buffer != nullptr) {
				delete[] Buffer;
				Buffer = nullptr;
			}
			MaxNum = NumEntries = 0;
			return;
		}
		// Request to reduce size. Ignore
		return;
	}
	// num > MaxNum. Increase Buffer
	ReAllocate(num);
	// OldBuffer must be deleted after calling ReAllocate
	if (OldBuffer) {
		delete[] OldBuffer;
		OldBuffer = nullptr;
	}
}

template <typename TX>
void DynamicArray<TX>::ReAllocate(int num) {
	// Increase size of memory buffer. 
	// This function is used only internally. 
	// Note: ReAllocate leaves OldBuffer to be deleted by the calling function,
	// just to cover the case where an object being copied into the new buffer
	// happens to be contained in the old buffer.
	if (OldBuffer != nullptr) {
		delete[] OldBuffer;// Should not occur in single-threaded applications
		OldBuffer = nullptr;
	}
	TX * Buffer2 = nullptr;// New buffer
	Buffer2 = new TX[num];// Allocate new buffer
	if (Buffer2 == nullptr) { //-V668
		throw std::runtime_error("DynamicArray::ReAllocate::Memory allocation failed");
	}// Error can't allocate
	if (Buffer != nullptr) {
		// A smaller buffer is previously allocated
		memcpy(Buffer2, Buffer, MaxNum*sizeof(TX));// Copy contents of old buffer into new one
	}
	OldBuffer = Buffer;// Save old buffer. Must be deleted by calling function
	Buffer = Buffer2;// Save pointer to buffer
	MaxNum = num;// Save new size
}

template <typename TX>
void DynamicArray<TX>::SetNum(int num) {
	// Set the number of objects that are considered used and valid.
	// NumEntries is initially zero. It is increased by Push or SetNum
	// Setting num > NumEntries is equivalent to pushing (num - NumEntries) objects with zero contents.
	// Setting num < NumEntries will decrease NumEntries so that all objects with index >= num are erased.
	// Setting num = 0 will erase all objects, but not de-allocate the buffer.
	if (num < 0) {
		throw std::runtime_error("DynamicArray::SetNum::Index out of range");
	}
	if (num > MaxNum) {
		Reserve(num);
	}
	if (num > NumEntries) {
		My_Memory::MM_memset(Buffer + NumEntries, 0, (num - NumEntries) * sizeof(TX));// Fill new entries with zero
	}
	NumEntries = num;//Set new DataSize
}

template <typename TX>
void DynamicArray<TX>::Push(const TX & obj) {
	// Add object to buffer, return index
	if (NumEntries >= MaxNum) {
		int NewSize = MaxNum * 2 + (AllocateSpace + sizeof(TX) - 1) / sizeof(TX);
		ReAllocate(NewSize);
	}
	Buffer[NumEntries] = obj;// Insert at top
	if (OldBuffer != nullptr) {
		// Old buffer can only be deleted after copying object, because obj might be contained in old buffer
		delete[] OldBuffer;
		OldBuffer = nullptr;
	}
	++NumEntries;// Increment NumEntries and return current index
}

template <typename TX>
void DynamicArray<TX>::Push_Empty() {
	// Add object to buffer, return index
	if (NumEntries >= MaxNum) {
		int NewSize = MaxNum * 2 + (AllocateSpace + sizeof(TX) - 1) / sizeof(TX);
		ReAllocate(NewSize);
	}
	//Buffer[NumEntries] = obj;// Insert at top
	if (OldBuffer != nullptr) {
		// Old buffer can only be deleted after copying object, because obj might be contained in old buffer
		delete[] OldBuffer;
		OldBuffer = nullptr;
	}
	++NumEntries;// Increment NumEntries and return current index
}

template <typename TX>
void DynamicArray<TX>::Pop() {
	if (NumEntries <= 0) {
		throw std::runtime_error("DynamicArray::Pop::Array is empty");
	}
	--NumEntries;
}

template <typename TX>
TX& DynamicArray<TX>::Top() {
	if (NumEntries <= 0) {
		throw std::runtime_error("DynamicArray::Top::Array is empty");
	}
	return Buffer[NumEntries - 1];
}

template <typename TX>
TX& DynamicArray<TX>::operator[] (int i) {
	if (i >= NumEntries) {
		throw std::runtime_error("DynamicArray::operator[]::Index out of range");
	}
	return Buffer[i];
}

template <typename TX>
const TX& DynamicArray<TX>::operator[] (int i) const {
	if (i >= NumEntries) {
		throw std::runtime_error("DynamicArray::operator[]::Index out of range");
	}
	return Buffer[i];
}
