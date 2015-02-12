#include <time.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "bool_matrix.h"

using namespace std;

void Bool_Matrix::init(int m, int n) {
	m_ = m;
	n_ = n;
	nb_ = (n + 7) / 8;
	sz_ = m*nb_;
	data_ = (char*)calloc(sz_, 1);
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
  clear();
  init(src.m_,src.n_);
  memcpy(data_,src.data_,sz_);
}

void Bool_Matrix::copy(const char* src, int m, int n) {
  clear();
  init(m,n);
  memcpy(data_,src,sz_);
}

void Bool_Matrix::move(char** src, int m, int n) {
  clear();
  m_=m; n_=n; sz_ = (m*n+7)/8;
  data_ = *src; *src = NULL;
}



void Bool_Matrix::read(const string& file_name) {
  FILE* pFile = fopen(file_name.c_str(),"r");
  read(pFile);
  fclose(pFile);
}

void Bool_Matrix::read(const char* file_name) {
  FILE* pFile = fopen(file_name,"r");
  read(pFile);
  fclose(pFile);
}

void Bool_Matrix::print(FILE* pFile) const {
  fprintf(pFile, "%d %d\n",m_,n_);
  for (int i = 0; i < m_; ++i) {
    for (int j = 0; j < n_; ++j) {
      fputc((at(i,j)!=0)+'0',pFile);
      fputc(' ',pFile);
    }
    fputc('\n',pFile);
  }
}

void Bool_Matrix::print(const string& file_name) const {
  FILE* pFile = fopen(file_name.c_str(),"w");
  print(pFile);
  fclose(pFile);
}

void Bool_Matrix::random(int m, int n, float d) {
  clear();
  init(m,n);
  int threshold = int(RAND_MAX * d);
  srand(time(NULL));
  for (int i = 0; i < sz_; ++i) {
    for (int j = 0; j < 8; ++j)
      if (rand() < threshold) data_[i] |= POW2[j];
  }
}

void Bool_Matrix::read(FILE* pFile) {
	clear();
	fscanf(pFile, "%d %d\r\n", &m_, &n_);
	init(m_, n_);
	int by = 0;//byte num
	int bi = 0;//bit num
	int tmp;
	for (int i = 0; i < m_; ++i) {
		for (int j = 0; j < n_; ++j) {
			int ind = index(i, j);
			by = (ind >> 3);
			bi = (ind & 7);
			fscanf(pFile, "%d", &tmp);
			if (tmp == 1)
				data_[by] |= POW2[bi];

		}
		fscanf(pFile, "%*[ \t\v\f\r\n]");
	}
}

void Bool_Matrix::read_bm(FILE* p_file) {
	char ch = 0;
	char state = 0;
	int width = 0;
	int height = 0;
	vector<char> buffer;
	while (state<10) {
		ch = fgetc(p_file);
		switch (state) {
		case 0:
			if (ch == '0' || ch == '1') {
				buffer.push_back(ch - '0');
				if (height == 0) width++;
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
				height++;
				state = 0;
			} else {
				state = 10;
			} 
			break;
		case 2:
			if (ch == '0' || ch == '1') {
				buffer.push_back(ch - '0');
				if (height == 0) width++;
				state = 1;
			} else if (ch == ' ') {
				state = 2;//skip
			} else if (ch == '\n') {
				height++;
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
	if (buffer.size() != width*height) {
		state = 10;
	}
	assert(state != 10);
}