#include <iostream>
#include <algorithm>
#include <functional>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "nlxml.h"

using namespace nlxml;

template<typename F>
void transform_neuron_data(nlxml::NeuronData &inout, const F &transform) {
	auto transform_points = [&](auto &points) {
		std::transform(std::begin(points), std::end(points), std::begin(points), transform);
	};

	auto transform_markers = [&](auto &markers) {
		for (auto &m : markers) {
			transform_points(m.points);
		}
	};

	std::function<void(nlxml::Branch &)> transform_branch = [&](nlxml::Branch &b) {
		transform_points(b.points);
		transform_markers(b.markers);
		for (auto &child_b : b.branches) {
			transform_branch(child_b);
		}
	};

	for (auto &t : inout.trees) {
		transform_points(t.points);

		for (auto &b : t.branches) {
			transform_branch(b);
		}

		transform_markers(t.markers);
	}

	transform_markers(inout.markers);
}

/* This program will take an NLXML file and apply its image transform
 * to all its points, taking its transform to identity.
 *
 * the -to-space flag can be used to bring you into another file's space,
 * by taking a the input file (with an assumed identity transform) and
 * applying the inverse of the to space file's transform to its points
 * and setting the to space file's transform as the input file's transform
 *
 * the -make-nl-start will generate a starting point neurolucida file we
 * can send to Fred, with a single marker at the starting point instead of
 * other points.
 *
 * the -flip-z will flip the z coordinates of all points in the file
 */
int main(int argc, char **argv) {
	std::string input, output, to_space, apply;
	bool make_nl_start = false;
	bool flip_z = false;
	for (int i = 1; i < argc; ++i) {
		if (std::strcmp(argv[i], "-o") == 0) {
			output = argv[++i];
		} else if (std::strcmp(argv[i], "-to-space") == 0) {
			to_space = argv[++i];
		} else if (std::strcmp(argv[i], "-apply") == 0) {
			apply = argv[++i];
		} else if (std::strcmp(argv[i], "-h") == 0) {
			std::cout << "Usage: ./nlxml_transformer <input> -o <output> [-to-space <file>] [-make-nl-start]"
				<< " [-apply <file>]\n"
				<< "\t-to-space will transform the input into the space of the specified file\n"
				<< "\t-apply will take the transform from the file and apply it to this one\n"
				<< "\t-make-nl-start will turn the first point on the tree in the file into a marker\n"
				<< "\t-flip-z will flip the z coordinates of all points\n";
			return 0;
		} else if (std::strcmp(argv[i], "-make-nl-start") == 0) {
			make_nl_start = true;
		} else if (std::strcmp(argv[i], "-flip-z") == 0) {
			flip_z = true;
		} else {
			input = argv[i];
		}
	}
	if (input.empty() || output.empty()) {
		std::cout << "Error: an input and output file are needed.\n"
			<< "Usage: ./nlxml_transformer <input> -o <output> [-to-space <file>]\n";
		return 1;
	}
	if (!to_space.empty() && !apply.empty()) {
		std::cout << "Error: apply and to space are mutually exclusive!\n";
		return 1;
	}

	NeuronData data = import_file(input);
	if (apply.empty() && to_space.empty() && data.images.empty()) {
		std::cout << "Warning: did not find transform data in '" << input << "'\n";
		export_file(data, output);
		return 0;
	}
	if (make_nl_start && (data.trees.empty() || data.trees[0].points.empty())) {
		std::cout << "Error: no trees in file to make start point from\n";
		return 1;
	}

	const float z_scale = flip_z ? -1.f : 1.f;

	NeuronData to_data;
	glm::mat4 mat(1);
	if (!to_space.empty()) {
		to_data = import_file(to_space);
		// TODO: are these transforms correct in general? Some files have a transform
		// already, so ignoring it is incorrect.
		glm::mat4 from_mat(1);
#if 0
		if (!data.images.empty()) {
			from_mat = glm::translate(glm::vec3(data.images[0].coord[0],
						data.images[0].coord[1], data.images[0].coord[2]))
				* glm::scale(glm::vec3(data.images[0].scale[0], data.images[0].scale[1],
							data.images[0].z_spacing * z_scale));
		}
#endif
		glm::mat4 to_mat(1);
		if (!to_data.images.empty()) {
			to_mat = glm::translate(glm::vec3(to_data.images[0].coord[0],
						to_data.images[0].coord[1], to_data.images[0].coord[2]))
				* glm::scale(glm::vec3(to_data.images[0].scale[0], to_data.images[0].scale[1],
							to_data.images[0].z_spacing * z_scale));
		}
		// Shouldn't we apply to_mat, not its inverse? or apply from mat
		// not its inverse?
		mat = glm::inverse(to_mat) * glm::inverse(from_mat);
	} else if (!apply.empty()) {
		NeuronData ap = import_file(apply);
		mat = glm::translate(glm::vec3(ap.images[0].coord[0], ap.images[0].coord[1], ap.images[0].coord[2]))
			* glm::scale(glm::vec3(ap.images[0].scale[0], ap.images[0].scale[1],
						ap.images[0].z_spacing * z_scale));
	} else {
		mat = glm::translate(glm::vec3(data.images[0].coord[0], data.images[0].coord[1], data.images[0].coord[2]))
			* glm::scale(glm::vec3(data.images[0].scale[0], data.images[0].scale[1], data.images[0].z_spacing * z_scale));
	}

	transform_neuron_data(data,
		[mat](const Point &p) {
			auto a = mat * glm::vec4(p.x, p.y, p.z, 1.f);
			return nlxml::Point(a.x, a.y, a.z, p.d);
		});

	if (!to_space.empty()) {
		data.images = to_data.images;
	} else if (!data.images.empty()) {
		// Set the transform to identity now that we've applied it
		data.images[0].coord.fill(0.f);
		data.images[0].scale.fill(1.f);
	}

	if (make_nl_start) {
		Marker start_pt = Marker{"FilledCircle", "Start", Color(1, 1, 1), false, {data.trees[0].points[0]}};
		data.trees.clear();
		data.markers.clear();
		data.contours.clear();
		data.images.clear();
		data.markers.push_back(start_pt);
	}

	export_file(data, output);
	return 0;
}

