#include <stdexcept>//for runtime_error
#include <iostream>

#include "dualizer_OPT.h"
#include "my_memory.h"



using namespace std;

int main(int argc, char** argv) {

#ifndef NDEBUG
	std::atexit(My_Memory::print);
#endif

	binary::Matrix L;
	Dualizer_OPT solver;
	try {
		L.read(argv[1], nullptr);
		//L.delete_le_rows();
		solver.init(L, argv[2]);
		solver.run1();
		solver.print();
	} catch (runtime_error& rte) {
		cout << rte.what() << endl;
		return 1;
	} catch (...) {
		cout << "Unknown error" << endl;
		return 2;
	}
	return 0;
}