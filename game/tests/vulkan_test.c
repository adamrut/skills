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

    if (failures != 0)
    {
        return 1;
    }
    puts("Game Vulkan tests passed.");
    return 0;
}
