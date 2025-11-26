cmake_minimum_required(VERSION 3.16)
project(sistema_solar)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -DGL_SILENCE_DEPRECATION ")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)

add_library(glad STATIC src/glad.c)
target_include_directories(glad PUBLIC include)

file(GLOB ALL_SRCS "src/**.cpp")

set(COMMON_SRCS)
set(MAIN_SRCS)

foreach(SRC_FILE ${ALL_SRCS})
  file(READ ${SRC_FILE} FILE_CONTENT)
  string(FIND "${FILE_CONTENT}" "int main" HAS_MAIN)

  if(HAS_MAIN GREATER_EQUAL 0)
    list(APPEND MAIN_SRCS ${SRC_FILE})
  else()
    list(APPEND COMMON_SRCS ${SRC_FILE})
  endif()
endforeach()

foreach(MAIN_FILE ${MAIN_SRCS})
  get_filename_component(EXE_NAME ${MAIN_FILE} NAME_WE)

  message(STATUS "Criando executável: ${EXE_NAME} a partir de ${MAIN_FILE}")

  add_executable(${EXE_NAME} ${COMMON_SRCS} ${MAIN_FILE})
  target_link_libraries(${EXE_NAME} OpenGL::GL glfw assimp::assimp glad)
  target_compile_definitions(${EXE_NAME} PRIVATE STB_IMAGE_IMPLEMENTATION)
endforeach()

list(LENGTH MAIN_SRCS NUM_MAINS)
if(NUM_MAINS EQUAL 0)
message(WARNING "Nenhum arquivo com main() encontrado")
endif()
