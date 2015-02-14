#include <stdexcept>
#include <iostream>
#include <cstdio>
#include <cstdint>
#include "bool_matrix.h"

using namespace std;

static const char POW2[8] = { 1, 2, 4, 8, 16, 32, 64, -127 };

static char positive(int32_t x) {
	return (-x >> 31) & 1;
}

static char zero(int32_t x) {
	return (((x | -x) >> 31) & 1) ^ 1;
}

int find_next(const char* x, int32_t i, int32_t n) {
	int upper = i;
	char buf = 0;
	while (positive(n-i) && zero(upper-i)) { 
		upper = 8 * (i / 8 + 1);
		buf = x[i >> 3] >> (i & 0x7);
		while (positive(n - i) & positive(upper-i) & ((buf & 1)^1)) {
			buf >>= 1;
			++i;
		}
	}
	return i;
}

void main() {
	int m = 1;
	int n = 95;
	Bool_Matrix L;
	try {
		L.random(m, n);
		L.print(stdout);
		cout << endl;
		char* p = nullptr; 
		p = L.row(0);
		int sum = 0;
		for (int j = find_next(p, 0, n); j < n; j = find_next(p, j + 1, n)) {
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