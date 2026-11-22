#include "../src/vulkan.h"

#include <stdio.h>

static int expect_true(int condition, const char *message)
{
    if (!condition)
    {
        fprintf(stderr, "FAIL: %s\n", message);
        return 1;
    }
    return 0;
}

int main(void)
{
    int failures = 0;
    VkQueueFamilyProperties queue_families[2];
    VkSurfaceFormatKHR formats[2];
    VkPresentModeKHR modes[2];
    VkSurfaceFormatKHR selected_format;
    VkPresentModeKHR selected_mode;
    uint32_t graphics_index = VK_QUEUE_FAMILY_IGNORED;
    uint32_t present_index = VK_QUEUE_FAMILY_IGNORED;
    VkFormat depth_formats[2];
    VkFormatFeatureFlags depth_features[2];
    VkFormat selected_depth;
    uint32_t in_flight;
    float perspective[16];
    float model[16];

    queue_families[0].queueFlags = VK_QUEUE_COMPUTE_BIT;
    queue_families[0].queueCount = 1;
    queue_families[0].timestampValidBits = 0;
    queue_families[0].minImageTransferGranularity.width = 0;
    queue_families[0].minImageTransferGranularity.height = 0;
    queue_families[0].minImageTransferGranularity.depth = 0;
    queue_families[1].queueFlags = VK_QUEUE_GRAPHICS_BIT;
    queue_families[1].queueCount = 1;
    queue_families[1].timestampValidBits = 0;
    queue_families[1].minImageTransferGranularity.width = 0;
    queue_families[1].minImageTransferGranularity.height = 0;
    queue_families[1].minImageTransferGranularity.depth = 0;

    failures |= expect_true(
        vulkan_find_queue_indices(queue_families, 2,
                                  &graphics_index,
                                  &present_index) &&
            graphics_index == 1 &&
            present_index == 1,
        "queue selection chooses the graphics/present family");

    formats[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    formats[0].colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    formats[1].format = VK_FORMAT_B8G8R8A8_SRGB;
    formats[1].colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    failures |= expect_true(
        vulkan_choose_surface_format(formats, 2, &selected_format) &&
            selected_format.format == VK_FORMAT_B8G8R8A8_SRGB,
        "surface format selection prefers B8G8R8A8 SRGB");

    modes[0] = VK_PRESENT_MODE_FIFO_KHR;
    modes[1] = VK_PRESENT_MODE_MAILBOX_KHR;
    failures |= expect_true(
        vulkan_choose_present_mode(modes, 2, &selected_mode) &&
            selected_mode == VK_PRESENT_MODE_MAILBOX_KHR,
        "present mode selection prefers mailbox then FIFO");

    depth_formats[0] = VK_FORMAT_D24_UNORM_S8_UINT;
    depth_formats[1] = VK_FORMAT_D32_SFLOAT;
    depth_features[0] = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
    depth_features[1] = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    failures |= expect_true(
        vulkan_choose_depth_format(depth_formats, 2, depth_features,
                                   &selected_depth) &&
            selected_depth == VK_FORMAT_D32_SFLOAT,
        "depth format selection requires depth-stencil attachment support");

    in_flight = vulkan_choose_in_flight_limit(2, 2);
    failures |= expect_true(
        in_flight == 2,
        "in-flight limit is capped by swap-chain image count");
    failures |= expect_true(
        vulkan_choose_in_flight_limit(1, 2) == 1,
        "in-flight limit cannot exceed one-image swap chains");
    failures |= expect_true(
        vulkan_cube_vertex_count() == 24 &&
            vulkan_cube_index_count() == 36,
        "procedural cube uses 24 vertices and 36 indices");
    failures |= expect_true(
        vulkan_plane_vertex_count() == 4 &&
            vulkan_plane_index_count() == 6,
        "procedural plane uses 4 vertices and 6 indices");
    failures |= expect_true(
        vulkan_scene_object_count() == 5,
        "procedural scene contains the expected reusable mesh objects");
    failures |= expect_true(
        vulkan_build_model_matrix(2.0f, -1.0f, 3.0f,
                                  2.0f, 3.0f, 4.0f, model) &&
            model[0] == 2.0f &&
            model[5] == 3.0f &&
            model[10] == 4.0f &&
            model[12] == 2.0f &&
            model[13] == -1.0f &&
            model[14] == 3.0f &&
            model[15] == 1.0f,
        "model transform applies scale and translation in one matrix");
    failures |= expect_true(
        vulkan_build_perspective(16.0f / 9.0f, perspective) &&
            perspective[0] != 0.0f &&
            perspective[5] != 0.0f &&
            perspective[11] == 1.0f,
        "perspective projection produces finite 45-degree camera parameters");

    if (failures != 0)
    {
        return 1;
    }
    puts("Game Vulkan tests passed.");
    return 0;
}
