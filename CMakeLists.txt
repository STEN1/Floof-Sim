cmake_minimum_required (VERSION 3.12)
project (Floof)
set(CMAKE_CXX_STANDARD 20)

add_executable(Floof 
	"Source/Floof.cpp"
	"Source/Floof.h"
	"Source/VulkanRenderer.h"
	"Source/VulkanRenderer.cpp"
	"Source/VulkanAllocator.cpp"
	"Source/Application.h"
	"Source/Application.cpp"
	"Source/Timer.h"
)


find_package(Vulkan REQUIRED)
target_include_directories(Floof PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(Floof ${Vulkan_LIBRARIES})

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("Libs/glfw")
target_link_libraries(Floof glfw)
target_link_libraries(Floof EnTT)

exec_program("$ENV{VULKAN_SDK}/Bin/glslc.exe ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Basic.vert -o ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Basic.vert.spv")
exec_program("$ENV{VULKAN_SDK}/Bin/glslc.exe ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Basic.frag -o ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Basic.frag.spv")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/Shaders
                ${CMAKE_CURRENT_BINARY_DIR}/Shaders)