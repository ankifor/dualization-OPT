#include <intrin.h>//for _bittest, _bittestandset, _bittestandreset, _tzcnt_u32, __popcnt
#include <cstdio>//for fgetc, EOF, fopen, fclose, fputc
#include <stdexcept>//for runtime_error
//#include <time.h>//for time()
//#include <cstdlib>//for rand
//#include <algorithm>//for std::swap

#include "binary.h"
#include "my_memory.h"

using namespace std;

ui32 binary::popcount(const ui32* p, ui32 bitsize) {
		assert(bitsize > 0);
		ui32 sum = 0;
		ui32 sz = size(bitsize);

		for (ui32 ind = 0; ind < sz - 1; ++ind) {
			sum += __popcnt(p[ind]);
		}
		sum += __popcnt(p[sz - 1] & mask(bitsize));

		return sum;
}

ui32 binary::find_next(const ui32* p, ui32 bitsize, ui32 bit) {
    assert(bitsize > 0);		
    ui32 ind = bit >> UI32_LOG2BIT;
    ui32 offset = bit & UI32_MASK;

    ui32 buf = (p[ind] >> offset) << offset;
    if (buf != 0) {
			offset = _tzcnt_u32(buf);
		} else {
			do {
				++ind;
			} while (ind < size(bitsize) && p[ind] == 0);
			offset = _tzcnt_u32(p[ind]);//UI32_BITS==32
		}
    return (ind << UI32_LOG2BIT) + offset;

//	offset = _tzcnt_u32(buf);//UI32_BITS==32


}

bool binary::any(const ui32* p, ui32 bitsize) {
	assert(bitsize > 0);

	ui32 sz = size(bitsize);

	ui32 buf = p[sz - 1];
	const_cast<ui32*>(p)[sz - 1] &= mask(bitsize);

	//res = 
	//	(*p != 0) || 
	//	(My_Memory::MM_memcmp(p, p + 1, UI32_SIZE*(sz - 1)) != 0);
	ui32 res = 0;
	for (ui32 ind = 0; ind < sz; ++ind) {
		res |= p[ind];
	}
	const_cast<ui32*>(p)[sz - 1] = buf;
	return res != 0;
}

bool binary::all(const ui32* p, ui32 bitsize) {
	assert(bitsize > 0);

	ui32 sz = size(bitsize);

	ui32 buf = p[sz - 1];
	const_cast<ui32*>(p)[sz - 1] |= ~mask(bitsize);

	ui32 res = UI32_ALL;
	for (ui32 ind = 0; ind < sz; ++ind) {
		res &= p[ind];
	}

	const_cast<ui32*>(p)[sz - 1] = buf;
	return res == UI32_ALL;
}

void binary::set(ui32* p, ui32 bit) {
	ui32 ind = (bit >> UI32_LOG2BIT);
	_bittestandset(reinterpret_cast<long*>(p + ind), bit & UI32_MASK);
}

void binary::reset(ui32* p, ui32 bit) {
	ui32 ind = (bit >> UI32_LOG2BIT);
	_bittestandreset(reinterpret_cast<long*>(p + ind), bit & UI32_MASK);
}

char binary::at(const ui32* p, ui32 bit) {
	ui32 ind = bit >> UI32_LOG2BIT;
	return _bittest(reinterpret_cast<const long*>(p + ind), bit & UI32_MASK);
}

void binary::reset_le(ui32* p, ui32 bit) {
	ui32 k = (bit + 1) >> UI32_LOG2BIT;
	My_Memory::MM_memset(p, 0, k*UI32_SIZE);
	ui32 offset = (bit + 1) & UI32_MASK;
	ui32 mask = UI32_ALL << offset;
	p[k] &= mask;
}

ui32* binary::transpose(const ui32* src, ui32 m, ui32 n) {
	ui32 size_byte = size(m) * n * UI32_SIZE;
	ui32* dst = static_cast<ui32*>(My_Memory::MM_malloc(size_byte));
	My_Memory::MM_memset(dst, 0, size_byte);

	for (ui32 i = 0; i < m; ++i) {
		for (ui32 j = 0; j < n; ++j) {
			if (at(src + i * size(n), j))
				set(dst + j * size(m), i);
		}
	}

	return dst;
}

ui32* binary::submatrix(const ui32* src, const ui32* rows, ui32& m, const ui32& n) {
	ui32 m_new = popcount(rows, m);
	ui32* res = static_cast<ui32*>(My_Memory::MM_malloc(m_new * size(n) * UI32_SIZE));
	
	ui32 i = find_next(rows, m, 0);	
	ui32* dst = res;
	while (i < m) {		
		My_Memory::MM_memcpy(dst, src + i*size(n), size(n)*UI32_SIZE);
		dst += size(n);
		i = find_next(rows, m, i + 1);
	}

	m = m_new;
	return res;
}

/**********************************************************
service functions
**********************************************************/

void binary::Matrix::copy(const Matrix& src) {
	if (this != &src) {
		reserve(src.m_, src.n_);
		My_Memory::MM_memcpy(data_, src.data_, m_*row_size()*UI32_SIZE);
	}
}

//void binary::Matrix::swap(Matrix& src) {
//	if (this != &src) {
//		std::swap(m_, src.m_);
//		std::swap(n_, src.n_);
//		std::swap(data_, src.data_);
//		std::swap(capacity_, src.capacity_);
//	}
//}

void binary::Matrix::reserve(ui32 m, ui32 n) {
	ui32 row_sz = size(n);
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

//void binary::Matrix::random(ui32 m, ui32 n, float d, unsigned seed) {
//	reserve(m,n);
//	My_Memory::MM_memset(data_, 0, m*row_size()*UI32_SIZE);
//	ui32 threshold = ui32(RAND_MAX * d);
//	if (seed == 0)
//		seed = static_cast<unsigned>(time(nullptr));
//	srand(seed);	
//	for (ui32 i = 0; i < m_; ++i) {
//		for (ui32 j = 0; j < n_; ++j) {
//			if (static_cast<ui32>(rand()) < threshold)
//				set(i, j);
//		}      
//	}
//}

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

void binary::Matrix::read(FILE* p_file) {
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

void binary::Matrix::read(const char* file_name) {
	FILE* p_file = fopen(file_name, "r");
	if (p_file == nullptr) {
		throw std::runtime_error(string("binary::Matrix::read::") + std::strerror(errno));
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

void binary::Matrix::print(FILE* p_file) const {
	for (ui32 i = 0; i < m_; ++i) {
		for (ui32 j = 0; j < n_; ++j) {
			fputc((at(i, j) != 0) + '0', p_file);
			fputc(' ', p_file);
		}
		fputc('\n', p_file);
	}
	if (ferror(p_file))
		throw std::runtime_error(string("binary::Matrix::print::") + std::strerror(errno));
}

void binary::Matrix::print0x(FILE* p_file) const {
	for (ui32 i = 0; i < m_; ++i) {
		for (ui32 ind = 0; ind < size(n_); ++ind) {
			fprintf(p_file, "%08x ", data_[i*row_size() + ind]);
		}
		fputc('\n', p_file);
	}
	if (ferror(p_file))
		throw std::runtime_error(string("binary::Matrix::print::") + std::strerror(errno));
}

void binary::Matrix::print(const char* file_name, const char* mode) const {
	FILE* p_file = fopen(file_name, mode);
	if (p_file == nullptr) {
		throw std::runtime_error(string("binary::Matrix::print::") + std::strerror(errno));
	}
	try {
		print(p_file);
	} catch (...) {
		fclose(p_file);
		throw;
	}
	fclose(p_file);
}
