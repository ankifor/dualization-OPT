#pragma once
#include <vector>
#include <string>
#include <cstdint>

//matrix elements are stored by rows
class Bool_Matrix {
public:
	//no verification for i and j is done
	char at(uint32_t i, uint32_t j) const throw();
	void set(uint32_t i, uint32_t j, char value = 1) throw();
	uint32_t* row(uint32_t i) throw();
	void copy(const Bool_Matrix& src);
	void swap(Bool_Matrix& src) throw();

	void init(uint32_t m, uint32_t n);
	
	void copy_and_transpose(const Bool_Matrix& src);
	
	void clear() throw();
	//reading data
	void read(const std::vector<char>& data, uint32_t m, uint32_t n);
	void read(FILE* pFile);	
	void read(const std::string& file_name);
	void read(const char* file_name);
	//printing data
	void print(const std::string& file_name, const char* mode = "w") const;
	void print(FILE* pFile) const;
	//generate random matrix with P(a[i,j]=1)=d
	void random(uint32_t m, uint32_t n, float d = 0.5, unsigned seed = 0);
	inline uint32_t size32() const {return sz32_;}
	inline uint32_t width() const {return n_;}
	inline uint32_t height() const {return m_;}

	Bool_Matrix(uint32_t m, uint32_t n);
	Bool_Matrix(uint32_t n);
	Bool_Matrix();
	~Bool_Matrix() throw();
protected:
	uint32_t* data_;  
	uint32_t n_;//matrix width
	uint32_t m_;//matrix hight
	uint32_t sz32_;//size in bytes
	uint32_t n32_;//uints32 in a row
};
