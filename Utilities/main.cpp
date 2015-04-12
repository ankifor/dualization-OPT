#include <stdexcept>//for runtime_error
#include <string>
#include <iostream>
#include "binary.h"

using namespace std;

void main(int argc, char** argv) {

	binary::Matrix L;
	ui32 m = 0;
	ui32 n = 0;
	float d = 0.0;
	unsigned seed = 0;
	try {
		std::string help = 
			"argv[1] is file_out\n"
			"argv[2] is m\n"
			"argv[3] is n\n"
			"argv[4] is d\n"
			"argv[5] is seed\n";
		if (argc < 6)
			throw std::runtime_error(string("not enough arguments:\n") + help);
		m = atoi(argv[2]);
		n = atoi(argv[3]);
		d = atof(argv[4]);
		seed = atoi(argv[5]);
		L.random(m, n, d, seed);
		L.print(argv[1], "bm");
		L.print(argv[1], "hg");
	} catch (runtime_error& rte) {
		std::cout << rte.what();
	} catch (...) {
		std::cout << "Unknown error\n";
	}
}