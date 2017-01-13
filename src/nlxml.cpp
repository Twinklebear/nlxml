#include <string>
#include <iostream>
#include "tinyxml2.h"
#include "nlxml.h"

namespace nlxml {

Point read_point(const tinyxml2::XMLElement *e) {
	Point p;
	p.x = e->FloatAttribute("x");
	p.y = e->FloatAttribute("y");
	p.z = e->FloatAttribute("z");
	p.d = e->FloatAttribute("d");
	return p;
}
Color parse_color(const std::string &str) {
	// The color strings in the file are #RRGGBB
	Color c;
	c.r = std::stoul(str.substr(1, 2), nullptr, 16);
	c.g = std::stoul(str.substr(3, 2), nullptr, 16);
	c.b = std::stoul(str.substr(5, 2), nullptr, 16);
	return c;
}
Marker read_marker(const tinyxml2::XMLElement *e) {
	Marker m;
	m.type = e->Attribute("type");
	m.color = parse_color(e->Attribute("color"));
	m.closed = e->BoolAttribute("closed");
	return Marker{};
}
Contour read_contour(const tinyxml2::XMLElement *e) {
	using namespace tinyxml2;
	// TODO: This is just a flat list of points
	Contour c;
	c.name = e->Attribute("name");
	c.color = parse_color(e->Attribute("color"));
	c.closed = e->BoolAttribute("closed");
	c.shape = e->Attribute("shape");
	// Go through the contour's children and load all the points
	for (const XMLElement *it = e->FirstChildElement(); it != nullptr; it = it->NextSiblingElement()) {
		if (std::strcmp(it->Name(), "point") == 0) {
			c.points.push_back(read_point(it));
		} else if (std::strcmp(it->Name(), "marker") == 0) {
			c.markers.push_back(read_marker(it));
		}
	}
	return c;
}
Tree read_tree(const tinyxml2::XMLElement *e) {
	// TODO: Need to recurse on branches
	return Tree{};
}
NeuronData import_file(const std::string &fname) {
	using namespace tinyxml2;
	XMLDocument doc;
	doc.LoadFile(fname.c_str());
	XMLElement *mbf_root = doc.FirstChildElement();
	NeuronData data;
	for (XMLElement *e = mbf_root->FirstChildElement(); e != nullptr; e = e->NextSiblingElement()) {
		if (!e) {
			continue;
		}
		if (std::strcmp(e->Name(), "contour") == 0) {
			data.contours.push_back(read_contour(e));
		} else if (std::strcmp(e->Name(), "tree") == 0) {
			data.trees.push_back(read_tree(e));
		} else if (std::strcmp(e->Name(), "marker") == 0) {
			data.markers.push_back(read_marker(e));
		}
		std::cout << e->Name() << "\n";
	}
	return NeuronData{};
}

}

std::ostream& operator<<(std::ostream &os, const nlxml::Point &p) {
	os << "Point { " << p.x << ", " << p.y << ", " << p.z
		<< ", " << p.d << " }";
	return os;
}
std::ostream& operator<<(std::ostream &os, const nlxml::Color &c) {
	os << "Color { " << c.r << ", " << c.g << ", " << c.g << " }";
	return os;
}

