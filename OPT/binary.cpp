
#if defined(_MSC_VER)
#include <intrin.h>//for __popcnt64, _tzcnt_u64
#define popcnt32 __popcnt
#define tzcnt32 _tzcnt_u32
#elif defined(__IBMC__) || defined(__IBMCPP__)
#include <builtins.h>
#define popcnt32 __popcnt4
#define tzcnt32 __cnttz4
#endif

#include <cstdio>//for fgetc, EOF, fopen, fclose, fputc
#include <stdexcept>//for runtime_error
#include <cstdlib>//for rand
#include <algorithm>//for random_shuffle
#include <vector>//for vector
#include <cerrno>

#include "binary.h"
#include "my_memory.h"

using namespace std;

ui32 binary::popcount(const ui32* p, ui32 bitsize) {
		assert(bitsize > 0);
		ui32 sum = 0;
		ui32 sz = size(bitsize);

		for (ui32 ind = 0; ind < sz - 1; ++ind) {
			sum += popcnt32(p[ind]);
		}
		sum += popcnt32(p[sz - 1] & mask(bitsize));

		return sum;
}

ui32 binary::find_next(const ui32* p, ui32 bitsize, ui32 bit) {
    assert(bitsize > 0);		
    ui32 ind = bit >> UI32_LOG2BIT;
    ui32 offset = bit & UI32_MASK;

		ui32 buf = (p[ind] >> offset) << offset;
		if (buf == 0) {
			do {
				++ind;
				buf = p[ind];
			} while ((ind < size(bitsize)) & (buf == 0));
		}	
		offset = (ui32) tzcnt32(buf);
    return (ind << UI32_LOG2BIT) + offset;

}

//ui32 binary::find_prev(const ui32* p, ui32 bitsize, ui32 bit) {
//	assert(bitsize > 0);
//	assert(bit <= bitsize);
//	if (bit == bitsize)
//		return bitsize;
//
//	ui32 ind = bit >> UI64_LOG2BIT;
//	ui32 offset = bit & UI64_MASK;
//
//	ui64 buf = (RE_C64(p)[ind] << offset) >> offset;
//	while ((ind != 0) & (buf == 0)) {
//		--ind;
//		buf = RE_C64(p)[ind];
//	}
//	if (_BitScanReverse64((unsigned long*) &offset, (unsigned long long) buf)) {
//		return (ind << UI64_LOG2BIT) + offset;
//	} else {
//		return bitsize;
//	}
//}

//ui32 binary::find_first(const ui32* p, ui32& ind, ui32 max_ind) {
//	while (ind < max_ind && p[ind] == 0)
//
//	ui32 res = _tzcnt_u32(p[ind]);
//	ind += res >> UI32_LOG2BIT;//binary::test_zero(res ^ 0x20);
//	return (ind << UI32_LOG2BIT) + res;
//}

bool binary::any(const ui32* p, ui32 bitsize) {
	assert(bitsize > 0);

	ui32 sz = size(bitsize);

	ui32 buf = p[sz - 1];
	const_cast<ui32*>(p)[sz - 1] &= mask(bitsize);

	//res = 
	//	(*p != 0) || 
	//	(My_Memory::MM_memcmp(p, p + 1, UI32_SIZE*(sz - 1)) != 0);
	ui32 res = 0;
	for (ui32 ind = 0; ind < sz; ++ind) {
		res |= p[ind];
	}
	const_cast<ui32*>(p)[sz - 1] = buf;
	return res != 0;
}

bool binary::all(const ui32* p, ui32 bitsize) {
	assert(bitsize > 0);

	ui32 sz = size(bitsize);

	ui32 buf = p[sz - 1];
	const_cast<ui32*>(p)[sz - 1] |= ~mask(bitsize);

	ui32 res = UI32_ALL;
	for (ui32 ind = 0; ind < sz; ++ind) {
		res &= p[ind];
	}

	const_cast<ui32*>(p)[sz - 1] = buf;
	return res == UI32_ALL;
}

void binary::set(ui32* p, ui32 bit) {
	ui32 ind = (bit >> UI32_LOG2BIT);
	bit &= UI32_MASK;
	p[ind] |= ui32(1) << bit;
	//_bittestandset(reinterpret_cast<long*>(p + ind), bit & UI32_MASK);
}

