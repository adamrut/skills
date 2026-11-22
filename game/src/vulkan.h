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

typedef struct VulkanFixedCamera {
    float eye[3];
    float target[3];
    float world_up[3];
    float fov_y_degrees;
    float near_plane;
    float far_plane;
} VulkanFixedCamera;

typedef struct VulkanLevelBounds {
    float min[3];
    float max[3];
} VulkanLevelBounds;

#define VULKAN_LEVEL_WIDTH 8
#define VULKAN_LEVEL_HEIGHT 8
#define VULKAN_LEVEL_TILE_SIZE 1.0f
#define VULKAN_LEVEL_GROUND_HEIGHT -0.7f
#define VULKAN_LEVEL_WALL_HEIGHT 1.2f
#define VULKAN_LEVEL_OBSTACLE_HEIGHT 0.8f
#define VULKAN_LEVEL_MAX_SCENE_OBJECTS 128

typedef struct VulkanLevelTile {
    bool walkable;
    bool collision;
    bool obstacle;
} VulkanLevelTile;

AppResult vulkan_init(VulkanState **state, void *native_window);
AppResult vulkan_recreate_surface(VulkanState *state);
AppResult vulkan_render_frame(VulkanState *state, const App *app);
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
bool vulkan_get_fixed_camera(VulkanFixedCamera *camera);
bool vulkan_get_fixed_level_bounds(VulkanLevelBounds *bounds);
bool vulkan_get_scene_center(float center[3]);
bool vulkan_clamp_camera_position(float position[3],
                                  const VulkanLevelBounds *bounds);
bool vulkan_build_fixed_view(const VulkanFixedCamera *camera, float *matrix);
bool vulkan_build_fixed_view_projection(float aspect,
                                        const VulkanFixedCamera *camera,
                                        float *view_projection);
bool vulkan_get_level_dimensions(uint32_t *width,
                                 uint32_t *height);
bool vulkan_tile_is_valid(uint32_t row, uint32_t column);
bool vulkan_tile_to_world(uint32_t row,
                          uint32_t column,
                          float world[3]);
bool vulkan_world_to_tile(const float world[3],
                          uint32_t *row,
                          uint32_t *column);
bool vulkan_get_level_tile(uint32_t row,
                           uint32_t column,
                           VulkanLevelTile *tile);
bool vulkan_is_world_position_walkable(const float world[3]);
bool vulkan_get_player_spawn(float spawn[3]);
bool vulkan_build_level_scene(VulkanSceneObject *scene,
                              uint32_t capacity,
                              uint32_t *scene_count);
bool vulkan_build_player_scene(const App *app,
                               VulkanSceneObject *scene,
                               uint32_t capacity,
                               uint32_t *scene_count);

#endif
