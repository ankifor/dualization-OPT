#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include "my_int.h"
#include "my_memory.h"
#include "bool_vector.h"
#include "bool_matrix.h"

int My_Memory::free_times = 0;
int My_Memory::malloc_times = 0;
int My_Memory::memset_times = 0;
int My_Memory::memcpy_times = 0;
int My_Memory::malloc_size = 0;
int My_Memory::memcpy_size = 0;
int My_Memory::memset_size = 0;


using namespace std;

void main() {
	std::atexit(My_Memory::print);
	ui32 m = 1;
	ui32 n = 66;
	Bool_Matrix L;
	Bool_Vector a;
	Bool_Vector mask;
	try {
		//L.read("mat1.txt");
		//L.random(m, n, 0.5f);
		//L.print(stdout);
		//cout << endl;
		//L.print("mat1.txt");
		//L.read("mat1.txt");
		//L.print("mat1.txt");
		a.make_mask(n);
		a.setall();
		for (ui32 j = a.find_next(0); j < a.bitsize(); j = a.find_next(j + 1)) {
			cout << j << ' ';
		}
		cout << endl;
		a.reset_irrelevant_bits();
		for (ui32 j = a.find_next(0); j < a.bitsize(); j = a.find_next(j + 1)) {
			cout << j << ' ';
		}
		cout << endl;
		//cout << endl;
		//cout << b.popcount() << endl;
		//cout << endl;
		
	} catch (runtime_error& rte) {
		cout << rte.what() << endl;
	} catch (...) {
		cout << "Unknown error" << endl;
	}
}