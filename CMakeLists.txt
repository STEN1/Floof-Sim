cmake_minimum_required (VERSION 3.12)
project (Floof)
set(CMAKE_CXX_STANDARD 20)

add_executable(Floof 
	"Source/Floof.cpp"
	"Source/Floof.h"
	"Source/VulkanRenderer.h"
	"Source/VulkanRenderer.cpp"
 "Source/Application.h" "Source/Application.cpp")

find_package(Vulkan REQUIRED)
target_include_directories(Floof PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(Floof ${Vulkan_LIBRARIES})

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("Libs/glfw")
target_link_libraries(Floof glfw)