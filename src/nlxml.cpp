#include <string>
#include <iomanip>
#include <algorithm>
#include <iostream>
#include "tinyxml2.h"
#include "nlxml.h"

namespace nlxml {

Point::Point(float x, float y, float z, float d) : x(x), y(y), z(z), d(d) {}
Color::Color(float r, float g, float b) : r(r), g(g), b(b) {}

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
	c.r = std::stoul(str.substr(1, 2), nullptr, 16) / 255.f;
	c.g = std::stoul(str.substr(3, 2), nullptr, 16) / 255.f;
	c.b = std::stoul(str.substr(5, 2), nullptr, 16) / 255.f;
	return c;
}
Marker read_marker(const tinyxml2::XMLElement *e) {
	using namespace tinyxml2;
	Marker m;
	m.type = e->Attribute("type");
	m.color = parse_color(e->Attribute("color"));
	m.name = e->Attribute("name");
	m.varicosity = e->BoolAttribute("varicosity");
	// Go through the markers's children and load all the points
	for (const XMLElement *it = e->FirstChildElement(); it != nullptr; it = it->NextSiblingElement()) {
		if (std::strcmp(it->Name(), "point") == 0) {
			m.points.push_back(read_point(it));
		}
	}
	return m;
}
Contour read_contour(const tinyxml2::XMLElement *e) {
	using namespace tinyxml2;
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
Branch read_branch(const tinyxml2::XMLElement *e) {
	using namespace tinyxml2;
	// TODO: Need to recurse on branches
	Branch b;
	if (e->Attribute("leaf")) {
		b.leaf = e->Attribute("leaf");
	} else {
		b.leaf = "Unspecified";
	}
	for (const XMLElement *it = e->FirstChildElement(); it != nullptr; it = it->NextSiblingElement()) {
		if (std::strcmp(it->Name(), "point") == 0) {
			b.points.push_back(read_point(it));
		} else if (std::strcmp(it->Name(), "marker") == 0) {
			b.markers.push_back(read_marker(it));
		} else if (std::strcmp(it->Name(), "branch") == 0) {
			b.branches.push_back(read_branch(it));
		}
	}
	return b;
}
Tree read_tree(const tinyxml2::XMLElement *e) {
	using namespace tinyxml2;
	Tree t;
	t.color = parse_color(e->Attribute("color"));
	t.type = e->Attribute("type");
	t.leaf = e->Attribute("leaf");
	for (const XMLElement *it = e->FirstChildElement(); it != nullptr; it = it->NextSiblingElement()) {
		if (std::strcmp(it->Name(), "point") == 0) {
			t.points.push_back(read_point(it));
		} else if (std::strcmp(it->Name(), "marker") == 0) {
			t.markers.push_back(read_marker(it));
		} else if (std::strcmp(it->Name(), "branch") == 0) {
			t.branches.push_back(read_branch(it));
		}
	}
	return t;
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
	}
	return data;
}


