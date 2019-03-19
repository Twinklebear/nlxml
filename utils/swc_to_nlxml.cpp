#include <iostream>
#include <set>
#include <map>
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
	int id = 0;
	int type = 0;
	float x = 0;
	float y = 0;
	float z = 0;
	float radius = 0;
	int parent_id = 0;
	int children = 0;
};

std::ostream& operator<<(std::ostream &os, const SWCPoint &p) {
	os << "(" << p.id << ", " << p.type << ", ["
		<< p.x << ", " << p.y << ", " << p.z << "], "
		<< p.radius << ", " << p.parent_id << ", {" << p.children << "})";
	return os;
}

bool operator<(const SWCPoint &a, const SWCPoint &b) {
	return a.id < b.id;
}

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
	p.radius = p.id;// debugging std::stof(vals[5]);
	p.parent_id = std::stoi(vals[6]);
	return p;
}

// Points in the map are indexed by their parent ID, with a set of
// all points who are parented by that point
using SWCMap = std::map<int, std::set<SWCPoint>>;

template<typename T>
void convert_swc_branch(T &branch, const SWCPoint parent,
		const std::set<SWCPoint> &children, const SWCMap &swcpoints)
{
	for (auto c = children.cbegin(); c != children.cend(); ++c) {
		SWCPoint child = *c;
		branch.points.push_back(Point(child.x, child.y, child.z, child.radius));

		auto child_children = swcpoints.find(child.id);
		if (child_children == swcpoints.end() || child_children->second.empty()) {
			return;
		} else if (child_children->second.size() == 1) {
			convert_swc_branch(branch, child, child_children->second, swcpoints);
		} else {
			// We're branching into the child's children
			for (auto branches = child_children->second.cbegin();
					branches != child_children->second.cend(); ++branches)
			{
				Branch b;
				SWCPoint bstart = *branches;
				b.points.push_back(Point(bstart.x, bstart.y, bstart.z, bstart.radius));
				auto bpoints = swcpoints.find(bstart.id);
				if (bpoints != swcpoints.end()) {
					convert_swc_branch(b, bstart, bpoints->second, swcpoints);
				}
				branch.branches.push_back(b);
			}
		}
	}
}

NeuronData convert_swc(const SWCMap &swcpoints) {
	NeuronData data;

	auto treeiter = swcpoints.find(-1);
	if (treeiter == swcpoints.end()) {
		std::cout << "No trees in file!?\n";
		return data;
	}

	auto trees = treeiter->second;
	for (auto it = trees.begin(); it != trees.end(); ++it) {
		const SWCPoint p = *it;
		Tree t;
		t.color = Color(1, 1, 1);
		switch (p.type) {
			case 1: t.type = "Soma"; break;
			case 2: t.type = "Axon"; break;
			case 3: t.type = "Dendrite"; break;
			case 4: t.type = "Apical Dendrite"; break;
			case 7: t.type = "Custom"; break;
			default: t.type = "Undefined";
		}
		t.leaf = "Normal";
		t.points.push_back(Point(p.x, p.y, p.z, p.radius));

		auto children = swcpoints.find(p.id);
		if (children != swcpoints.end()) {
			std::cout << "tree parent id " << p.id << " has children: {\n";
			for (auto &p : children->second) {
				std::cout << "  " << p << "\n";
			}
			std::cout << "}\n";

			convert_swc_branch(t, p, children->second, swcpoints);
		}
		data.trees.push_back(t);
	}

	return data;
}

SWCMap import_swc(const std::string &fname) {
	std::ifstream fin(fname.c_str());
	
	SWCMap points;
	std::string line;
	while (std::getline(fin, line)) {
		if (line.empty() || line[0] == '#') {
			continue;
		}
		SWCPoint p = read_point(line);
		points[p.parent_id].insert(p);
	}
	return points;
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
	auto swcpoints = import_swc(input);

	auto data = convert_swc(swcpoints);
	export_file(data, output);

	return 0;
}


