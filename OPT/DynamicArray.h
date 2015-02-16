#pragma once

// Class DynamicArray makes a dynamic array which can grow as new data are added
template <typename TX>
class DynamicArray {
public:
	DynamicArray() throw();                             
	~DynamicArray() throw();                             
	void Reserve(int num);// Allocate buffer for num objects
	void SetNum(int num);// Set the number of valid entries. New entries will be zero
	int GetNum() { return NumEntries; };// Get number of objects stored
	int GetMaxNum() { return MaxNum; };// Get number of objects that can be stored without re-allocating memory
	int Push(TX const & obj);// Add object to end of array. Return its index
	void Pop();// Delete last object out of list
	TX & Top();// Take last object
	TX & operator[] (int i);// Access object with index i
	// Define desired allocation size
	enum DefineSize {
		AllocateSpace = 1024                       // Minimum size, in bytes, of automatic re-allocation done by Push
	};
private:
	TX * Buffer;// Buffer containing data
	TX * OldBuffer;// Old buffer before re-allocation
	int MaxNum;// Maximum number of objects that buffer can contain
	void ReAllocate(int num);// Allocate new memory buffer, leave OldBuffer intact
	DynamicArray(DynamicArray const&) {};// Make private copy constructor to prevent copying
	void operator = (DynamicArray const&) {};// Make private assignment operator to prevent copying
protected:
	int NumEntries;// Number of objects stored
	//void Error(int e, int n);// Make fatal error message
};