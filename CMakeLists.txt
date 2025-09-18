cmake_minimum_required (VERSION 3.13)
set(CMAKE_CXX_STANDARD 17) 
project(Vulkan-Layer)

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB HEADERS ${CMAKE_SOURCE_DIR}/src/*.h)
file(GLOB JSON ${CMAKE_SOURCE_DIR}/json/*.json)

add_library(${PROJECT_NAME} SHARED 
    ${SOURCES}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME  "VkLayer_lmj_test")

# load vulkan
find_package(Vulkan REQUIRED)
if(VULKAN_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARY})
    message(STATUS "Vulkan Version = ${Vulkan_VERSION}。")
else(VULKAN_FOUND)
    message(FATAL_ERROR ”VULKAN library not found”)
endif(VULKAN_FOUND)
