#include <stdexcept>
#include <iostream>
#include "my_int.h"
#include "bool_vector.h"
#include "bool_matrix.h"


using namespace std;

void main() {
	ui32 m = 1;
	ui32 n = 35;
	Bool_Matrix L;
	Bool_Vector a;
	Bool_Vector mask;
	try {
		L.read("mat1.txt");
		//L.random(m, n, 0.5f);
		L.print(stdout);
		cout << endl;
		//L.print("mat1.txt");
		//L.read("mat1.txt");
		//L.print("mat1.txt");

		a.resetupto(0);
		Bool_Vector b(L.row(0));
		
		for (ui32 j = b.find_next(0); j < b.size()*32; j = b.find_next(j + 1)) {
			cout << j << ' ';
		}
		//cout << endl;
		//cout << b.popcount() << endl;
		//cout << endl;

	} catch (runtime_error& rte) {
		cout << rte.what() << endl;
	} catch (...) {
		cout << "Unknown error" << endl;
	}
}