#ifdef NDEBUG
//#include <ctime>//for clock
#endif 

#include <stdexcept>//for runtime_error
#include <iostream>

#include "dualizer_OPT.h"
#include "my_memory.h"

#ifndef NDEBUG
int My_Memory::free_times = 0;
int My_Memory::malloc_times = 0;
int My_Memory::malloc_size = 0;
int My_Memory::memset_times = 0;
int My_Memory::memcpy_times = 0;
int My_Memory::memcmp_times = 0;
int My_Memory::memcpy_size = 0;
int My_Memory::memcmp_size = 0;
int My_Memory::memset_size = 0;
#endif


using namespace std;

void main(int argc, char** argv) {

#ifndef NDEBUG
	std::atexit(My_Memory::print);
#endif

	binary::Matrix L;
	Dualizer_OPT solver;

	try {
		

#ifdef NDEBUG
		//clock_t begin = clock();
		L.read(argv[1]);
		solver.init(L, argv[2], "w");
		solver.run();
		//clock_t end = clock();
		//double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
		//cout << "Elapsed time: " << elapsed_secs << endl;
#else
		L.read("mat.txt");
		solver.init(L);
		solver.run();
#endif

	} catch (runtime_error& rte) {
		cout << rte.what() << " sec" << endl;
	} catch (...) {
		cout << "Unknown error" << endl;
	}
}