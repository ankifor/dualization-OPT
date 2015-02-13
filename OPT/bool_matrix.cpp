#include <time.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <stdexcept>
#include "bool_matrix.h"

using namespace std;

static void read_into_vector(FILE* p_file, vector<char>& buffer, int& m, int& n) {
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
				if (m == 0) n++;
					state = 1;
				} else if (ch == ' ') {
					state = 2;//skip
				} else if (ch == '\n') {
				m++;
					state = 0;
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

void Bool_Matrix::init(int m, int n) {
	m_ = m;
	n_ = n;
	nb_ = (n + 7) / 8;
	int sz_old = sz_;
	sz_ = m*nb_;
	if (data_ == NULL) {
		data_ = static_cast<char*>(malloc(sz_));
	} else if (sz_ >= sz_old) {
		data_ = static_cast<char*>(realloc(data_, sz_));
	}
	if (data_ == NULL)
		throw std::runtime_error("Allocation memory error");
	memset(data_, 0, sz_);
}

void Bool_Matrix::clear() {
	if (data_)
		free(data_);
	data_ = NULL;
	m_ = 0;
	n_ = 0;
	sz_ = 0;
}

void Bool_Matrix::copy(const Bool_Matrix& src) {
	init(src.m_,src.n_);
	memcpy(data_,src.data_,sz_);
}

void Bool_Matrix::copy(const char* src, int m, int n) {
	init(m,n);
	memcpy(data_,src,sz_);
}

void Bool_Matrix::move(char** src, int m, int n) {
	clear();
	m_=m; n_=n; sz_ = (m*n+7)/8;
	data_ = *src; *src = NULL;
}


void Bool_Matrix::read(const string& file_name) {
	FILE* p_file = fopen(file_name.c_str(),"r");
	if (p_file == NULL) {
		throw std::runtime_error(std::strerror(errno));
	}
	read(p_file);
	fclose(p_file);
}

void Bool_Matrix::read(const char* file_name) {
	FILE* p_file = fopen(file_name,"r");
	if (p_file == NULL) {
		throw std::runtime_error(std::strerror(errno));
	}
	read(p_file);
	fclose(p_file);
}

void Bool_Matrix::print(const string& file_name, const char* mode) const {
	FILE* p_file = fopen(file_name.c_str(), mode);
	if (p_file == NULL) {
		throw std::runtime_error(std::strerror(errno));
	}
	print(p_file);
	fclose(p_file);
}

void Bool_Matrix::print(FILE* p_file) const {
	for (int i = 0; i < m_; ++i) {
		for (int j = 0; j < n_; ++j) {
			fputc((at(i, j) != 0) + '0', p_file);
			fputc(' ', p_file);
		}
		fputc('\n', p_file);
	}
}

void Bool_Matrix::random(int m, int n, float d, unsigned seed) {
	init(m,n);
	int threshold = int(RAND_MAX * d);
	char tmp = 0;
	if (seed == 0)
		seed = time(NULL);
	srand(seed);	
	for (int i = 0; i < m_; ++i) {
		for (int j = 0; j < n_; ++j) {
			tmp = (rand() < threshold ? 1 : 0);
			set(i, j, tmp);
		}      
	}
}

void Bool_Matrix::read(FILE* p_file) {
	vector<char> buffer;
	int m = 0;
	int n = 0;
	read_into_vector(p_file, buffer, m, n);
	read(buffer, m, n);
}

void Bool_Matrix::read(const std::vector<char>& data, int m, int n) {
	init(m, n);
	char tmp = 0;
	for (int i = 0; i < m; ++i) {
		for (int j = 0; j < n; ++j) {
			tmp = data[i*n + j];
			set(i, j, tmp);
		}
	}
}

void Bool_Matrix::copy_and_transpose(const Bool_Matrix& src) {
	init(src.n_, src.m_);
	char tmp = 0;
	for (int i = 0; i < src.m_; ++i) {
		for (int j = 0; j < src.n_; ++j) {
			tmp = src.at(i, j);
			set(j, i, tmp);
		}
	}
}