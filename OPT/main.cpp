#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <ctime>
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

void main(int argc, char** argv) {
	std::atexit(My_Memory::print);
	//ui32 m = 1;
	//ui32 n = 66;
	Bool_Matrix L;
	Dualizer_OPT solver;

	try {
		clock_t begin = clock();

#ifdef NDEBUG
		L.read(argv[1]);
		solver.init(L, argv[2]);
#else
		L.read("mat1.txt");
		solver.init(L, "res.txt");
#endif

		
		solver.run();

		clock_t end = clock();
		double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
		cout << "Elapsed time: " << elapsed_secs << endl;
	} catch (runtime_error& rte) {
		cout << rte.what() << " sec" << endl;
	} catch (...) {
		cout << "Unknown error" << endl;
	}
}