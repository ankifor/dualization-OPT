#include <stdexcept>//for runtime_error
#include <string>
#include <iostream>
#include "binary.h"

using namespace std;

void main(int argc, char** argv) {

	
	

	try {
		
		std::string help = 
			"-random <file_out> <m> <n> <prob> <seed>\n"
			"-sort <file_in> <file_out>\n"
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
			L.print(argv[2], "bm");
			L.print(argv[2], "hg");
		} else if (strcmp(argv[1], "-sort") == 0) {
			if (argc < 4)
				throw std::runtime_error(string("not enough arguments:\n") + help);
			binary::Matrix L;
			L.read(argv[2]);
			binary::Matrix L1;
			L1.sort_cols(L);
			L1.print(argv[3], "bm");
			L1.print(argv[3], "hg");
		} else {
			throw std::runtime_error(string("invalid input:\n") + help);
		}

		

		//L.print_bm(stdout);
		//fputc('\n', stdout);
		//u = atoi(argv[6]);
		//L1.random_stripe(L, u);
		//L1.print_bm(stdout);

	} catch (runtime_error& rte) {
		std::cout << rte.what();
	} catch (...) {
		std::cout << "Unknown error\n";
	}
}