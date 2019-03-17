#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include "nlxml.h"

using namespace nlxml;

/* This program will take a gold standard NLXML file and a list
 * of points that the compared tracing missed and generate an NLXML
 * file in the right space with markers placed at missed nodes.
 *
 * To generate pipe the output of the diadem metric to this program
 */
std::vector<Point> read_nodes() {
	std::vector<Point> pts;
	std::string line;
	while (std::getline(std::cin, line)) {
		if (line.empty()) {
			break;
		}
		Point p;
		if (std::sscanf(line.c_str(), "(%f,%f,%f) %f", &p.x, &p.y, &p.z, &p.d) != 4) {
			std::cerr << "Error reading point string '" << line << "'\n";
			std::exit(1);
		}
		pts.push_back(p);
	}
	return pts;
}

int main(int argc, char **argv) {
	std::string gold_file, output;
	for (int i = 1; i < argc; ++i) {
		if (std::strcmp(argv[i], "-o") == 0) {
			output = argv[++i];
		} else if (std::strcmp(argv[i], "-g") == 0) {
			gold_file = argv[++i];
		}
	}
	if (gold_file.empty() || output.empty()) {
		std::cout << "Error: a gold and output file are needed.\n"
			<< "Usage: ./nlxml_diadem_missed -g <gold> -o <output>\n";
		return 1;
	}

	NeuronData gold = import_file(gold_file);
	NeuronData missed;
	missed.images = gold.images;

	std::string line;
	while (std::getline(std::cin, line)) {
		if (line == "Nodes that were missed (position and weight):") {
			std::vector<Point> pts = read_nodes();
			std::cout << "Found " << pts.size() << " missed nodes\n";
			Marker markers{"FilledSquare", "missed pts", Color{1.0, 0.0, 0.0}, false, pts};
			missed.markers.push_back(markers);
		} else if (line == "Extra nodes in test reconstruction (position and weight):") {
			std::vector<Point> pts = read_nodes();
			std::cout << "Found " << pts.size() << " extra nodes\n";
			Marker markers{"FilledDiamond", "extra pts", Color{0.0, 0.0, 1.0}, false, pts};
			missed.markers.push_back(markers);
		}
	}
	export_file(missed, output);
	return 0;
}


