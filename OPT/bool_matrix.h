#pragma once
#include <vector>
#include <string>

//matrix elements are stored by rows
class Bool_Matrix {
public:
	//no verification for i and j is done
	char at(int i, int j) const throw();
	void set(int i, int j, char value = 1) throw();
	char* row(int i) throw();
	void copy(const Bool_Matrix& src);
	void swap(Bool_Matrix& src) throw();

	void init(int m, int n);
	
	void copy_and_transpose(const Bool_Matrix& src);
	
	void clear() throw();
	//reading data
	void read(const std::vector<char>& data, int m, int n);
	void read(FILE* pFile);	
	void read(const std::string& file_name);
	void read(const char* file_name);
	//printing data
	void print(const std::string& file_name, const char* mode = "w") const;
	void print(FILE* pFile) const;
	//generate random matrix with P(a[i,j]=1)=d
	void random(int m, int n, float d = 0.5, unsigned seed = 0);
	inline int size() const {return sz_;}
	inline int width() const {return n_;}
	inline int height() const {return m_;}

	Bool_Matrix(int m, int n);
	Bool_Matrix(int n);
	Bool_Matrix();
	~Bool_Matrix() throw();
protected:
	char* data_;  
	int n_;//matrix width
	int m_;//matrix hight
	int sz_;//size in bytes
	int nb_;//bytes in a row
};
