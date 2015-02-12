#pragma once
#include <vector>
#include <string>

//matrix elements are stored by rows
class Bool_Matrix {
public:
	Bool_Matrix() : data_(NULL), n_(0), m_(0), sz_(0), nb_(0) {	}
	~Bool_Matrix() { clear(); }
	void init(int m, int n);
	void copy(const Bool_Matrix& src);
	void copy(const char* src, int m, int n);
	void move(char** src, int m, int n);
	void clear();
	//reading data
	void read(const std::vector<char>& data, int m, int n);
	void read(FILE* pFile);	
	int read(const std::string& file_name);
	int read(const char* file_name);
	//printing data
	int print(const std::string& file_name, const char* mode = "w") const;
	void print(FILE* pFile) const;
	//no verification for i and j is done
	inline char at(int i, int j) const { return (data_[i*nb_+j/8] >> (j & 0x7)) & 1; }
	inline char at(int i, int j) const { return (data_[i*nb_+j/8] >> (j & 0x7)) & 1; }
	inline const char* row(int i) const { return &data_[i*nb_]; }
	//generate random matrix with P(a[i,j]=1)=d
	void random(int m, int n, float d = 0.5, unsigned seed = 0);
	inline int size() const {return sz_;}
	inline int width() const {return n_;}
	inline int height() const {return m_;}
private:
	char* data_;  
	int n_;//matrix width
	int m_;//matrix hight
	int sz_;//size in bytes
	int nb_;//bytes in a row
};
