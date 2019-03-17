#include <iostream>
#include "nlxml.h"

using namespace nlxml;

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <file.xml>\n";
		return 1;
	}
	NeuronData data = import_file(argv[1]);
	std::cout << "File contains " << data.trees.size() << " trees and "
		<< data.contours.size() << " contours\n";
	std::cout << "== Trees ==\n";
	for (const auto &t : data.trees) {
		std::cout << t << "\n";
	}
	std::cout << "== Contours ==\n";
	for (const auto &c : data.contours) {
		std::cout << c << "\n";
	}
	std::cout << "== Markers ==\n";
	for (const auto &m : data.markers) {
		std::cout << m << "\n";
	}
	std::cout << "== Images ==\n";
	for (const auto &i : data.images) {
		std::cout << i << "\n";
	}
	return 0;
}

