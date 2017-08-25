#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

namespace nlxml {

struct Point {
	float x, y, z, d;

	Point(float x = 0, float y = 0, float z = 0, float d = 0);
};

struct Color {
	float r, g, b;

	Color(float r = 0, float g = 0, float b = 0);
};

struct Marker {
	// TODO: There's a fixed # of marker types which specify which glyph
	// should be displayed.
	std::string type;
	std::string name;
	Color color;
	// Varicosity is a specific type of synaps so you may want to tag a marker
	// as referring to a varicosity specifically.
	bool varicosity;
	// Each point in the list is the center point of a location that the marker
	// is placed at.
	std::vector<Point> points;
};

struct Branch {
	// Leaf attrib is the ending type of the branch.
	// One of: Normal, High, Low, Incomplete, Origin Midpoint
	std::string leaf;
	std::vector<Point> points;
	std::vector<Marker> markers;
	std::vector<Branch> branches;
};

struct Tree {
	Color color;
	// TODO: enum for the different types? Do we know all the different types?
	// Type is the type of neuron tracing, one of: Cell Body, Dendrite, Apical Dendrite, Axon
	// Except that cell bodies are in the file as contours, not trees.
	std::string type;
	// TODO: What does the leaf attrib mean?
	// One of: Normal, High, Low, Incomplete, Origin Midpoint
	std::string leaf;
	std::vector<Point> points;
	std::vector<Branch> branches;
	std::vector<Marker> markers;
};

struct Contour {
	std::string name, shape;
	Color color;
	bool closed;
	// TODO: Do we care about the <property>'s in the file?
	std::vector<Point> points;
	// TODO: Can a contour have markers? Probably
	std::vector<Marker> markers;
};

struct Image {
	struct { double x, y; } scale;
	struct { double x, y, z; } coord;
	struct { double z; } zspacing;
};

struct NeuronData {
	std::vector<Image> images;
	std::vector<Tree> trees;
	std::vector<Contour> contours;
	std::vector<Marker> markers;
};

NeuronData import_file(const std::string &fname);

void export_file(const NeuronData &data, const std::string &fname);

}

std::ostream& operator<<(std::ostream &os, const nlxml::Point &p);
std::ostream& operator<<(std::ostream &os, const nlxml::Color &c);
std::ostream& operator<<(std::ostream &os, const nlxml::Branch &b);
std::ostream& operator<<(std::ostream &os, const nlxml::Tree &t);
std::ostream& operator<<(std::ostream &os, const nlxml::Contour &c);
std::ostream& operator<<(std::ostream &os, const nlxml::Marker &m);

