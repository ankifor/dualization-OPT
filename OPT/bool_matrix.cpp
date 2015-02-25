//#include <memory.h>//for memset, memcpy
#include <intrin.h>//for _bittest, _bittestandset, _bittestandreset
#include <cstdio>//for fgetc, EOF, fopen, fclose, fputc
#include <stdexcept>//for runtime_error
#include <time.h>//for time()
#include <cstdlib>//for rand
#include <algorithm>//for std::swap

#include "bool_matrix.h"
#include "my_memory.h"

using namespace std;

/**********************************************************
taking and changing elements
**********************************************************/

char Bool_Matrix::at(ui32 i, ui32 j) const {
	ui32 ind = i*row_size() + (j >> UI32_LOG2BIT);
	return _bittest(reinterpret_cast<const long*>(data_+ind), j & UI32_MASK);
}

void Bool_Matrix::set(ui32 i, ui32 j) {
	ui32 ind = i*row_size() + (j >> UI32_LOG2BIT);
	_bittestandset(reinterpret_cast<long*>(data_ + ind), j & UI32_MASK);
}

void Bool_Matrix::reset(ui32 i, ui32 j) {
	ui32 ind = i*row_size() + (j >> UI32_LOG2BIT);
	_bittestandreset(reinterpret_cast<long*>(data_ + ind), j & UI32_MASK);
}

Bool_Vector Bool_Matrix::row(ui32 i) {
	return Bool_Vector(&data_[i*row_size()], width());
}

const Bool_Vector Bool_Matrix::row(ui32 i) const {
	return Bool_Vector(&data_[i*row_size()], width());
}

/**********************************************************
service functions
**********************************************************/

void Bool_Matrix::copy(const Bool_Matrix& src) {
	if (this != &src) {
		reserve(src.m_, src.n_);
		memcpy(data_, src.data_, m_*row_size()*UI32_SIZE);
	}
}

void Bool_Matrix::swap(Bool_Matrix& src) {
	if (this != &src) {
		std::swap(m_, src.m_);
		std::swap(n_, src.n_);
		std::swap(data_, src.data_);
		std::swap(capacity_, src.capacity_);
	}
}

void Bool_Matrix::transpose(const Bool_Matrix& src) {
	reserve(src.n_, src.m_);
	My_Memory::MM_memset(data_, 0, m_*row_size()*UI32_SIZE);
	for (ui32 i = 0; i < src.m_; ++i) {
		for (ui32 j = 0; j < src.n_; ++j) {
			if (src.at(i, j))
				set(j, i);
		}
	}
}

void Bool_Matrix::submatrix(const Bool_Vector& rows) {
	reserve(rows.popcount(), n_);
	for (ui32 j = rows.find_next(0), k = 0; j < rows.bitsize(); j = rows.find_next(j + 1), ++k) {
		if (k != j)
			memcpy(&data_[k*row_size()], &data_[j*row_size()], row_size()*UI32_SIZE);
	}
}

void Bool_Matrix::reserve(ui32 m, ui32 n) {
	ui32 row_sz = size_from_bitsize(n);
	ui32 sz = m*row_sz;
	if (sz == 0) {//deallocate memory
		if (capacity_ > 0)
			My_Memory::MM_free(data_);
		capacity_ = 0;
		data_ = nullptr;
	} else {
		if (capacity_ < sz) {//reallocation
			if (capacity_ > 0)
				My_Memory::MM_free(data_);
			data_ = static_cast<ui32*>(My_Memory::MM_malloc(sz*UI32_SIZE));
			capacity_ = sz;
		}
	}
	m_ = m;
	n_ = n;
}

void Bool_Matrix::random(ui32 m, ui32 n, float d, unsigned seed) {
	reserve(m,n);
	My_Memory::MM_memset(data_, 0, m*row_size()*UI32_SIZE);
	ui32 threshold = ui32(RAND_MAX * d);
	if (seed == 0)
		seed = static_cast<unsigned>(time(nullptr));
	srand(seed);	
	for (ui32 i = 0; i < m_; ++i) {
		for (ui32 j = 0; j < n_; ++j) {
			if (static_cast<ui32>(rand()) < threshold)
				set(i, j);
		}      
	}
}

