cmake_minimum_required(VERSION 3.0.0)
project(Geometry-Processing VERSION 0.1.0)

#Use VCPKG and force x64
set(CMAKE_TOOLCHAIN_FILE "D:/dev/Extras/vcpkg/scripts/buildsystems/vcpkg.cmake")
include("D:/dev/Extras/vcpkg/scripts/buildsystems/vcpkg.cmake")
set(-DUSE64bit=true)

include(Ctest)
enable_testing()


set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


file(GLOB_RECURSE files "Source/*.cpp" "Source/*.h" "Source/*.c""Source/TuranAPI/*.cpp" "Source/TuranAPI/*.h" "Source/TuranAPI/*.c")

add_library(RecDLL SHARED ${files})
target_include_directories(RecDLL PRIVATE ${CMAKE_SOURCE_DIR}/turanapi/turanapi ${CMAKE_SOURCE_DIR}/tgfx/core)
set_target_properties(RecDLL PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)
set_target_properties(RecDLL PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release PREFIX "")

find_package(glm CONFIG REQUIRED)
target_link_libraries(RecDLL PRIVATE glm::glm)

find_package(assimp CONFIG REQUIRED)
target_link_libraries(RecDLL PRIVATE assimp::assimp)

add_executable(Test  "RECTEST.cpp")
