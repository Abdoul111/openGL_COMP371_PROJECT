cmake_minimum_required(VERSION 3.0)
project(openGL_COMP371_PROJECT)

include(ExternalProject)

# Set install directory
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/dist CACHE PATH ${CMAKE_SOURCE_DIR}/dist FORCE)
if(WIN32)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Debug;Release" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(OpenGL REQUIRED COMPONENTS OpenGL)

include(BuildGLEW)
include(BuildGLFW)
include(BuildGLM)

# lab02
set(EXEC main)

file(GLOB SRC src/*.cpp)

add_executable(${EXEC} ${SRC} src/lib/Camera.h src/lib/shader.h src/lib/shader_m.h src/lib/stb_image.h src/vao.cpp src/vao.h)

target_link_libraries(${EXEC} OpenGL::GL glew_s glfw glm)

list(APPEND BIN ${EXEC})
# end lab02

# install files to install location
install(TARGETS ${BIN} DESTINATION ${CMAKE_INSTALL_PREFIX})

