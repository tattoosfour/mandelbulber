cmake_minimum_required(VERSION 2.6)
project(MANDELBULBER)

include(FindPkgConfig)

# dependencies for mandelbulber executable
set(MANDELBULBER_SRCS
algebra.cpp    common_math.cpp  fractal.cpp    settings.cpp  timeline.cpp
callbacks.cpp  database.cpp     image.cpp      morph.cpp      shaders.cpp   undo.cpp
cimage.cpp     files.cpp        interface.cpp  Render3D.cpp   texture.cpp
)

# # to change the build type, run cmake with the parameter:
#   -D CMAKE_BUILD_TYPE=<build-type>
# run "cmake --help-variable CMAKE_BUILD_TYPE" for details

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native -Wall -fmessage-length=0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -mfpmath=both -ffast-math -fno-math-errno -ffinite-math-only -fno-trapping-math -fno-signed-zeros")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -pg")

# find the libraries and include them
pkg_check_modules(GTK2 REQUIRED gtk+-2.0)
include_directories(${GTK2_INCLUDE_DIRS})
link_libraries(${GTK2_LIBRARIES})

pkg_check_modules(GTHREAD REQUIRED gthread-2.0)
include_directories(${GTHREAD_INCLUDE_DIRS})
link_libraries(${GTHREAD_LIBRARIES})

pkg_check_modules(SND sndfile)
include_directories(${SND_INCLUDE_DIRS})
link_libraries(${SND_LIBRARIES})

find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
link_libraries(${JPEG_LIBRARIES} ${PNG_LIBRARIES})

# build the executable
add_executable(mandelbulber ${MANDELBULBER_SRCS})
target_link_libraries(mandelbulber ${LINK_LIBRARIES_ALL})

# installation instructions
install(PROGRAMS ${MANDELBULBER_BINARY_DIR}/mandelbulber DESTINATION bin)
