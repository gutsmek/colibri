project( muconfig )
cmake_minimum_required( VERSION 3.1 )

set(VERSION 0.2.2)
set(SOVERSION 0.2)

set(CPACK_PACKAGE_NAME lib${PROJECT_NAME})

LIST( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include( CheckIncludeFile )
include( CheckSymbolExists )
include( CheckFunctionExists )
#include( format )

set( BUILD_SHARED_LIBS ON )

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )

set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -gdwarf-3 -DDEBUG" )
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3 -DNDEBUG -Wextra -Wno-deprecated-declarations -Wold-style-cast")

set( CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -gdwarf-3 -DDEBUG" )
set( CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3 -DNDEBUG -Wextra -Wno-deprecated-declarations -Wold-style-cast")

set( HEADER_OUTPUT_PATH ${CMAKE_BINARY_DIR}/output/include )
set( LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/output/lib )
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/output/bin )

file( MAKE_DIRECTORY ${HEADER_OUTPUT_PATH} )
file( MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})
file( MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH} )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory( src )
add_subdirectory( tests )

set(COMPONENT_NAME ${PROJECT_NAME})
set(COMPONENT_NAME_BOLD MUCONFIG)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/${GLOB_PACKAGE_NAME}-${COMPONENT_NAME}-config.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${GLOB_PACKAGE_NAME}-${COMPONENT_NAME}-config.cmake DESTINATION lib/${GLOB_PACKAGE_NAME}-${VERSION}/ COMPONENT ${COMPONENT_NAME}-dev)
