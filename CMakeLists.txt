cmake_minimum_required(VERSION 3.25)
project(ggpo-py)

add_subdirectory(pybind11)


# Set the GGPO_BUILD_VECTORWAR option to OFF to exclude building the Vector War example app
set(GGPO_BUILD_VECTORWAR OFF CACHE BOOL "Enable the build of the Vector War example app" FORCE)
# Set the BUILD_SHARED_LIBS option to OFF to build GGPO as a static library
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Enable the build of shared libraries instead of static ones" FORCE)

add_subdirectory(ggpo)

set(CMAKE_CXX_STANDARD 17)
find_package(Python 3.11 COMPONENTS Interpreter Development REQUIRED)
add_definitions(-DPYBIND11_EXCEPTIONS)

pybind11_add_module(ggpo_py src/main.cpp src/py_ggposession.cpp)

if(WIN32)
    target_link_libraries(GGPO PUBLIC winmm.lib ws2_32.lib)
endif()

target_link_libraries(ggpo_py PRIVATE GGPO)