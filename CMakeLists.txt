cmake_minimum_required (VERSION 3.12)
project (Floof)
set(CMAKE_CXX_STANDARD 20)

add_executable(Floof "Floof.cpp" "Floof.h" "Utils/Utils.h" "Utils/Utils.cpp" "Utils/Math.h" "Utils/Math.cpp")

find_package(Vulkan REQUIRED)
target_include_directories(Floof PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(Floof ${Vulkan_LIBRARIES})