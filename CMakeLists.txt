cmake_minimum_required(VERSION 2.8)

project(snake)
add_executable(${PROJECT_NAME} "main.c" "wingl.h" "sound_xm.h" "ufmod.h" "music.xm.h")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mwindows  -O3 -s ") # -mwindows чтоб убрать окно консоли
# OpenGL
find_package(OpenGL 2.1 REQUIRED )
include_directories( ${OPENGL_INCLUDE_DIR}  )
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} )
# ufmod
target_link_libraries(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/lib/ufmod.obj "winmm")



