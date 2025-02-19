cmake_minimum_required(VERSION 3.7.2)
set (CMAKE_CXX_STANDARD 17)

set (PROJECT_NAME "Island-ComputeExample")

project (${PROJECT_NAME})

# Point this to the base directory of your Island installation
set (ISLAND_BASE_DIR "${PROJECT_SOURCE_DIR}/../../../")

# Select which standard Island modules to use
set(REQUIRES_ISLAND_LOADER ON )
set(REQUIRES_ISLAND_CORE ON )

# Loads Island framework, based on selected Island modules from above
include ("${ISLAND_BASE_DIR}CMakeLists.txt.island_prolog.in")

# Add application module, and (optional) any other private
# island modules which should not be part of the shared framework.
add_subdirectory (compute_example_app)

# Specify any optional modules from the standard framework here
add_island_module(le_pipeline_builder)
add_island_module(le_camera)
add_island_module(le_mesh_generator)

# Main application c++ file. Not much to see there,
set (SOURCES main.cpp)

# Sets up Island framework linkage and housekeeping, based on user selections
include ("${ISLAND_BASE_DIR}CMakeLists.txt.island_epilog.in")

# create a link to local resources
link_resources(${PROJECT_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/local_resources)
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
        