cmake_minimum_required(VERSION 2.8)

cmake_policy(SET CMP0015 NEW)
cmake_policy(SET CMP0072 OLD)

project(BaseCode)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

execute_process(COMMAND ln -s ../shaders)

set(appName BaseCode)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)

include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${GLUT_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIRS})
include_directories("${CMAKE_SOURCE_DIR}/glm")

link_directories(${OPENGL_LIBRARY_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})
link_directories(${GLEW_LIBRARY_DIRS})

file(GLOB ImGUI_SRC
     "ImGUI/*.h"
     "ImGUI/*.cpp"
)

add_executable(${appName} Utils.h Utils.cpp PLYReader.h PLYReader.cpp TriangleMesh.h TriangleMesh.cpp Camera.h Camera.cpp Node.h Node.cpp Scene.h Scene.cpp Shader.h Shader.cpp ShaderProgram.h ShaderProgram.cpp Application.h Application.cpp main.cpp ${ImGUI_SRC})
                
#ImGUI/imgui_demo.cpp ImGUI/imgui_draw.cpp ImGUI/imgui_demo.cpp ImGUI/imgui_impl_glut.cpp ImGUI/imgui_impl_opengl3.cpp ImGUI/imgui_tables.cpp
 #               ImGUI/imgui_widgets.cpp ImGUI/imgui.cpp)

target_link_libraries(${appName} ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${GLEW_LIBRARIES})



