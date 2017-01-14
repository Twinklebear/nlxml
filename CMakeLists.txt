cmake_minimum_required(VERSION 3.0)
project(NLXML)

# Bump up warning levels appropriately for each compiler
if (UNIX OR APPLE OR MINGW)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -std=c++14 -m64 -mf16c")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O3")
elseif (MSVC)
	add_definitions(-DNOMINMAX)
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /Zi")
	endif()
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
endif()

add_subdirectory(src)

if (NLXML_VL_PLUGIN)
	get_plugin_name(plugin_name "nlxml")
	add_library(${plugin_name} MODULE nlxml_plugin.cpp)
	target_link_libraries(${plugin_name} vl nlxml)
	install(TARGETS ${plugin_name} DESTINATION bin)
endif()

