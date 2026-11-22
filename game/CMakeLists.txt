cmake_minimum_required(VERSION 3.20)

project(vulkan_isometric_rpg LANGUAGES C)

if(DEFINED ENV{VULKAN_SDK})
    set(Vulkan_ROOT "$ENV{VULKAN_SDK}")
elseif(EXISTS "C:/VulkanSDK")
    file(GLOB VULKAN_SDK_VERSIONS "C:/VulkanSDK/*")
    if(VULKAN_SDK_VERSIONS)
        list(SORT VULKAN_SDK_VERSIONS COMPARE NATURAL ORDER DESCENDING)
        list(GET VULKAN_SDK_VERSIONS 0 Vulkan_ROOT)
    endif()
endif()

find_package(Vulkan REQUIRED)

add_executable(game src/main.c)

target_link_libraries(game PRIVATE Vulkan::Vulkan)

if(MSVC)
    target_compile_options(game PRIVATE /W4 /WX)
else()
    target_compile_options(game PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