void binary::reset(ui32* p, ui32 bit) {
	ui32 ind = (bit >> UI32_LOG2BIT);
	bit &= UI32_MASK;
	p[ind] &= ~(ui32(1) << bit);
	//_bittestandreset(reinterpret_cast<long*>(p + ind), bit & UI32_MASK);
}

char binary::at(const ui32* p, ui32 bit) {
	ui32 ind = bit >> UI32_LOG2BIT;
	bit &= UI32_MASK;
	return (p[ind] >> bit) & ui32(1);
	//return _bittest(reinterpret_cast<const long*>(p + ind), bit & UI32_MASK);
}

void binary::reset_le(ui32* p, ui32 bit) {
	ui32 k = (bit + 1) >> UI32_LOG2BIT;
	My_Memory::MM_memset(p, 0, k*UI32_SIZE);
	ui32 offset = (bit + 1) & UI32_MASK;
	ui32 mask = UI32_ALL << offset;
	p[k] &= mask;
}

void binary::transpose(ui32* dst, const ui32* src, ui32 m, ui32 n) {
	My_Memory::MM_memset(dst, 0, size(m) * n * UI32_SIZE);

	for (ui32 i = 0; i < m; ++i) {
		for (ui32 j = 0; j < n; ++j) {
			if (at(src + i * size(n), j))
				set(dst + j * size(m), i);
		}
	}
}

void binary::submatrix(ui32* dst, ui32* src, const ui32* rows, ui32 m, ui32 n) {
	//ui32 m_new = popcount(rows, m);
	//ui32* res = static_cast<ui32*>(My_Memory::MM_malloc(m_new * size(n) * UI32_SIZE));
	
	ui32 i = find_next(rows, m, 0);	
	while (i < m) {		
		My_Memory::MM_memcpy(dst, src + i*size(n), size(n)*UI32_SIZE);
		dst += size(n);
		i = find_next(rows, m, i + 1);
	}
}

/**********************************************************
service functions
**********************************************************/

void binary::Matrix::copy(const Matrix& src) {
	if (this != &src) {
		reserve(src.m_, src.n_);
		My_Memory::MM_memcpy(data_, src.data_, m_*row_size()*UI32_SIZE);
	}
}

//void binary::Matrix::swap(Matrix& src) {
//	if (this != &src) {
//		std::swap(m_, src.m_);
//		std::swap(n_, src.n_);
//		std::swap(data_, src.data_);
//		std::swap(capacity_, src.capacity_);
//	}
//}

void binary::Matrix::reserve(ui32 m, ui32 n) {
	ui32 row_sz = size(n);
	ui32 sz = m*row_sz;
	if (sz == 0) {//deallocate memory
		if (capacity_ > 0)
			My_Memory::MM_free(data_);
		capacity_ = 0;
		data_ = nullptr;
	} else {
		if (capacity_ < sz) {//reallocation
			if (capacity_ > 0)
				My_Memory::MM_free(data_);
			data_ = static_cast<ui32*>(My_Memory::MM_malloc(sz*UI32_SIZE));
			capacity_ = sz;
		}
	}
	m_ = m;
	n_ = n;
}

void binary::Matrix::random(ui32 m, ui32 n, float d) {
	reserve(m,n);
	My_Memory::MM_memset(data_, 0, m*row_size()*UI32_SIZE);
	ui32 threshold = ui32(RAND_MAX * d);
	//srand(seed);	
	for (ui32 i = 0; i < m_; ++i) {
		for (ui32 j = 0; j < n_; ++j) {
			if (static_cast<ui32>(rand()) < threshold)
				set(i, j);
		}      
	}
}

/**********************************************************
io functions
**********************************************************/

