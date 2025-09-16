cmake_minimum_required (VERSION 3.13)
set(CMAKE_CXX_STANDARD 17) 
project(Vulkan-Layer)

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB HEADERS ${CMAKE_SOURCE_DIR}/src/*.h)
 
add_executable(${PROJECT_NAME}
    ${HEADERS}
    ${SOURCES}
)

# add_library(my_custom_layer SHARED ${CMAKE_SOURCE_DIR}/src/vk_layer_lmj_test.cpp)

set_target_properties(Vulkan-Layer PROPERTIES
    OUTPUT_NAME "VkLayer_lmj_test"  
    PREFIX ""                     
    SUFFIX ""                    
)

# load vulkan
find_package(Vulkan REQUIRED)
if(VULKAN_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARY})
    message(STATUS "Vulkan Version = ${Vulkan_VERSION}。")
else(VULKAN_FOUND)
    message(FATAL_ERROR ”VULKAN library not found”)
endif(VULKAN_FOUND)