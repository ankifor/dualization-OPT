#pragma once
#include <vector>
#include <string>
#include <fstream>

//matrix elements are stored by rows
class Bool_Matrix {
public:
	Bool_Matrix() : data_(NULL), n_(0), m_(0), sz_(0), nb_(0), {
		for (int i = 0; i < 8; ++i) POW2[i] = (1 << i);
	}
	~Bool_Matrix() { clear(); }
	void init(int m, int n);
	void copy(const Bool_Matrix& src);
	void copy(const char* src, int m, int n);
	void move(char** src, int m, int n);
	void clear();
	//clears old data and allocates memory for new matrix
	void read(FILE* pFile);
	void read_bm(FILE* p_file);
	void read(const std::string& file_name);
	void read(const char* file_name);
	void print(FILE* pFile) const;
	void print(const std::string& file_name) const;
	char* get_data() { return data_; }
	//no verification for i and j is done
	//inline char at(int i, int j) const { return at(index(i, j)); }
	//inline char at(int i) const { return (data_[i >> 3] >> (i & 7)) & 1; }
	inline const char* row(int i) const { return &data_[i*nb_]; }
	//inline int index(int i,int j) const {return n_*i+j;}
	//generate random matrix with P(a[i,j]=1)=d
	//void random(int m, int n, float d = 0.5);
	inline int size() const {return sz_;}
	inline int width() const {return n_;}
	inline int height() const {return m_;}
private:
	char* data_;  
	int n_;//matrix width
	int m_;//matrix hight
	int sz_;//size in bytes
	int nb_;//bytes in a row
	unsigned char POW2[8];// = {1,2,4,8,16,32,64,128};
};
