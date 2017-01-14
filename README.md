# nlxml

Library for importing the Neurolucida XML file format. Also includes a plugin to render the imported
trees and contours in vislight, to build this clone the repo in your vislight plugin directory
and pass `-DNLXML_VL_PLUGIN=ON` when running CMake. You'll also need to copy the two shaders
into vislights resources directory to run the plugin.

