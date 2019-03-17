cmake_minimum_required(VERSION 3.13)
project(nlxml)

# Bump up warning levels appropriately for each compiler
if (UNIX OR APPLE OR MINGW)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
elseif (MSVC)
	add_definitions(-DNOMINMAX)
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /Zi")
	endif()
endif()

# Python Bindings
find_package(SWIG 3.0.8 REQUIRED)
include(${SWIG_USE_FILE})
cmake_policy(SET CMP0078 NEW)

find_package(Python3 3.6 COMPONENTS Interpreter Development REQUIRED)
include_directories(SYSTEM ${Python3_INCLUDE_DIRS})

add_library(nlxml nlxml.cpp tinyxml2.cpp)
set_target_properties(nlxml PROPERTIES
	POSITION_INDEPENDENT_CODE ON
	CXX_STANDARD_REQUIRED 11
)
target_include_directories(nlxml PUBLIC
	$<BUILD_INTERFACE:${nlxml_SOURCE_DIR}>
	$<INSTALL_INTERFACE:include>
)

set (
    LIBRARIES
    ${Python3_LIBRARIES}
    nlxml
)

set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/NLXMLBindings.i PROPERTY CPLUSPLUS ON)
set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/NLXMLBindings.i PROPERTY USE_TARGET_INCLUDE_DIRECTORIES TRUE)
swig_add_library(NLXMLBindings TYPE SHARED LANGUAGE python OUTFILE_DIR ${CMAKE_CURRENT_SOURCE_DIR} SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/NLXMLBindings.i)
set_target_properties(NLXMLBindings PROPERTIES INSTALL_RPATH "${RPATHS}")
set_target_properties(NLXMLBindings PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
target_link_libraries(NLXMLBindings PUBLIC ${LIBRARIES})

install(TARGETS nlxml EXPORT nlxmlConfig
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
	INCLUDES DESTINATION include
)
install(FILES nlxml.h
	DESTINATION include
)
install(EXPORT nlxmlConfig
	DESTINATION lib/cmake/nlxml
)

# Install
install(FILES ${CMAKE_BINARY_DIR}/NLXMLBindings.py DESTINATION ${CMAKE_INSTALL_PREFIX})

if (WIN32) 
install(FILES ${CMAKE_BINARY_DIR}/_NLXMLBindings.pyd DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_BINARY_DIR}/_NLXMLBindings.pdb DESTINATION ${CMAKE_INSTALL_PREFIX} OPTIONAL)
elseif(APPLE)
install(FILES ${CMAKE_BINARY_DIR}/_NLXMLBindings.so DESTINATION ${CMAKE_INSTALL_PREFIX})
else()
install(FILES ${CMAKE_BINARY_DIR}/_NLXMLBindings.so DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()

