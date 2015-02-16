/*************************  DynamicArray.cpp  *********************************
* Author:        Agner Fog
* Date created:  2008-06-12
* Last modified: 2008-07-24
* Description:
* Defines linear array of dynamic size.
*
* The latest version of this file is available at:
* www.agner.org/optimize/cppexamples.zip
* (c) 2008 GNU General Public License www.gnu.org/copyleft/gpl.html
*******************************************************************************
*
* DynamicArray is a container class defining a dynamic array or memory pool 
* that can contain any number of objects of the same type.
*
* The objects stored can be of any type that do not require a constructor
* or destructor. The size of the memory pool can grow, but not shrink.
* Objects cannot be removed randomly. Objects can only be removed by 
* calling SetNum, which removes all subsequently stored objects as well.
* While this restriction can cause some memory space to be wasted, it has
* the advantage that no time-consuming garbage collection is needed.
*
* DynamicArray is not thread safe if shared between threads. If your program 
* needs storage in multiple threads then each thread must have its own 
* private instance of DynamicArray, or you must prevent other threads from
* accessing DynamicArray while you are adding objects.
*
* Note that you should never store a pointer to an object in DynamicArray
* if Push() is used because the pointer will become invalid in case a 
* subsequent addition of another object by Push() causes the memory to be 
* re-allocated.
*
* It is possible, but not necessary, to allocate a certain amount of 
* memory before adding any objects. This can reduce the risk of having
* to re-allocate memory if the first allocated memory block turns out
* to be too small. Use Reserve(n) to reserve space for a dynamically
* growing array, or use SetNum(n) to set a fixed size array.
*
* At the end of this file you will find a working example of how to use 
* DynamicArray.
*
******************************************************************************/

#include <memory.h>// For memcpy and memset
//#include <stdlib.h>// For exit in Error function
//#include <stdio.h>// Needed for example only
#include <stdexcept>
#include "DynamicArray.h"



// Members of class DynamicArray
template <typename TX>
DynamicArray<TX>::DynamicArray() {  
   // Constructor
   Buffer = OldBuffer = nullptr;
   MaxNum = NumEntries = 0;
}


template <typename TX>
DynamicArray<TX>::~DynamicArray() {
   // Destructor
   Reserve(0);                                   // De-allocate buffer
}


