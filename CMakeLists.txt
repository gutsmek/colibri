project( Colibri )
cmake_minimum_required( VERSION 3.1 )

LIST( APPEND CMAKE_MODULE_PATH ~/staging_dir/lib/cmake)
LIST( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
LIST( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
LIST( APPEND CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
        /opt/ros/melodic/share/catkin/cmake/
        /opt/ros/melodic/share/rosbag/cmake/)

include( CheckIncludeFile )
include( CheckSymbolExists )
include( CheckFunctionExists )
include( CPack )
include( CTest )

# debug compilation time and mem
#add_compile_options("-v" "-ftime-report") # "-Q"

# gprof support
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
#SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")

set( LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/output/lib )
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/output/bin )

file( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/output/include )
file( MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH} )
file( MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH} )

link_directories( ${LIBRARY_OUTPUT_PATH} )

include_directories( ${CMAKE_BINARY_DIR}/output/include )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/muconfig/include )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/muqueue/include )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/muflow/include )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/muroute/include )

add_subdirectory( muconfig )
add_subdirectory( muqueue )
add_subdirectory( muroute )
add_subdirectory( muflow )
add_subdirectory( evioslam )
