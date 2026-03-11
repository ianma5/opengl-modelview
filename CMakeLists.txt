cmake_minimum_required(VERSION 4.1)
project(opengl)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)


add_executable(opengl
        main.cpp
        importobj.cpp
        importobj.h)

target_link_libraries(opengl PRIVATE
        OpenGL::GL
        GLEW::GLEW
        glfw
)