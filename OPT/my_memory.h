#pragma once

#include <memory.h>
#include <malloc.h>
#include <stdexcept>
#include <cstdio>

class My_Memory {
public:
	static void* MM_malloc(int size) {
		void* data = malloc(size);
		if (data == nullptr)
			throw std::runtime_error("My_Memory::MM_malloc::Memory allocation problem");
		malloc_size += size;
		++malloc_times;
		return data;
	}

	static void MM_free(void* data) throw() {
		if (data != nullptr) {
			free(data);
			++free_times;
		}
	}

	static void* MM_memset(void* dst, int val, int size) {
		void* data = nullptr;
		data = memset(dst, val, size)	;
		memset_size += size;
		++memset_times;
		return data;
	}

	static void* MM_memcpy(void* dst, const void* src, int size) {
		void* data = nullptr;
		data = memcpy(dst, src, size);
		memcpy_size += size;
		++memcpy_times;
		return data;
	}

	static void print() {
		printf("My_Memory statistics\n");
		printf("\tmalloc: %d, %d b\n", malloc_times, malloc_size);
		printf("\tfree  : %d\n", free_times);
		printf("\tmemset: %d, %d b\n", memset_times, memset_size);
		printf("\tmemcpy: %d, %d b\n", memcpy_times, memcpy_size);
	}

private:
	static int malloc_times;
	static int free_times;   
	static int memset_times;
	static int memcpy_times;
	static int malloc_size;
	static int memcpy_size;
	static int memset_size;
};