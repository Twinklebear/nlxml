add_library(nlxml STATIC nlxml.cpp tinyxml2.cpp ../marker_renderer.cpp)
set_target_properties(nlxml PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_executable(nlxml_inspector nlxml_inspector.cpp)
target_link_libraries(nlxml_inspector nlxml)

if (NLXML_TRANSFORMER)
	add_definitions(-DGLM_FORCE_RADIANS)
	find_package(GLM REQUIRED)
	include_directories(${GLM_INCLUDE_DIRS})
	add_executable(nlxml_transformer nlxml_transformer.cpp)
	target_link_libraries(nlxml_transformer nlxml)
endif()

