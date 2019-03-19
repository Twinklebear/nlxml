find_package(glm REQUIRED)
add_definitions(-DGLM_FORCE_RADIANS -DGLM_ENABLE_EXPERIMENTAL)
include_directories(${GLM_INCLUDE_DIRS})

add_executable(nlxml_inspector nlxml_inspector.cpp)
set_property(TARGET nlxml_inspector PROPERTY CXX_STANDARD 14)
target_link_libraries(nlxml_inspector nlxml)

add_executable(nlxml_transformer nlxml_transformer.cpp)
set_property(TARGET nlxml_transformer PROPERTY CXX_STANDARD 14)
target_link_libraries(nlxml_transformer nlxml)

add_executable(nlxml_simplifier nlxml_simplifier.cpp)
set_property(TARGET nlxml_simplifier PROPERTY CXX_STANDARD 14)
target_link_libraries(nlxml_simplifier nlxml)

add_executable(nlxml_diadem_missed nlxml_diadem_missed.cpp)
set_property(TARGET nlxml_diadem_missed PROPERTY CXX_STANDARD 14)
target_link_libraries(nlxml_diadem_missed nlxml)

add_executable(nlxml_to_swc nlxml_to_swc.cpp)
set_property(TARGET nlxml_to_swc PROPERTY CXX_STANDARD 14)
target_link_libraries(nlxml_to_swc nlxml)

add_executable(swc_to_nlxml swc_to_nlxml.cpp)
set_property(TARGET swc_to_nlxml PROPERTY CXX_STANDARD 14)
target_link_libraries(swc_to_nlxml nlxml)

