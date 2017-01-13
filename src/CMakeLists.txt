add_library(nlxml STATIC nlxml.cpp tinyxml2.cpp)

add_executable(nlxml_inspector nlxml_inspector.cpp)
target_link_libraries(nlxml_inspector nlxml)