// exporting to xml
void write_point(const Point &p, tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *const parent) {
	tinyxml2::XMLElement *const e = doc.NewElement("point");
	e->SetAttribute("x", p.x);
	e->SetAttribute("y", p.y);
	e->SetAttribute("z", p.z);
	e->SetAttribute("d", p.d);

	parent->InsertEndChild(e);
}
std::string color_to_string(const Color &c) {
	char buffer[8]; // #RRGGBB hex
	snprintf(buffer, sizeof buffer, "#%02X%02X%02X", (uint8_t)(c.r*255), (uint8_t)(c.g*255), (uint8_t)(c.b*255));
	return std::string(buffer);
}
void write_marker(const Marker &marker, tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *const parent) {
	tinyxml2::XMLElement *const e = doc.NewElement("marker");
	e->SetAttribute("type", marker.type.c_str());
	e->SetAttribute("name", marker.name.c_str());
	e->SetAttribute("color", color_to_string(marker.color).c_str());
	e->SetAttribute("varicosity", marker.varicosity);

	for (auto p : marker.points)
		write_point(p, doc, e);

	parent->InsertEndChild(e);
}
void write_contour(const Contour &contour, tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *const parent) {
	tinyxml2::XMLElement *const e = doc.NewElement("contour");
	e->SetAttribute("name", contour.name.c_str());
	e->SetAttribute("shape", contour.shape.c_str());
	e->SetAttribute("color", color_to_string(contour.color).c_str());
	e->SetAttribute("closed", contour.closed);

	for (auto p : contour.points)
		write_point(p, doc, e);
	for (auto &m : contour.markers)
		write_marker(m, doc, e);

	parent->InsertEndChild(e);
}
void write_branch(const Branch &branch, tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *const parent) {
	tinyxml2::XMLElement *const e = doc.NewElement("branch");
	e->SetAttribute("leaf", branch.leaf.c_str());

	for (auto p : branch.points)
		write_point(p, doc, e);
	for (auto &b : branch.branches)
		write_branch(b, doc, e);
	for (auto &m : branch.markers)
		write_marker(m, doc, e);

	parent->InsertEndChild(e);
}
void write_tree(const Tree &tree, tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *const parent) {
	tinyxml2::XMLElement *const e = doc.NewElement("tree");
	e->SetAttribute("color", color_to_string(tree.color).c_str());
	e->SetAttribute("type", tree.type.c_str());
	e->SetAttribute("leaf", tree.leaf.c_str());

	for (auto p : tree.points)
		write_point(p, doc, e);
	for (auto &b : tree.branches)
		write_branch(b, doc, e);
	for (auto &m : tree.markers)
		write_marker(m, doc, e);

	parent->InsertEndChild(e);
}
void export_file(const NeuronData &data, const std::string &fname) {
	using namespace tinyxml2;
	XMLDocument doc;
	doc.LinkEndChild(doc.NewDeclaration());

	// MBF meta information
	XMLElement *const mbf = doc.NewElement("mbf");
	mbf->SetAttribute("version", "4.0");
	mbf->SetAttribute("xmlns", "http://www.mbfbioscience.com/2007/neurolucida");
	mbf->SetAttribute("xmlns:nl", "http://www.mbfbioscience.com/2007/neurolucida");
	doc.LinkEndChild(mbf);

	for (auto &t : data.trees)
		write_tree(t, doc, mbf);
	for (auto &c : data.contours)
		write_contour(c, doc, mbf);
	for (auto &m : data.markers)
		write_marker(m, doc, mbf);

	doc.SaveFile(fname.c_str());
}

}

std::ostream& operator<<(std::ostream &os, const nlxml::Point &p) {
	os << "Point { " << p.x << ", " << p.y << ", " << p.z
		<< ", " << p.d << " }";
	return os;
}
std::ostream& operator<<(std::ostream &os, const nlxml::Color &c) {
	os << "Color { " << static_cast<int>(c.r) << ", "
		<< static_cast<int>(c.g) << ", "
		<< static_cast<int>(c.b) << " }";
	return os;
}
std::ostream& operator<<(std::ostream &os, const nlxml::Branch &b) {
	os << "Branch { leaf = " << b.leaf << ", #points = " << b.points.size()
		<< ", #markers = " << b.markers.size() << ", #branches = " << b.branches.size()
		<< "\npoints = \n";
	for (const auto &p : b.points) {
		os << p << ", ";
	}
	os << "\nmarkers = \n";
	for (const auto &m : b.markers) {
		os << m << ", ";
	}
	os << "\nbranches = \n";
	for (const auto &i : b.branches) {
		os << i << ", ";
	}
	os << "\n}";
	return os;
}
std::ostream& operator<<(std::ostream &os, const nlxml::Tree &t) {
	os << "Tree { color = " << t.color << ", type = " << t.type
		<< ", #points = " << t.points.size() << ", #markers = " << t.markers.size()
		<< ", #branches = " << t.branches.size()
		<< "\npoints = \n";
	for (const auto &p : t.points) {
		os << p << ", ";
	}
	os << "\nmarkers = \n";
	for (const auto &m : t.markers) {
		os << m << ", ";
	}
	os << "\nbranches = \n";
	for (const auto &b : t.branches) {
		os << b << ", ";
	}
	os << "\n}";
	return os;
}
std::ostream& operator<<(std::ostream &os, const nlxml::Contour &c) {
	os << "Contour { name = " << c.name << ", shape = " << c.shape
		<< "\ncolor = " << c.color << ", closed = " << std::boolalpha
		<< c.closed << "\npoints =\n";
	for (const auto &p : c.points) {
		os << p << ", ";
	}
	os << "\nmarkers =\n";
	for (const auto &m : c.markers) {
		os << m << ", ";
	}
	os << "\n}";
	return os;
}
std::ostream& operator<<(std::ostream &os, const nlxml::Marker &m) {
	os << "Marker { name = " << m.name << ", type = " << m.type
		<< "\ncolor = " << m.color << ", varicosity = " << m.varicosity
		<< "\npoints = \n";
	for (const auto &p : m.points) {
		os << p << ", ";
	}
	os << "\n}";
	return os;
}

