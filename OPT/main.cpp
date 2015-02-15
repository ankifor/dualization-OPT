#include <stdexcept>
#include <iostream>
#include <cstdio>
#include <cstdint>
#include "bool_matrix.h"

using namespace std;

static const char POW2[8] = { 1, 2, 4, 8, 16, 32, 64, -127 };
static const uint32_t BITS = 32;
static const uint32_t LOG2BIT2 = 5;
static const uint32_t MASK = BITS - 1;

static char nonzero(uint32_t x) {
	return (x | (~x+1)) >> (BITS - 1);
}

uint32_t find_next(const uint32_t* x, uint32_t i, uint32_t n) {
	uint32_t ind = i >> LOG2BIT2;
	uint32_t upper = BITS * (ind + 1);
	uint32_t buf = ~(x[ind] >> (i & MASK));
	if (i >= n)
		return i;
	while (true) { 
		if (upper < n) {
			while (nonzero(upper^i) & buf) {
				buf >>= 1;
				++i;
			}
			if (i == upper) {
				upper += BITS;
				++ind;
				buf = ~x[ind];
			} else {
				break;
			}
		} else {
			while (nonzero(n^i) & buf) {
				buf >>= 1;
				++i;
			}
			break;
		}
	}
	return i;
}

void main() {
	uint32_t m = 1;
	uint32_t n = 260;
	Bool_Matrix L;
	try {
		L.random(m, n,0.5);
		//L.read("mat.txt");
		L.print(stdout);
		cout << endl;

		uint32_t* p = nullptr; 
		p = L.row(0);
		uint32_t sum = 0;
		for (uint32_t j = find_next(p, 0, n); j < n; j = find_next(p, j + 1, n)) {
			cout << j << ' ';
			sum++;
		}
		cout << endl << sum << endl;
		
	} catch (runtime_error& rte) {
		cout << rte.what() << endl;
	} catch (...) {
		cout << "Unknown error" << endl;
	}
	//L.random(m, n);
	//L.print("mat.txt", "w");

}