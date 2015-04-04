#pragma once
#include <cstdio>
#include "my_int.h"
#include "stack_array.h"
#include "binary.h"

class Dualizer_OPT {
protected:

	class Covering {
	public:

		void reserve(ui32 size, ui32 width) {
			data_.reserve(size);
			text_.reserve(size * 4);
			frequency_ = (ui32*) My_Memory::MM_malloc(width*UI32_SIZE);
			My_Memory::MM_memset(frequency_, 0, width*UI32_SIZE);
		}

		~Covering() {
			if (frequency_ != nullptr) {
				My_Memory::MM_free(frequency_);
			}
		}

		void append(ui32 num) {
			assert(num <= 99999);

			data_.push(num);

			char buf[5];
			ui32 len = 0;

			do {
				buf[len] = '0' + num % 10;
				num /= 10;
				++len;
			} while (num > 0);

			do {
#pragma warning(suppress: 6385)
				text_.push(buf[--len]);
			} while (len > 0);

			text_.push(' ');
		}

		void remove_last() {
			assert(data_.size() > 0);
			data_.pop();
			do {
				text_.pop();
			} while (text_.size() > 0 && text_.top() != ' ');
		}

		ui32& top() {
			return data_.top();
		}

		void print(FILE* p_file) {
			if (p_file == nullptr) {
				++frequency_[data_[0]];
			} else {
				assert(text_.size() > 0);
				text_.top() = '\n';
				//text_.push('\0');
				//fputs(text_.get_data(), p_file);
				fwrite(text_.get_data(), 1, text_.size(), p_file);
				//text_.pop();
				text_.top() = ' ';
			}
		}

		ui32 operator[] (ui32 ind) const throw() { return data_[ind]; }
		
		ui32 size() const throw() { return data_.size(); }

		void print_freq(ui32 width) {
			for (ui32 i = 0; i < width; ++i) {
				printf("%d ", frequency_[i]);
			}
			printf("\n");
		}

		ui32* get_freq() {
			return frequency_;
		}
	private:

		Stack_Array<ui32> data_;
		Stack_Array<char> text_;
		ui32* frequency_ = nullptr;

	};

public:

	Dualizer_OPT() { My_Memory::MM_memset(this, 0, sizeof(Dualizer_OPT)); }
	~Dualizer_OPT() { clear(); }

	//preprocesses matrix, allocates memory, set matrix_, matrix_t_, m_, n_, etc
	void init(const binary::Matrix& L, const char* file_name = nullptr, const char* mode = "wb");
	void clear() throw();
	void run();
	void print() {
		printf("Irreducible coverings: %d\n", n_coverings);
		if (p_file == nullptr)
			covering.print_freq(n());
	}
	ui32* get_freq() { return covering.get_freq(); }

protected:

	void update_covered_and_support_rows(ui32 j) throw();

	void delete_zero_cols() throw();
	
	void delete_fobidden_cols()  throw();
	void delete_fobidden_cols1() throw();
	void delete_fobidden_cols2() throw();
	void delete_fobidden_cols3() throw();
	
	void delete_le_rows() throw();

	//char create_search_set(ui32* set) throw();

private:

	Dualizer_OPT(Dualizer_OPT const&) {};
	void operator = (Dualizer_OPT const&) {};

	inline ui32 m() const throw() { return m_; }
	inline ui32 size32_m() const throw() { return binary::size(m_); }
	inline ui32 size64_m() const throw() { return binary::size64(m_); }
	inline ui32 mask32_m() const throw() { return binary::mask(m_); }
	inline ui64 mask64_m() const throw() { return binary::mask64(m_); }

	inline ui32 n() const throw() { return n_; }
	inline ui32 size32_n() const throw() { return binary::size(n_); }
	inline ui32 size64_n() const throw() { return binary::size64(n_); }
	inline ui32 mask32_n() const throw() { return binary::mask(n_); }
	inline ui64 mask64_n() const throw() { return binary::mask64(n_); }

private:
	ui32* pool_;

	ui32* matrix_;
	ui32* cols;
	ui32* rows;
	ui32* support_rows;
	ui32* p_j;
	ui32* matrix_t_;
	ui32* cov;	
	Covering covering;

	ui32 m_;
	ui32 n_;
	ui32 n_coverings;
	FILE* p_file;
	char* file_buffer_;
};