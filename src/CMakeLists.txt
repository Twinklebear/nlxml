add_library(nlxml STATIC nlxml.cpp tinyxml2.cpp ../marker_renderer.cpp)
set_target_properties(nlxml PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_executable(nlxml_inspector nlxml_inspector.cpp)
target_link_libraries(nlxml_inspector nlxml)

