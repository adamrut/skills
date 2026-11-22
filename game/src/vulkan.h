#ifndef VULKAN_H
#define VULKAN_H

#include "app.h"

#include <stdbool.h>
#include <vulkan/vulkan.h>

typedef struct VulkanState VulkanState;

AppResult vulkan_init(VulkanState **state, void *native_window);
AppResult vulkan_recreate_surface(VulkanState *state);
void vulkan_shutdown(VulkanState *state);

bool vulkan_choose_surface_format(const VkSurfaceFormatKHR *formats,
                                  uint32_t format_count,
                                  VkSurfaceFormatKHR *selected);
bool vulkan_choose_present_mode(const VkPresentModeKHR *modes,
                                uint32_t mode_count,
                                VkPresentModeKHR *selected);
bool vulkan_find_queue_indices(const VkQueueFamilyProperties *families,
                               uint32_t family_count,
                               uint32_t *graphics_index,
                               uint32_t *present_index);

#endif
