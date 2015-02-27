#pragma once
#include <cstdio>
#include "my_int.h"
#include "stack_array.h"
#include "binary.h"

class Dualizer_OPT {
protected:

	class Covering {
	public:

		void reserve(ui32 size) {
			data_.reserve(size);
			text_.reserve(size * 4);
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

		void print(FILE* p_file) {
			if (p_file == nullptr)
				return;
			assert(text_.size() > 0);
			text_.top() = '\n';
			text_.push('\0');
			fputs(text_.get_data(), p_file);
			text_.pop();
			text_.top() = ' ';
		}

		ui32 operator[] (ui32 ind) const throw() { return data_[ind]; }
		
		ui32 size() const throw() { return data_.size(); }

	private:

		Stack_Array<ui32> data_;
		Stack_Array<char> text_;
	};

public:

	Dualizer_OPT() throw() : p_file(nullptr), n_coverings(0) {}
	~Dualizer_OPT() throw() { clear(); }

	void init(const binary::Matrix& L0, const char* file_name = nullptr, const char* mode = "w");
	void clear() throw();
	void run();

protected:

	void update_covered_and_support_rows(ui32* rows, ui32* covered_rows,
		ui32* support_rows, const ui32* col_j) const throw();

	void delete_zero_cols(const ui32* rows, ui32* cols) const throw();
	
	void delete_fobidden_cols(const ui32* one_sums, 
		ui32* cols, const Covering& cov) const throw();
	
	void delete_le_rows(ui32* rows, const ui32* cols) const throw();

private:

	Dualizer_OPT(Dualizer_OPT const&) {};
	void operator = (Dualizer_OPT const&) {};

	ui32 m() const throw() { return L.height(); }
	ui32 size_m() const throw() { return binary::size(m()); }
	ui32 mask_m() const throw() { return binary::mask(m()); }

	ui32 n() const throw() { return L.width(); }
	ui32 size_n() const throw() { return binary::size(n()); }
	ui32 mask_n() const throw() { return binary::mask(n()); }

private:

	binary::Matrix L;
	binary::Matrix L_t;

	ui32 n_coverings;
	FILE* p_file;
};