#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "nlxml.h"

using namespace nlxml;

void transform_branch(Branch &b, const glm::mat4 &transform) {
	const auto apply_mat = [&](const Point &p) {
		auto a = transform * glm::vec4(p.x, p.y, p.z, 1.f);
		return nlxml::Point(a.x, a.y, a.z, p.d);
	};

	auto transform_points = [&](auto &points) {
		std::transform(std::begin(points), std::end(points), std::begin(points), apply_mat);
	};
	auto transform_markers = [&](auto &markers) {
		for (auto &m : markers) {
			transform_points(m.points);
		}
	};
	transform_points(b.points);
	transform_markers(b.markers);
	for (auto &child_b : b.branches) {
		transform_branch(child_b, transform);
	}
}
void transform_neuron_data(NeuronData &inout, const glm::mat4 &transform) {
	const auto apply_mat = [&](const Point &p) {
		auto a = transform * glm::vec4(p.x, p.y, p.z, 1.f);
		return nlxml::Point(a.x, a.y, a.z, p.d);
	};
	auto transform_points = [&](auto &points) {
		std::transform(std::begin(points), std::end(points), std::begin(points), apply_mat);
	};
	auto transform_markers = [&](auto &markers) {
		for (auto &m : markers) {
			transform_points(m.points);
		}
	};
	for (auto &t : inout.trees) {
		transform_points(t.points);
		for (auto &b : t.branches) {
			transform_branch(b, transform);
		}
		transform_markers(t.markers);
	}
	transform_markers(inout.markers);
}

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

// Write the branch and sub-branches out in SWC, returns the id of the last point on this branch
template<typename B>
size_t write_branch_swc(std::ostream &os, const B &b, int parent_id, size_t point_id) {
	for (size_t i = 0; i < b.points.size(); ++i) {
		const auto &p = b.points[i];

		os << point_id << " ";
		if (parent_id == -1) {
			// Start point
			os << "1 ";
		} else if (i + 1 == b.points.size()) {
			if (!b.branches.empty()) {
				// Fork point
				os << "5 ";
			} else {
				// End point
				os << "6 ";
			}
		} else {
			// regular point 
			os << "0 ";
		}

		os << p.x << " " << p.y << " " << p.z << " 1 " << parent_id << "\n";

		parent_id = point_id;
		++point_id;
	}
	for (const auto &b : b.branches) {
		point_id = write_branch_swc(os, b, parent_id, point_id);
	}
	return point_id;
}

int main(int argc, char **argv) {
	std::string input, output, output_xml;
	bool apply_file_tfm = false;
	glm::mat4 user_translation(1.f);
	glm::mat4 user_scale(1.f);
	for (int i = 1; i < argc; ++i) {
		if (std::strcmp(argv[i], "-o") == 0) {
			output = argv[++i];
		} else if (std::strcmp(argv[i], "-oxml") == 0) {
			output_xml = argv[++i];
		} else if (std::strcmp(argv[i], "-apply") == 0) {
			apply_file_tfm = true;
		} else if (std::strcmp(argv[i], "-translate") == 0) {
			glm::vec3 v;
			v.x = std::atof(argv[++i]);
			v.y = std::atof(argv[++i]);
			v.z = std::atof(argv[++i]);
			user_translation = glm::translate(v);
		} else if (std::strcmp(argv[i], "-scale") == 0) {
			glm::vec3 v;
			v.x = std::atof(argv[++i]);
			v.y = std::atof(argv[++i]);
			v.z = std::atof(argv[++i]);
			user_scale = glm::scale(v);
		} else {
			input = argv[i];
		}
	}
	if (input.empty() || (output.empty() && output_xml.empty())) {
		std::cout << "Error: an input and output file are needed.\n"
			<< "Usage: ./" << argv[0] << " <input> -o <output> [-oxml <output>]\n";
		return 1;
	}

	NeuronData data = import_file(input);

	// Remove all degree-2 nodes (branches w/o points)
	for (auto &t : data.trees) {
		for (auto &b : t.branches) {
			remove_degree2_nodes(b);
		}
	}

	glm::mat4 file_mat(1);
	if (apply_file_tfm && !data.images.empty()) {
		file_mat = glm::translate(glm::vec3(data.images[0].coord[0], data.images[0].coord[1], data.images[0].coord[2]))
			* glm::scale(glm::vec3(data.images[0].scale[0], data.images[0].scale[1], data.images[0].z_spacing));
	}
	const glm::mat4 tfm = user_translation * user_scale * glm::inverse(file_mat);
	transform_neuron_data(data, tfm);

	/* Write out the SWC file (http://research.mssm.edu/cnic/swc.html)
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
	if (!output.empty()) {
		std::cout << "Exporting transformed and converted SWC file " << output << "\n";
		std::ofstream fout(output.c_str());
		fout << "# Converted from NLXML file " << input << "\n";
		if (data.trees.size() > 1) {
			std::cout << "There should just be one tree!\n";
		}

		size_t point_id = 1;
		for (const auto &t : data.trees) {
			point_id = write_branch_swc(fout, t, -1, point_id);
		}
	}

	if (!output_xml.empty()) {
		std::cout << "Exporting transformed NLXML file " << output_xml << "\n";
		for (auto &img : data.images) {
			img.coord.fill(0.f);
			img.scale.fill(1.f);
			img.z_spacing = 1.f;
		}
		export_file(data, output_xml);
	}
	return 0;
}

