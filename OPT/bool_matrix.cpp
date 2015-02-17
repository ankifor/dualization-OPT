#include <time.h>//for time()
#include <cstdio>//for fgetc, EOF, fopen, fclose, fputc
#include <cstdlib>//for rand
#include <memory.h>//for memset, memcpy
#include <stdexcept>//for std::runtime_error
#include <intrin.h>//for _tzcnt_u32, __popcnt, _bittest, _bittestandset, _bittestandreset
#include "bool_matrix.h"


using namespace std;

ui32 Bool_Matrix::find_next(ui32 j) const {
	ui32 ind = j >> LOG2BIT;
	ui32 offset = j & MASK;
	ui32 buf = (data_[ind] >> offset) << offset;

	while (ind < n32_) {
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

ui32 Bool_Matrix::popcount() const throw() {
	ui32 sum = 0;
	ui32 ind = 0;
	for (ind = 0; ind < n32_-1; ++ind) {
		sum += __popcnt(data_[ind]);
	}
	ui32 mask = ALL >> (BITS - (n_ & MASK));
	sum += __popcnt(data_[ind] & mask);
	return sum;
}

char Bool_Matrix::at(ui32 i, ui32 j) const {
	ui32 ind = i*n32_ + (j >> LOG2BIT);
	return _bittest(reinterpret_cast<const long*>(data_+ind), j & MASK);
	//return (data_[ind] >> (j & MASK)) & 1;
}

char Bool_Matrix::at(ui32 j) const {
	ui32 ind = (j >> LOG2BIT);
	return _bittest(reinterpret_cast<const long*>(data_ + ind), j & MASK);
	//return (data_[ind] >> (j & MASK)) & 1;
}

void Bool_Matrix::set(ui32 i, ui32 j) {
	ui32 ind = i*n32_ + (j >> LOG2BIT);
	_bittestandset(reinterpret_cast<long*>(data_ + ind), j & MASK);
	//data_[ind] = (data_[ind] & ~POW2(j & MASK)) | (1 << (j & MASK));
	//data_[ind] |= (value << (j & 0x7)); 
}

void Bool_Matrix::set(ui32 j) {
	ui32 ind = (j >> LOG2BIT);
	_bittestandset(reinterpret_cast<long*>(data_ + ind), j & MASK);
}

void Bool_Matrix::reset(ui32 i, ui32 j) {
	ui32 ind = i*n32_ + (j >> LOG2BIT);
	_bittestandreset(reinterpret_cast<long*>(data_ + ind), j & MASK);
}

void Bool_Matrix::reset(ui32 j) {
	ui32 ind = (j >> LOG2BIT);
	_bittestandreset(reinterpret_cast<long*>(data_ + ind), j & MASK);
}

ui32* Bool_Matrix::row(ui32 i) {
	return &data_[i*n32_];
}

const ui32* Bool_Matrix::row(ui32 i) const {
	return &data_[i*n32_];
}

void Bool_Matrix::copy(const Bool_Matrix& src) {
	if (this != &src) {
		init(src.m_, src.n_);
		memcpy(data_, src.data_, sz32_*SIZE);
	}
}

void Bool_Matrix::swap(Bool_Matrix& src) {
	if (this != &src) {
		ui32 m = m_;
		ui32 n = n_;
		ui32 sz32 = sz32_;
		ui32* data = data_;
		m_ = src.m_;
		n_ = src.n_;
		sz32_ = src.sz32_;
		data_ = src.data_;
		src.m_ = m;
		src.n_ = n;
		src.sz32_ = sz32;
		src.data_ = data;
	}
}


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
			if (ch == ' ') {
				state = 2;
			} else if (ch == '\n') {
				if (m == 0 || n == n0) {
					n0 = n;
					n = 0;
					++m;
					state = 0;
				} else {
					state = 10;
				}
			} else if (ch == EOF) {
				if (m == 0 || n == n0) {
					n0 = n;
					n = 0;
					++m;
					state = 11;
				} else {					
					state = 10;
				}
			} else {
				state = 10;
			}
			break;
		case 2:
			if (ch == '0' || ch == '1') {
				++n;
				state = 1;
			} else if (ch == ' ') {
				state = 2;//skip
			} else if (ch == '\n') {
				if (m == 0 || n == n0) {
					n0 = n;
					n = 0;
					++m;
					state = 0;
				} else {
					state = 10;
				}
			} else if (ch == EOF) {
				if (m == 0 || n == n0) {
					n0 = n;
					n = 0;
					++m;
					state = 11;
				} else {
					state = 10;
				}
			} else {
				state = 10;//error
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

void Bool_Matrix::init(ui32 m, ui32 n, char value) {
	m_ = m;
	n_ = n;
	n32_ = (n + BITS - 1) / BITS;
	ui32 sz32_old = sz32_;
	sz32_ = m*n32_;
	if (data_ == nullptr) {
		data_ = static_cast<ui32*>(malloc(sz32_*SIZE));
	} else if (sz32_ >= sz32_old) {
		data_ = static_cast<ui32*>(realloc(data_, sz32_*SIZE)); //-V701
	}
	if (data_ == nullptr)
		throw std::runtime_error("Bool_Matrix::init::Allocation memory error");
	memset(data_, value, sz32_*SIZE);
}

void Bool_Matrix::clear() {
	if (data_ != nullptr) {
		free(data_);
		data_ = nullptr;
	}
	m_ = 0;
	n_ = 0;
	sz32_ = 0;
}

void Bool_Matrix::read(const char* file_name) {
	FILE* p_file = fopen(file_name,"r");
	if (p_file == nullptr) {
		throw std::runtime_error(string("Bool_Matrix::read::")+std::strerror(errno));
	}
	try {
		read(p_file);
	} catch (...) {
		fclose(p_file);
		throw;
	}
	fclose(p_file);
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

void Bool_Matrix::random(ui32 m, ui32 n, float d, unsigned seed) {
	init(m,n);
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
	init(m, n);

	for (ui32 i = 0; i < m_; ++i) {
		for (ui32 j = 0; j < n_; ++j) {
			if (skip_space(p_file) == '1')
				set(i, j);
		}
	}

}

void Bool_Matrix::read(const DynamicArray<char>& data, ui32 m, ui32 n) {
	init(m, n);
	for (ui32 i = 0; i < m; ++i) {
		for (ui32 j = 0; j < n; ++j) {
			if (data[i*n + j])
				set(i, j);
		}
	}
}

void Bool_Matrix::copy_and_transpose(const Bool_Matrix& src) {
	init(src.n_, src.m_);
	for (ui32 i = 0; i < src.m_; ++i) {
		for (ui32 j = 0; j < src.n_; ++j) {
			if (src.at(i, j))
				set(j, i);
		}
	}
}



Bool_Matrix::Bool_Matrix(ui32 m, ui32 n) : data_(nullptr), n_(0), m_(0), sz32_(0), n32_(0) {
	init(m, n);
}

Bool_Matrix::Bool_Matrix(ui32 n) : data_(nullptr), n_(0), m_(0), sz32_(0), n32_(0) {
	init(1, n);
}

Bool_Matrix::Bool_Matrix(const Bool_Matrix& src) : data_(nullptr), n_(0), m_(0), sz32_(0), n32_(0) {
	copy(src);
}

Bool_Matrix::Bool_Matrix(const Bool_Matrix& src, const Bool_Matrix& rows) 
	: data_(nullptr), n_(0), m_(0), sz32_(0), n32_(0) 
{
	init(rows.popcount(), src.n_);
	for (ui32 j = rows.find_next(0), k = 0; j < rows.width(); j = rows.find_next(j + 1), ++k) {
		memcpy(row(k), src.row(j), n32_*SIZE);
	}
}

Bool_Matrix& Bool_Matrix::operator= (const Bool_Matrix& src) {
	copy(src);
	return *this;
}

Bool_Matrix::Bool_Matrix() : data_(nullptr), n_(0), m_(0), sz32_(0), n32_(0) {
}

Bool_Matrix::~Bool_Matrix() {
	clear();
}


