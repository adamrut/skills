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

find_program(GLSLC_EXECUTABLE
    NAMES glslc
    HINTS "$ENV{VULKAN_SDK}"
    PATH_SUFFIXES Bin bin
)

if(NOT GLSLC_EXECUTABLE)
    file(GLOB VULKAN_SDK_GLSLC_CANDIDATES "C:/VulkanSDK/*/Bin/glslc.exe")
    if(VULKAN_SDK_GLSLC_CANDIDATES)
        list(SORT VULKAN_SDK_GLSLC_CANDIDATES COMPARE NATURAL ORDER DESCENDING)
        list(GET VULKAN_SDK_GLSLC_CANDIDATES 0 GLSLC_EXECUTABLE)
    endif()
endif()

if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "Vulkan shader compiler glslc was not found")
endif()

add_executable(game WIN32
    src/main.c
    src/app.c
    src/platform_win32.c
    src/vulkan.c
)

target_link_libraries(game PRIVATE Vulkan::Vulkan)

set(SHADER_OUTPUT_DIR "$<TARGET_FILE_DIR:game>/shaders")
add_custom_command(
    TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_OUTPUT_DIR}"
    COMMAND "${GLSLC_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/cube.vert" -o "${SHADER_OUTPUT_DIR}/cube.vert.spv"
    COMMAND "${GLSLC_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/cube.frag" -o "${SHADER_OUTPUT_DIR}/cube.frag.spv"
    COMMENT "Compiling Vulkan cube shaders"
)

target_compile_definitions(game PRIVATE
    $<$<CONFIG:Debug>:GAME_ENABLE_VULKAN_VALIDATION=1>
)

if(MSVC)
    target_compile_options(game PRIVATE /W4 /WX)
else()
    target_compile_options(game PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

enable_testing()
add_executable(game_foundation_test
    tests/foundation_test.c
    src/app.c
)

if(MSVC)
    target_compile_options(game_foundation_test PRIVATE /W4 /WX)
else()
    target_compile_options(game_foundation_test PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_test(NAME game_foundation COMMAND game_foundation_test)

add_executable(game_vulkan_test
    tests/vulkan_test.c
    src/vulkan.c
)

target_link_libraries(game_vulkan_test PRIVATE Vulkan::Vulkan)

if(MSVC)
    target_compile_options(game_vulkan_test PRIVATE /W4 /WX)
else()
    target_compile_options(game_vulkan_test PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

target_compile_definitions(game_vulkan_test PRIVATE
    $<$<CONFIG:Debug>:GAME_ENABLE_VULKAN_VALIDATION=1>
)

add_test(NAME game_vulkan COMMAND game_vulkan_test)
