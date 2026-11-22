#ifndef VULKAN_H
#define VULKAN_H

#include "app.h"

#include <stdbool.h>
#include <vulkan/vulkan.h>

typedef struct VulkanState VulkanState;

typedef struct VulkanVertex {
    float position[3];
    float normal[3];
} VulkanVertex;

typedef struct VulkanUniformBufferObject {
    float view_projection[16];
} VulkanUniformBufferObject;

typedef struct VulkanPushConstants {
    float model[16];
    float color[4];
} VulkanPushConstants;

typedef enum VulkanMeshType {
    VULKAN_MESH_CUBE = 0,
    VULKAN_MESH_PLANE = 1
} VulkanMeshType;

typedef struct VulkanSceneObject {
    VulkanMeshType mesh_type;
    float model[16];
    float color[4];
} VulkanSceneObject;

AppResult vulkan_init(VulkanState **state, void *native_window);
AppResult vulkan_recreate_surface(VulkanState *state);
AppResult vulkan_render_frame(VulkanState *state);
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
bool vulkan_choose_depth_format(const VkFormat *formats,
                                uint32_t format_count,
                                const VkFormatFeatureFlags *features,
                                VkFormat *selected);
uint32_t vulkan_choose_in_flight_limit(uint32_t swapchain_image_count,
                                       uint32_t preferred_limit);
uint32_t vulkan_cube_vertex_count(void);
uint32_t vulkan_cube_index_count(void);
uint32_t vulkan_plane_vertex_count(void);
uint32_t vulkan_plane_index_count(void);
uint32_t vulkan_scene_object_count(void);
bool vulkan_build_model_matrix(float translation_x,
                               float translation_y,
                               float translation_z,
                               float scale_x,
                               float scale_y,
                               float scale_z,
                               float *matrix);
bool vulkan_build_perspective(float aspect, float *matrix);

#endif
