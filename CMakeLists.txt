cmake_minimum_required (VERSION 3.10)
project (imgmatch)
set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)
add_executable(imgmatch main.cpp)
set (imgmatch_VERSION_MAJOR 0)
set (imgmatch_VERSION_MINOR 9)
target_sources(imgmatch PUBLIC
	image_hist.cpp image_matcher.cpp lodepng.cpp read_bmp.cpp read_jpeg.cpp read_png.cpp
)
configure_file (
	"${PROJECT_SOURCE_DIR}/imgmatch_config.h.in"
	"${PROJECT_SOURCE_DIR}/imgmatch_config.h"
)
include_directories("$(PROJECT_BINARY_DIR)" /usr/local/include)
find_library(BOOST_FILESYS boost_filesystem PATHS /usr/local/lib)
find_library(BOOST_SYSTEM boost_system PATHS /usr/local/lib)
find_library(BOOST_PROG_OPT boost_program_options PATHS /usr/local/lib)
find_library(LIBJPEG jpeg PATHS /usr/local/lib)
target_link_libraries(imgmatch PUBLIC
	${BOOST_FILESYS}
	${BOOST_SYSTEM}
	${BOOST_PROG_OPT}
	${LIBJPEG})
