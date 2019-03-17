%module NLXMLBindings

%feature("autodoc","3");

%include "std_string.i"
%include "std_vector.i"
%include "stdint.i"

%include "exception.i"

%exception {
  try {
    $action
  } catch (const std::exception& e) {
    SWIG_exception(SWIG_RuntimeError, e.what());
  }
}

%{
#include "./nlxml.h"
using namespace nlxml;
%}

namespace std {
   %template(UIntVector) vector<uint32_t>;
   %template(FloatVector) vector<float>;

   %template(PointVector) vector<nlxml::Point>;
   %template(ColorVector) vector<nlxml::Color>;
   %template(TreeVector) vector<nlxml::Tree>;
   %template(BranchVector) vector<nlxml::Branch>;
   %template(ContourVector) vector<nlxml::Contour>;
   %template(MarkerVector) vector<nlxml::Marker>;
   %template(ImageVector) vector<nlxml::Image>;
};

%include "./nlxml.h"