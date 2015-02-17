#include <stdexcept>
#include <iostream>
#include "my_int.h"
#include "bool_matrix.h"

using namespace std;


void main() {
	ui32 m = 1;
	ui32 n = 260;
	Bool_Matrix L;

	try {
		L.random(m, n, 0.01f);
		L.print(stdout);
		cout << endl;

		for (ui32 j = L.find_next(0); j < L.width(); j = L.find_next(j+1)) {	
			cout << j << ' ';
		}
		cout << endl;
		cout << L.popcount() << endl;
		cout << endl;
	} catch (runtime_error& rte) {
		cout << rte.what() << endl;
	} catch (...) {
		cout << "Unknown error" << endl;
	}
}