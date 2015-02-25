#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include "my_int.h"
#include "my_memory.h"
#include "bool_vector.h"
#include "bool_matrix.h"
#include "dualizer_OPT.h"

int My_Memory::free_times = 0;
int My_Memory::malloc_times = 0;
int My_Memory::memset_times = 0;
int My_Memory::memcpy_times = 0;
int My_Memory::memcmp_times = 0;
int My_Memory::malloc_size = 0;
int My_Memory::memcpy_size = 0;
int My_Memory::memcmp_size = 0;
int My_Memory::memset_size = 0;


using namespace std;

void main() {
	std::atexit(My_Memory::print);
	//ui32 m = 1;
	//ui32 n = 66;
	Bool_Matrix L;
	Dualizer_OPT solver;

	try {
		L.read("mat.txt");
		//L.random(m, n, 0.5f);
		L.print(stdout);
		cout << endl;
		solver.init(L, "res.txt");
		solver.run();

		
	} catch (runtime_error& rte) {
		cout << rte.what() << endl;
	} catch (...) {
		cout << "Unknown error" << endl;
	}
}