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

set( LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/output/lib )
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/output/bin )

file( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/output/include/ )
file( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib )
file( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/output/bin )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_BINARY_DIR}/output/include )
link_directories( ${CMAKE_BINARY_DIR}/output/lib  )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/muqueue/src )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/muflow/src )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/muroute/src )

add_subdirectory( muqueue )
add_subdirectory( muroute )
add_subdirectory( muflow )
add_subdirectory( evioslam )

