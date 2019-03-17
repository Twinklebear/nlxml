#include <iostream>
#include <algorithm>
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "nlxml.h"

using namespace nlxml;

void remove_degree2_nodes(Branch &b) {
	while (b.branches.size() == 1) {
		auto &deg2 = b.branches[0];
		std::copy(deg2.points.begin(), deg2.points.end(), std::back_inserter(b.points));
		std::copy(deg2.markers.begin(), deg2.markers.end(), std::back_inserter(b.markers));
		std::copy(deg2.branches.begin(), deg2.branches.end(), std::back_inserter(b.branches));
		b.branches.erase(b.branches.begin());
	}
	for (auto &c : b.branches) {
		remove_degree2_nodes(c);
	}
}

int main(int argc, char **argv) {
	std::string input, output;
	for (int i = 1; i < argc; ++i) {
		if (std::strcmp(argv[i], "-o") == 0) {
			output = argv[++i];
		} else {
			input = argv[i];
		}
	}
	if (input.empty() || output.empty()) {
		std::cout << "Error: an input and output file are needed.\n"
			<< "Usage: ./blah <input> -o <output>\n";
		return 1;
	}

	NeuronData data = import_file(input);

	// Go through and remove all degree-2 nodes
	for (auto &t : data.trees) {
		for (auto &b : t.branches) {
			remove_degree2_nodes(b);
		}
	}

	// TODO: Go through and remove points which are extremely close to each other

	export_file(data, output);
	return 0;
}