static void read_get_width_and_check(FILE* p_file, ui32& m, ui32& n) {
	char ch = 0;
	char state = 0;
	ui32 n0 = 0;
	fpos_t pos;
	if (fgetpos(p_file, &pos) != 0)
		throw std::runtime_error(string("read_get_width::") + std::strerror(errno));

	while (state<10 && ch != char(EOF)) {
		ch = static_cast<char>(fgetc(p_file));
		if (ferror(p_file))
			throw std::runtime_error(string("read_get_width::") + std::strerror(errno));
		//printf("ch = %d, state = %d, EOF = %d, m = %d, n = %d, n0 = %d, \\n = %d, \\r = %d\n", ch, state, char(EOF), m, n, n0, '\n', '\r');
		switch (state) {
		case 0:
			if (ch == '0' || ch == '1') {
				++n;
				state = 1;
			} else if (ch == ' ') {
				state = 0;//skip
			} else if (ch == char(EOF) || ch == '\n') {
				state = 11;
			} else if (ch == '\r') {
				;
			} else {
				state = 10;//error
			}
			break;
		case 1:
			if (ch == '0' || ch == '1') {
				++n;
			} else if (ch == ' ') {
				// skip
			} else if (ch == '\n' || ch == char(EOF)) {
				if (m == 0 || n == n0) {
					n0 = n;
					n = 0;
					++m;
					state = 0;
				} else {
					state = 10;
				}
			} else if (ch == '\r') {
				;
			} else {
				state = 10;
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

static char skip_space(FILE* p_file) {
	char ch = ' ';
	while (ch == ' ' || ch == '\n') {
		ch = static_cast<char>(fgetc(p_file));
	}
	return ch;
}

void binary::Matrix::read(FILE* p_file) {
	ui32 m = 0;
	ui32 n = 0;
	read_get_width_and_check(p_file, m, n);
	reserve(m, n);
	My_Memory::MM_memset(data_, 0, m*row_size()*UI32_SIZE);
	for (ui32 i = 0; i < m_; ++i) {
		for (ui32 j = 0; j < n_; ++j) {
			if (skip_space(p_file) == '1')
				set(i, j);
		}
	}

}

void binary::Matrix::read(const char* file_name) {
	FILE* p_file = fopen(file_name, "r");
	if (p_file == nullptr) {
		throw std::runtime_error(string("binary::Matrix::read::") + std::strerror(errno));
	}
	try {
		read(p_file);
	} catch (...) {
		fclose(p_file);
		throw;
	}
	fclose(p_file);
}

//void Bool_Matrix::read(const DynamicArray<char>& data, ui32 m, ui32 n) {
//	init(m, n);
//	for (ui32 i = 0; i < m; ++i) {
//		for (ui32 j = 0; j < n; ++j) {
//			if (data[i*n + j])
//				set(i, j);
//		}
//	}
//}

void binary::Matrix::print_bm(FILE* p_file) const {
	for (ui32 i = 0; i < m_; ++i) {
		for (ui32 j = 0; j < n_; ++j) {
			fputc((at(i, j) != 0) + '0', p_file);
			fputc(' ', p_file);
		}
		fputc('\n', p_file);
	}
	if (ferror(p_file))
		throw std::runtime_error(string("binary::Matrix::print::") + std::strerror(errno));
}

void binary::Matrix::print_0x(FILE* p_file) const {
	for (ui32 i = 0; i < m_; ++i) {
		for (ui32 ind = 0; ind < size(n_); ++ind) {
			fprintf(p_file, "%08x ", data_[i*row_size() + ind]);
		}
		fputc('\n', p_file);
	}
	if (ferror(p_file))
		throw std::runtime_error(string("binary::Matrix::print::") + std::strerror(errno));
}

void binary::Matrix::print_hg(FILE* p_file) const {
	for (ui32 i = 0; i < m_; ++i) {
		ui32 j = 0;
		ui32 const* row_i = row(i);
		j = binary::find_next(row_i, n_, 0);
		while (j < n_) {
			fprintf(p_file, "%d ", j + 1);
			j = binary::find_next(row_i, n_, j + 1);
		}
		fputc('\n', p_file);
	}
	if (ferror(p_file))
		throw std::runtime_error(string("binary::Matrix::print::") + std::strerror(errno));
}

void binary::Matrix::print(const char* file_name, const char* mode) const {
	std::string file_out = file_name;
	file_out += ".";
	file_out += std::string(mode);
	FILE* p_file = fopen(file_out.c_str(), "w");
	if (p_file == nullptr) {
		throw std::runtime_error(std::string("binary::Matrix::print::") + std::strerror(errno));
	}
	try {
		if (strcmp(mode, "bm") == 0) {
			print_bm(p_file);
		} else if (strcmp(mode, "hg") == 0) {
			print_hg(p_file);
		} else if (strcmp(mode, "0x") == 0) {
			print_0x(p_file);
		} else {
			throw std::runtime_error("binary::Matrix::print::invalid mode");
		}
	} catch (...) {
		fclose(p_file);
		throw;
	}
	fclose(p_file);
}

/**********************************************************
special functions
**********************************************************/

binary::Matrix& binary::Matrix::delete_le_rows() throw() {
	ui32 size32_n_ = binary::size(n_);
	ui32 size32_m_ = binary::size(m_);
	ui32* rows = SC_32(My_Memory::MM_malloc(size32_m_ * UI32_SIZE));
	My_Memory::MM_memset(rows, ui32(~0), size32_m_ * UI32_SIZE);

	ui32 i1 = binary::find_next(rows, m_, 0);
	ui32 i2 = 0;


	while (i1 < m_) {
		ui32 const* row1 = row(i1);
		i2 = binary::find_next(rows, m_, i1 + 1);
		
		while (i2 < m_) {
			ui32 const* row2 = row(i2);
			ui32 buf1 = 0;
			ui32 buf2 = 0;
			ui32 ind = 0;
			do {
				buf1 |= row1[ind] & ~row2[ind];
				buf2 |= ~row1[ind] & row2[ind];
				++ind;
			} while (ind < size32_n_);

			if (buf1 == 0) {
				binary::reset(rows, i2);
			} else if (buf2 == 0) {
				binary::reset(rows, i1);
				break;
			}

			i2 = binary::find_next(rows, m_, i2 + 1);
		}
		i1 = binary::find_next(rows, m_, i1 + 1);
	}

	binary::submatrix(data_, data_, rows, m_, n_);
	m_ = binary::popcount(rows, m_);
	My_Memory::MM_free(rows);

	return *this;
}

binary::Matrix& binary::Matrix::random_stripe(const binary::Matrix& src, ui32 height) {
	ui32 m = src.m_;
	assert(height <= m);

	std::vector<ui32> selection;
	selection.resize(m);
	for (ui32 i = 0; i < m; ++i) {
		selection[i] = i;
	}
	std::random_shuffle(selection.begin(), selection.end());
	selection.resize(height);

	reserve(height, src.n_);
	for (ui32 i = 0; i < height; ++i) {
		My_Memory::MM_memcpy(row(i), src.row(selection[i]), size(n_) * UI32_SIZE);
	}

	return *this;
}

binary::Matrix& binary::Matrix::sort_cols(const binary::Matrix& src) {

	std::vector<ui32> permutation(src.width(), 0);
	for (ui32 j = 0; j < src.width(); ++j) {
		permutation[j] = j;
	}

	{
		std::vector<ui32> weights(src.width(), 0);
		for (ui32 i = 0; i < src.height(); ++i) {
			ui32 const* row_i = src.row(i);
			ui32 j = find_next(row_i, src.width(), 0);
			while (j < src.width()) {
				++weights[j];
				j = find_next(row_i, src.width(), j + 1);
			}
		}

		ui32 n = src.width();
		ui32 newn = 0;
		do {

			//for (auto& e = weights.begin(); e < weights.end(); ++e) {
			//	printf("%d ", *e);
			//}
			//printf("\n");
			//fflush(stdout);

			newn = 0;
			for (ui32 j = 1; j < n; ++j) {
				if (weights[j - 1] < weights[j]) {
					std::swap(weights[j - 1], weights[j]);
					std::swap(permutation[j - 1], permutation[j]);
					newn = j;
				}
			}
			n = newn;
		} while (n > 0);



		
	}



	reserve(src.height(), src.width());
	for (ui32 i = 0; i < src.height(); ++i) {
		ui32* row_i = row(i);
		ui32 const* row_i_src = src.row(i);
		for (ui32 j = 0; j < src.width(); ++j) {
			if (binary::at(row_i_src, permutation[j])) {
				binary::set(row_i, j);
			} else {
				binary::reset(row_i, j);
			}
		}
	}

	return *this;
}