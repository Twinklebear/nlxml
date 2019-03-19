#include <iostream>
#include <iterator>
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

struct SWCPoint {
	int id;
	int type;
	float x, y, z;
	float radius;
	int parent_id;
};

SWCPoint read_point(const std::string &l) {
	std::istringstream iss(l);
	std::vector<std::string> vals{std::istream_iterator<std::string>(iss),
		std::istream_iterator<std::string>()};

	SWCPoint p;
	p.id = std::stoi(vals[0]);
	p.type = std::stoi(vals[1]);
	p.x = std::stof(vals[2]);
	p.y = std::stof(vals[3]);
	p.z = std::stof(vals[4]);
	p.radius = std::stof(vals[5]);
	p.parent_id = std::stoi(vals[6]);
	return p;
}

size_t depth = 1;

// Returns the next line to be read, since in the format we
// don't know if we're done until we actually go past it
template<typename T>
std::string import_swc_tree(std::istream &is, T &branch, std::string line) {
	std::string indent(depth, '\t');
	depth += 1;
	do {
		if (line.empty() || line[0] == '#') {
			continue;
		}

		SWCPoint p = read_point(line);
		if (p.parent_id == -1 || p.type == 1) {
			depth -= 1;
			return line;
		}

		if (p.type == 6) {
			// This branch has ended, we're done
			branch.points.push_back(Point(p.x, p.y, p.z, p.radius));
			return "";
		} else if (p.type == 5) {
			// We've got a child branch starting at the point we read,
			// so build all the branches we split into
			branch.points.push_back(Point(p.x, p.y, p.z, p.radius));
			int branch_point_id = p.id;
			line.clear();
			// We'll either get back a line from a child when it encounters a
			// new branch that it's not the parent of, or we get an empty line
			// back and keep reading our own branches
			while (!line.empty() || std::getline(is, line)) {
				p = read_point(line);
				// Check if we're actually the parent of this point we read
				if (p.parent_id != branch_point_id) {
					return line;
				}

				Branch b;
				b.leaf = "Normal";
				// The child branches won't return anything to us, since they terminate
				// when they read their end point
				line = import_swc_tree(is, b, line);
				branch.branches.push_back(b);
			}
			// This branch ends here, since it forked into two or more branches
			return "";
		} else {
			branch.points.push_back(Point(p.x, p.y, p.z, p.radius));
		}
	} while (std::getline(is, line));
	return line;
}

NeuronData import_swc(const std::string &fname) {
	NeuronData data;

	std::ifstream fin(fname.c_str());
	
	std::string line;
	while (!line.empty() || std::getline(fin, line)) {
		if (line.empty() || line[0] == '#') {
			line.clear();
			continue;
		}
		SWCPoint p = read_point(line);

		// Start of a new tree
		if (p.id == 1 || p.parent_id == -1 || p.type == 1) {
			Tree t;
			t.color = Color(1, 1, 1);
			t.type = "Axon";
			t.leaf = "Normal";
			t.points.push_back(Point(p.x, p.y, p.z, p.radius));

			if (std::getline(fin, line)) {
				line = import_swc_tree(fin, t, line);
			}
			data.trees.push_back(t);
		} else {
			std::cout << "Unexpected normal point?? " << line << "\n";
			break;
		}
	}

	return data;
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
			<< "Usage: ./" << argv[0] << " <input> -o <output>\n";
		return 1;
	}

	std::cout << "Exporting SWC file as NLXML to " << output << "\n";
	export_file(import_swc(input), output);

	return 0;
}


