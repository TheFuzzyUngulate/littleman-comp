#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <deque>
#include <algorithm>
#include <fstream>
#include <iostream>

#include "scanner.hpp"
#include "processor.hpp"

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cout << "Illegal argument count\n";
		exit(-1);
	}

	std::ifstream myfile;
	myfile.open(argv[1]);
	if (!myfile.is_open()) {
		std::cout << "unable to open file\n";
		exit(-1);
	}

	Scanner sc = Scanner((std::fstream*)&myfile);
	Processor pr = Processor(&sc);
	pr.instr_cycle();
}
