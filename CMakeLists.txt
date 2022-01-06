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


file(GLOB_RECURSE files "Source/*.cpp" "Source/*.h" "Source/*.c")
list(REMOVE_ITEM "D:/dev/Geometry-Processing/build/CMakeFiles/3.22.1/CompilerIdC/CMakeCCompilerId.c" "D:/dev/Geometry-Processing/build/CMakeFiles/3.22.1/CompilerIdCXX/CMakeCXXCompilerId.cpp" ${files})

add_executable(Geometry-Processing ${files})
target_include_directories(Geometry-Processing PUBLIC ${CMAKE_SOURCE_DIR}/Source)
set_target_properties(Geometry-Processing PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug PREFIX "")
set_target_properties(Geometry-Processing PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release PREFIX "")

add_compile_definitions(SOURCE_DIR="${CMAKE_SOURCE_DIR}")

find_package(assimp CONFIG REQUIRED)
target_link_libraries(Geometry-Processing PRIVATE assimp::assimp)
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(Geometry-Processing PRIVATE glfw)
find_package(glad CONFIG REQUIRED)
target_link_libraries(Geometry-Processing PRIVATE glad::glad)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)