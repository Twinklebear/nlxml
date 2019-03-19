#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "nlxml.h"

using namespace nlxml;

/* Read the SWC file (http://research.mssm.edu/cnic/swc.html) recursively
 * down the branches to import it as NLXML
 * File format is a bunch of lines in ASCII with numbers specifying:
 *
 * n T x y z R P
 *
 * n is an integer label that identifies the current point
 * and increments by one from one line to the next.
 *
 * T is an integer representing the type of neuronal segment,
 * such as soma, axon, apical dendrite, etc. The standard accepted
 * integer values are given below.
 *
 * 0 = undefined
 * 1 = soma
 * 2 = axon
 * 3 = dendrite
 * 4 = apical dendrite
 * 5 = fork point
 * 6 = end point
 * 7 = custom
 *
 * x, y, z gives the cartesian coordinates of each node.
 *
 * R is the radius at that node.
 *
 * P indicates the parent (the integer label) of the current point
 * or -1 to indicate an origin (soma). 
 */

size_t depth = 1;

// Returns the next line to be read, since in the format we
// don't know if we're done until we actually go past it
template<typename T>
std::string import_swc_tree(std::istream &is, T &branch, int prev_point_id) {
	std::string indent(depth, '\t');
	std::string line;
	depth += 1;
	while (true) {
		if (line.empty()) {
			if (!std::getline(is, line)) {
				break;
			}
		}
		if (line.empty() || line[0] == '#') {
			continue;
		}
		std::istringstream iss(line);
		std::vector<std::string> vals{std::istream_iterator<std::string>(iss),
				std::istream_iterator<std::string>()};

		const int id = std::stoi(vals[0]);
		const int type = std::stoi(vals[1]);
		const float x = std::stof(vals[2]);
		const float y = std::stof(vals[3]);
		const float z = std::stof(vals[4]);
		const float radius = std::stof(vals[5]);
		const int parent_id = std::stoi(vals[6]);

		if (parent_id == -1) {
			return line;
		}

		std::cout << indent << line << "\n";
		// While it would be nice if the SWC files always followed the
		// type convention described above, it looks like Vaa3D doesn't do
		// this, so we can't rely on this.
		if (parent_id != prev_point_id) {
			// If it's one of our branches parse it, otherwise pop back
			std::cout << indent << "Branching at " << line << "\n";
			if (parent_id == prev_point_id) {
				Branch b;
				b.points.push_back(Point(x, y, z, radius));
				line = import_swc_tree(is, b, id);
				branch.branches.push_back(b);
			} else {
				break;
			}
		} else {
			std::cout << indent << "regular point " << line << "\n";
			branch.points.push_back(Point(x, y, z, radius));
			line.clear();
			prev_point_id = id;
		}
	}
	depth -=1;
	return line;
}

NeuronData import_swc(const std::string &is) {
	NeuronData data;

	std::ifstream fin(is.c_str());
	
	std::string line;
	std::getline(fin, line);
	while (true) {
		if (line.empty() || line[0] == '#') {
			std::getline(fin, line);
			continue;
		}
		std::istringstream iss(line);
		std::vector<std::string> vals{std::istream_iterator<std::string>(iss),
				std::istream_iterator<std::string>()};

		const int id = std::stoi(vals[0]);
		const int type = std::stoi(vals[1]);
		const float x = std::stof(vals[2]);
		const float y = std::stof(vals[3]);
		const float z = std::stof(vals[4]);
		const float radius = std::stof(vals[5]);
		const int parent_id = std::stoi(vals[6]);

		// Start of a new tree
		if (id == 1 || parent_id == -1) {
			Tree t;
			t.color = Color(0, 0, 1);
			t.points.push_back(Point(x, y, z, radius));
			line = import_swc_tree(fin, t, id);
			data.trees.push_back(t);
			if (line.empty()) {
				break;
			}
		} else {
			std::cout << "Unexpected normal point?? " << line << "\n";
			break;
		}
	}

	return data;
}

int main(int argc, char **argv) {
	throw std::runtime_error("This is not implemented yet");
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
			<< "Usage: ./" << argv[0] << " <input> -o <output>\n";
		return 1;
	}

	std::cout << "Exporting SWC file as NLXML to " << output << "\n";
	export_file(import_swc(input), output);

	return 0;
}


