cmake_minimum_required(VERSION 3.5)
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

add_library(nlxml nlxml.cpp tinyxml2.cpp)
set_target_properties(nlxml PROPERTIES
	POSITION_INDEPENDENT_CODE ON
	CXX_STANDARD_REQUIRED 11
)
target_include_directories(nlxml PUBLIC
	$<BUILD_INTERFACE:${nlxml_SOURCE_DIR}>
	$<INSTALL_INTERFACE:include>
)

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

add_subdirectory(utils)

