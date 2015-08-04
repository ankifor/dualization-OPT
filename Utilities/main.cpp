#include <stdexcept>//for runtime_error
#include <string>
#include <iostream>
#include "binary.h"

using namespace std;



int main(int argc, char** argv) {	

	try {
		
		std::string help = 
			"-random <file_out> <m> <n> <prob> <seed>\n"
			"-sort <file_in> <file_out>\n"
			"-convert <file_in> <file_out>\n"
			"valid extensions for input: bm, hg, packed\n"
			"valid extensions for output: bm, hg, packed, 0x\n"
			;
		if (argc < 2)
			throw std::runtime_error(string("not enough arguments:\n") + help);
		if (strcmp(argv[1], "-random") == 0) {
			if (argc < 7)
				throw std::runtime_error(string("not enough arguments:\n") + help);			
			ui32 m = atoi(argv[3]);
			ui32 n = atoi(argv[4]);
			float d = atof(argv[5]);
			unsigned seed = atoi(argv[6]);
			srand(seed);
			binary::Matrix L;
			L.random(m, n, d);
			L.print(argv[2], nullptr);
		} else if (strcmp(argv[1], "-sort") == 0) {
			if (argc < 4)
				throw std::runtime_error(string("not enough arguments:\n") + help);
			binary::Matrix L;
			L.read(argv[2], nullptr);
			binary::Matrix L1;
			L1.sort_cols(L);
			L1.print(argv[3], nullptr);
		} else if (strcmp(argv[1], "-convert") == 0) {
			binary::Matrix L;
			L.read(argv[2], nullptr);
			L.print(argv[3], nullptr);
		} else {
			throw std::runtime_error(string("invalid input:\n") + help);
		}
	} catch (runtime_error& rte) {
		std::cout << rte.what();
		return 1;
	} catch (...) {
		std::cout << "Unknown error\n";
		return 2;
	}
	return 0;
}