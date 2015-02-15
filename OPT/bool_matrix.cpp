#include <time.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <stdexcept>
#include "bool_matrix.h"

using namespace std;

#define _POW2(x) 1<<(x)
#define _POW2_10(x) _POW2(x),_POW2(x+1),_POW2(x+2),_POW2(x+3),_POW2(x+4),_POW2(x+5),_POW2(x+6),_POW2(x+7),_POW2(x+8),_POW2(x+9)
static const char POW2[32] = { _POW2_10(0), _POW2_10(10), _POW2_10(20), _POW2(30), _POW2(31) };
#undef _POW2_10
#undef _POW2

const static uint32_t BITS = 32;
const static uint32_t LOG2BITS = 5;
const static uint32_t MASK = BITS-1;
const static size_t SIZE = 4;//sizeof(uint32_t)

char Bool_Matrix::at(uint32_t i, uint32_t j) const {
	uint32_t ind = i*n32_ + (j / BITS);
	return (data_[ind] >> (j & MASK)) & 1;
}

void Bool_Matrix::set(uint32_t i, uint32_t j, char value) {
	uint32_t ind = i*n32_ + (j / BITS);
	data_[ind] = (data_[ind] & ~POW2[j & MASK]) | (value << (j & MASK));
	//data_[ind] |= (value << (j & 0x7)); 
}

uint32_t* Bool_Matrix::row(uint32_t i) {
	return & data_[i*n32_];
}

void Bool_Matrix::copy(const Bool_Matrix& src) {
	init(src.m_, src.n_);
	memcpy(data_, src.data_, sz32_*SIZE);
}

void Bool_Matrix::swap(Bool_Matrix& src) {
	uint32_t m = m_;
	uint32_t n = n_;
	uint32_t sz32 = sz32_;
	uint32_t* data = data_;
	m_ = src.m_;
	n_ = src.n_;
	sz32_ = src.sz32_;
	data_ = src.data_;
	src.m_ = m;
	src.n_ = n;
	src.sz32_ = sz32;
	src.data_ = data;
}

static void read_into_vector(FILE* p_file, vector<char>& buffer, uint32_t& m, uint32_t& n) {
	char ch = 0;
	char state = 0;
	n = 0;
	m = 0;
	while (state<10) {
		ch = fgetc(p_file);
		switch (state) {
			case 0:
				if (ch == '0' || ch == '1') {
					buffer.push_back(ch - '0');
					if (m == 0) 
						n++;
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
					m++;
					state = 0;
				} else {
					state = 10;
				}
				break;
			case 2:
				if (ch == '0' || ch == '1') {
					buffer.push_back(ch - '0');
					if (m == 0) 
						n++;
					state = 1;
				} else if (ch == ' ') {
					state = 2;//skip
				} else if (ch == '\n') {
					m++;
					state = 0;
				} else if (ch == EOF) {
					m++;
					state = 11;
				} else {
					state = 10;//error
				}
				break;
			default:
				state = 10;
				break;
		}//switch
	}//while
	if (buffer.size() != n*m)
		state = 10;
	if (state == 10)
		throw std::runtime_error("Invalid file format");
}

void Bool_Matrix::init(uint32_t m, uint32_t n) {
	m_ = m;
	n_ = n;
	n32_ = (n + BITS - 1) / BITS;
	uint32_t sz32_old = sz32_;
	sz32_ = m*n32_;
	if (data_ == nullptr) {
		data_ = static_cast<uint32_t*>(malloc(sz32_*SIZE));
	} else if (sz32_ >= sz32_old) {
		data_ = static_cast<uint32_t*>(realloc(data_, sz32_*SIZE));
	}
	if (data_ == nullptr)
		throw std::runtime_error("Allocation memory error");
	//memset(data_, 0, sz32_*SIZE);
}

void Bool_Matrix::clear() {
	if (data_ == nullptr) {
		free(data_);
		data_ = nullptr;
	}
	m_ = 0;
	n_ = 0;
	sz32_ = 0;
}


void Bool_Matrix::read(const string& file_name) {
	FILE* p_file = fopen(file_name.c_str(),"r");
	if (p_file == nullptr) {
		throw std::runtime_error(std::strerror(errno));
	}
	read(p_file);
	fclose(p_file);
}

void Bool_Matrix::read(const char* file_name) {
	FILE* p_file = fopen(file_name,"r");
	if (p_file == nullptr) {
		throw std::runtime_error(std::strerror(errno));
	}
	read(p_file);
	fclose(p_file);
}

void Bool_Matrix::print(const string& file_name, const char* mode) const {
	FILE* p_file = fopen(file_name.c_str(), mode);
	if (p_file == nullptr) {
		throw std::runtime_error(std::strerror(errno));
	}
	print(p_file);
	fclose(p_file);
}

void Bool_Matrix::print(FILE* p_file) const {
	uint32_t count = 0;
	for (uint32_t i = 0; i < m_; ++i) {
		for (uint32_t j = 0; j < n_; ++j) {
			count += at(i, j);
			fputc((at(i, j) != 0) + '0', p_file);
			fputc(' ', p_file);
		}
		fprintf(p_file,"\n%d", count);
		fputc('\n', p_file);
	}
}

void Bool_Matrix::random(uint32_t m, uint32_t n, float d, unsigned seed) {
	init(m,n);
	uint32_t threshold = uint32_t(RAND_MAX * d);
	char tmp = 0;
	if (seed == 0)
		seed = static_cast<unsigned>(time(nullptr));
	srand(seed);	
	for (uint32_t i = 0; i < m_; ++i) {
		for (uint32_t j = 0; j < n_; ++j) {
			tmp = (static_cast<uint32_t>(rand()) < threshold ? 0x1 : 0x0);
			set(i, j, tmp);
		}      
	}
}

void Bool_Matrix::read(FILE* p_file) {
	vector<char> buffer;
	uint32_t m = 0;
	uint32_t n = 0;
	read_into_vector(p_file, buffer, m, n);
	read(buffer, m, n);
}

void Bool_Matrix::read(const std::vector<char>& data, uint32_t m, uint32_t n) {
	init(m, n);
	char tmp = 0;
	for (uint32_t i = 0; i < m; ++i) {
		for (uint32_t j = 0; j < n; ++j) {
			tmp = data[i*n + j];
			set(i, j, tmp);
		}
	}
}

void Bool_Matrix::copy_and_transpose(const Bool_Matrix& src) {
	init(src.n_, src.m_);
	char tmp = 0;
	for (uint32_t i = 0; i < src.m_; ++i) {
		for (uint32_t j = 0; j < src.n_; ++j) {
			tmp = src.at(i, j);
			set(j, i, tmp);
		}
	}
}

Bool_Matrix::Bool_Matrix(uint32_t m, uint32_t n) : data_(nullptr), n_(0), m_(0), sz32_(0), n32_(0) {
	init(m, n);
	memset(data_, 0, sz32_*SIZE);
}

Bool_Matrix::Bool_Matrix(uint32_t n) : data_(nullptr), n_(0), m_(0), sz32_(0), n32_(0) {
	init(1, n);
	memset(data_, 0, sz32_*SIZE);
}

Bool_Matrix::Bool_Matrix() : data_(nullptr), n_(0), m_(0), sz32_(0), n32_(0) {
}

Bool_Matrix::~Bool_Matrix() {
	clear();
}