template <typename TX>
void DynamicArray<TX>::Reserve(int num) {
   // Allocate buffer of the specified size
   // Setting num > current MaxNum will allocate a larger buffer and 
   // move all data to the new buffer.
   // Setting num <= current MaxNum will do nothing. The buffer will 
   // only grow, not shrink.
   // Setting num = 0 will discard all data and de-allocate the buffer.
   if (num <= MaxNum) {
      if (num <= 0) {
				if (num < 0) {
					//Error(1, num);
					throw std::runtime_error("DynamicArray::Reserve::Index out of range");
				}
					 
         // num = 0. Discard data and de-allocate buffer
         if (Buffer != nullptr) 
					 delete[] Buffer;// De-allocate buffer
         Buffer = nullptr;
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
      delete[] OldBuffer;  OldBuffer = 0;
   }
}

template <typename TX>
void DynamicArray<TX>::ReAllocate(int num) {
   // Increase size of memory buffer. 
   // This function is used only internally. 
   // Note: ReAllocate leaves OldBuffer to be deleted by the calling function,
   // just to cover the case where an object being copied into the new buffer
   // happens to be contained in the old buffer.
   if (OldBuffer != nullptr) 
		 delete[] OldBuffer;// Should not occur in single-threaded applications

   TX * Buffer2 = nullptr;// New buffer
   Buffer2 = new TX[num];// Allocate new buffer
   if (Buffer2 == nullptr) {
		 //Error(3,num); return;
		 throw std::runtime_error("DynamicArray::ReAllocate::Memory allocation failed");
	 }// Error can't allocate
   if (Buffer != nullptr) {
      // A smaller buffer is previously allocated
      memcpy(Buffer2, Buffer, MaxNum*sizeof(TX));// Copy contents of old buffer into new one
   }
   OldBuffer = Buffer;                           // Save old buffer. Must be deleted by calling function
   Buffer = Buffer2;                             // Save pointer to buffer
   MaxNum = num;                                 // Save new size
}


template <typename TX>
void DynamicArray<TX>::SetNum(int num) {
   // Set the number of objects that are considered used and valid.
   // NumEntries is initially zero. It is increased by Push or SetNum
   // Setting num > NumEntries is equivalent to pushing (num - NumEntries)
   // objects with zero contents.
   // Setting num < NumEntries will decrease NumEntries so that all objects 
   // with index >= num are erased.
   // Setting num = 0 will erase all objects, but not de-allocate the buffer.

   if (num < 0) { // Cannot be negative
      //Error(1, num); return;
		 throw std::runtime_error("DynamicArray::SetNum::Index out of range");
   }
   if (num > MaxNum) {
      // Allocate larger buffer. 
      Reserve(num);
   }
   if (num > NumEntries) {
      // Fill new entries with zero
      memset(Buffer + NumEntries, 0, (num - NumEntries) * sizeof(TX));
   }
   // Set new DataSize
   NumEntries = num;
}

   
template <typename TX>
int DynamicArray<TX>::Push(const TX & obj) {
   // Add object to buffer, return index

   if (NumEntries >= MaxNum) {
      // buffer too small or no buffer. Allocate more memory
      // Determine new size = 2 * current size + the number of objects that correspond to AllocateSpace
      int NewSize = MaxNum * 2 + (AllocateSpace+sizeof(TX)-1)/sizeof(TX);
      ReAllocate(NewSize);
   }
   Buffer[NumEntries] = obj;                     // Insert at top
   if (OldBuffer != nullptr) {
      // Old buffer can only be deleted after copying object, 
      // because obj might be contained in old buffer
      delete[] OldBuffer;  
			OldBuffer = nullptr;
   }
   return NumEntries++;                          // Increment NumEntries and return current index
}


template <typename TX>
void DynamicArray<TX>::Pop() {
   // Remove last object and return it
   if (NumEntries <= 0) {
      // buffer is empty. Make error message
      //Error(2, 0);
		  throw std::runtime_error("DynamicArray::Pop::Array is empty");
      // Return empty object
      TX temp;
      memset(&temp, 0, sizeof(temp));
      return temp;
   }
   // Return object and decrement NumEntries
   return Buffer[--NumEntries];
}


template <typename TX>
TX& DynamicArray<TX>::Top() {
	// Remove last object and return it
	if (NumEntries <= 0) {
		// buffer is empty. Make error message
		//Error(2, 0);
		throw std::runtime_error("DynamicArray::Top::Array is empty");
		// Return empty object
		TX temp;
		memset(&temp, 0, sizeof(temp));
		return temp;
	}
	// Return object
	return Buffer[NumEntries];
}

template <typename TX>
TX & DynamicArray<TX>::operator[] (int i) {
	// Access object with index i
	if ((unsigned int)i >= (unsigned int)NumEntries) {
		// Index i does not exist
		//Error(1, i);  i = 0;
		throw std::runtime_error("DynamicArray::operator[]::Index out of range");
	}
	return Buffer[i];
}

// Produce fatal error message. Used internally.
// Note: If your program has a graphical user interface (GUI) then you
// must rewrite this function to produce a message box with the error message.
//template <typename TX>
//void DynamicArray<TX>::Error(int e, int n) {
//   // Define error texts
//   static const char * ErrorTexts[] = {
//      "Unknown error",                 // 0
//      "Index out of range",            // 1
//      "Array is empty",                // 2
//      "Memory allocation failed"       // 3
//   };
//   // Number of texts in ErrorTexts
//   const unsigned int NumErrorTexts = sizeof(ErrorTexts) / sizeof(*ErrorTexts);
//
//   // check that index is within range
//   if ((unsigned int)e >= NumErrorTexts) e = 0;
//
//   // Replace this with your own error routine, possibly with a message box:
//   fprintf(stderr, "\nDynamicArray error: %s (%i)\n", ErrorTexts[e], n);
//	 
//   // Terminate execution
//   exit(1);
//}