/**********************************************************
io functions
**********************************************************/

static void read_get_width_and_check(FILE* p_file, ui32& m, ui32& n) {
	char ch = 0;
	char state = 0;
	ui32 n0 = 0;
	fpos_t pos = 0;
	if (fgetpos(p_file, &pos) != 0)
		throw std::runtime_error(string("read_get_width::") + std::strerror(errno));

	while (state<10) {
		ch = static_cast<char>(fgetc(p_file));
		if (ferror(p_file))
			throw std::runtime_error(string("read_get_width::") + std::strerror(errno));

		switch (state) {
		case 0:
			if (ch == '0' || ch == '1') {
				++n;
				state = 1;
			} else if (ch == ' ') {
				state = 0;//skip
			} else if (ch == EOF || ch == '\n') {
				state = 11;
			} else {
				state = 10;//error
			}
			break;
		case 1:
			if (ch == '0' || ch == '1') {
				++n;
			} else if (ch == ' ') {
				// skip
			} else if (ch == '\n' || ch == EOF) {
				if (m == 0 || n == n0) {
					n0 = n;
					n = 0;
					++m;
					state = 0;
				} else {
					state = 10;
				}
			} else {
				state = 10;
			}
			break;
		default:
			state = 10;
			break;
		}//switch
	}//while
	n = n0;
	if (state == 10)
		throw std::runtime_error("read_get_width::Invalid file format");
	if (fsetpos(p_file, &pos) != 0)
		throw std::runtime_error(string("read_get_width::") + std::strerror(errno));
}

static char skip_space(FILE* p_file) {
	char ch = ' ';
	while (ch == ' ' || ch == '\n') {
		ch = static_cast<char>(fgetc(p_file));
	}
	return ch;
}

void Bool_Matrix::read(FILE* p_file) {
	ui32 m = 0;
	ui32 n = 0;
	read_get_width_and_check(p_file, m, n);
	reserve(m, n);
	My_Memory::MM_memset(data_, 0, m*row_size()*UI32_SIZE);
	for (ui32 i = 0; i < m_; ++i) {
		for (ui32 j = 0; j < n_; ++j) {
			if (skip_space(p_file) == '1')
				set(i, j);
		}
	}

}

void Bool_Matrix::read(const char* file_name) {
	FILE* p_file = fopen(file_name, "r");
	if (p_file == nullptr) {
		throw std::runtime_error(string("Bool_Matrix::read::") + std::strerror(errno));
	}
	try {
		read(p_file);
	} catch (...) {
		fclose(p_file);
		throw;
	}
	fclose(p_file);
}

//void Bool_Matrix::read(const DynamicArray<char>& data, ui32 m, ui32 n) {
//	init(m, n);
//	for (ui32 i = 0; i < m; ++i) {
//		for (ui32 j = 0; j < n; ++j) {
//			if (data[i*n + j])
//				set(i, j);
//		}
//	}
//}

void Bool_Matrix::print(FILE* p_file) const {
	for (ui32 i = 0; i < m_; ++i) {
		for (ui32 j = 0; j < n_; ++j) {
			fputc((at(i, j) != 0) + '0', p_file);
			fputc(' ', p_file);
		}
		fputc('\n', p_file);
	}
	if (ferror(p_file))
		throw std::runtime_error(string("Bool_Matrix::print::") + std::strerror(errno));
}

void Bool_Matrix::print(const char* file_name, const char* mode) const {
	FILE* p_file = fopen(file_name, mode);
	if (p_file == nullptr) {
		throw std::runtime_error(string("Bool_Matrix::print::") + std::strerror(errno));
	}
	try {
		print(p_file);
	} catch (...) {
		fclose(p_file);
		throw;
	}
	fclose(p_file);
}
