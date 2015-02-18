#include <stdexcept>
#include <iostream>
#include "my_int.h"
#include "bool_vector.h"
#include "bool_matrix.h"

using namespace std;


void main() {
	ui32 m = 1;
	ui32 n = 34;
	Bool_Matrix L;
	Bool_Vector a;
	try {
		L.random(m, n, 0.5f);
		//L.print(stdout);
		//cout << endl;
		//L.print("mat1.txt");
		//L.read("mat1.txt");
		//L.print("mat1.txt");
		
		a.assign(L.row(0), L.width());
		Bool_Vector b(a.bitsize()+32);
		b.copy(a);
		L.print(stdout);
		cout << endl;
		for (ui32 j = b.find_next(0); j < b.bitsize(); j = b.find_next(j + 1)) {
			cout << j << ' ';
		}
		cout << endl;
		cout << b.popcount() << endl;
		cout << endl;

	} catch (runtime_error& rte) {
		cout << rte.what() << endl;
	} catch (...) {
		cout << "Unknown error" << endl;
	}
}