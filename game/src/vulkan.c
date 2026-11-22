#include "vulkan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <stddef.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif

struct VulkanState
{
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
    uint32_t graphics_queue_family;
    uint32_t present_queue_family;
    VkSurfaceFormatKHR surface_format;
    VkPresentModeKHR present_mode;
    VkSwapchainKHR swapchain;
    VkImage *swapchain_images;
    VkImageView *swapchain_image_views;
    VkFramebuffer *framebuffers;
    uint32_t swapchain_image_count;
    VkExtent2D swapchain_extent;
    VkRenderPass render_pass;
    VkImage depth_image;
    VkDeviceMemory depth_memory;
    VkImageView depth_image_view;
    VkFormat depth_format;
    VkCommandPool command_pool;
    VkCommandBuffer *command_buffers;
    VkSemaphore *image_available_semaphores;
    VkSemaphore *render_finished_semaphores;
    VkFence *in_flight_fences;
    uint32_t in_flight_count;
    uint32_t current_frame;
    VkShaderModule vertex_shader_module;
    VkShaderModule fragment_shader_module;
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet *descriptor_sets;
    VkBuffer *uniform_buffers;
    VkDeviceMemory *uniform_memories;
    void **uniform_mappings;
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;
    VkBuffer cube_vertex_buffer;
    VkDeviceMemory cube_vertex_memory;
    VkBuffer cube_index_buffer;
    VkDeviceMemory cube_index_memory;
    VkBuffer plane_vertex_buffer;
    VkDeviceMemory plane_vertex_memory;
    VkBuffer plane_index_buffer;
    VkDeviceMemory plane_index_memory;
    void *native_window;
};

static bool has_extension(const char *name,
                          const VkExtensionProperties *extensions,
                          uint32_t extension_count)
{
    uint32_t i;
    for (i = 0; i < extension_count; ++i)
    {
        if (strcmp(name, extensions[i].extensionName) == 0)
        {
            return true;
        }
    }
    return false;
}

static bool has_layer(const char *name,
                      const VkLayerProperties *layers,
                      uint32_t layer_count)
{
    uint32_t i;
    for (i = 0; i < layer_count; ++i)
    {
        if (strcmp(name, layers[i].layerName) == 0)
        {
            return true;
        }
    }
    return false;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT types,
    const VkDebugUtilsMessengerCallbackDataEXT *data,
    void *user_data)
{
    (void)severity;
    (void)types;
    (void)user_data;
    fprintf(stderr, "Vulkan validation: %s\n",
            data != NULL ? data->pMessage : "unknown validation message");
    return VK_FALSE;
}

bool vulkan_find_queue_indices(const VkQueueFamilyProperties *families,
                               uint32_t family_count,
                               uint32_t *graphics_index,
                               uint32_t *present_index)
{
    uint32_t i;
    if (families == NULL || graphics_index == NULL ||
        present_index == NULL)
    {
        return false;
    }

    for (i = 0; i < family_count; ++i)
    {
        if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
        {
            *graphics_index = i;
            *present_index = i;
            return true;
        }
    }
    return false;
}

bool vulkan_choose_surface_format(const VkSurfaceFormatKHR *formats,
                                  uint32_t format_count,
                                  VkSurfaceFormatKHR *selected)
{
    uint32_t i;
    if (formats == NULL || selected == NULL)
    {
        return false;
    }
    for (i = 0; i < format_count; ++i)
    {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            *selected = formats[i];
            return true;
        }
    }
    if (format_count > 0)
    {
        *selected = formats[0];
        return true;
    }
    return false;
}

bool vulkan_choose_present_mode(const VkPresentModeKHR *modes,
                                uint32_t mode_count,
                                VkPresentModeKHR *selected)
{
    uint32_t i;
    if (modes == NULL || selected == NULL)
    {
        return false;
    }
    for (i = 0; i < mode_count; ++i)
    {
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            *selected = modes[i];
            return true;
        }
    }
    for (i = 0; i < mode_count; ++i)
    {
        if (modes[i] == VK_PRESENT_MODE_FIFO_KHR)
        {
            *selected = modes[i];
            return true;
        }
    }
    return false;
}

bool vulkan_choose_depth_format(const VkFormat *formats,
                                uint32_t format_count,
                                const VkFormatFeatureFlags *features,
                                VkFormat *selected)
{
    uint32_t i;
    if (formats == NULL || features == NULL || selected == NULL)
    {
        return false;
    }
    for (i = 0; i < format_count; ++i)
    {
        if ((features[i] & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0)
        {
            *selected = formats[i];
            return true;
        }
    }
    return false;
}

uint32_t vulkan_choose_in_flight_limit(uint32_t swapchain_image_count,
                                       uint32_t preferred_limit)
{
    if (swapchain_image_count == 0)
    {
        return 0;
    }
    if (preferred_limit == 0)
    {
        return 1;
    }
    return preferred_limit < swapchain_image_count
               ? preferred_limit
               : swapchain_image_count;
}

uint32_t vulkan_cube_vertex_count(void)
{
    return 24;
}

uint32_t vulkan_cube_index_count(void)
{
    return 36;
}

uint32_t vulkan_plane_vertex_count(void)
{
    return 4;
}

uint32_t vulkan_plane_index_count(void)
{
    return 6;
}

uint32_t vulkan_scene_object_count(void)
{
    VulkanSceneObject scene[VULKAN_LEVEL_MAX_SCENE_OBJECTS];
    uint32_t scene_count = 0;
    vulkan_build_level_scene(scene, VULKAN_LEVEL_MAX_SCENE_OBJECTS,
                             &scene_count);
    return scene_count;
}

bool vulkan_build_model_matrix(float translation_x,
                               float translation_y,
                               float translation_z,
                               float scale_x,
                               float scale_y,
                               float scale_z,
                               float *matrix)
{
    if (matrix == NULL || scale_x == 0.0f ||
        scale_y == 0.0f || scale_z == 0.0f)
    {
        return false;
    }
    memset(matrix, 0, sizeof(float) * 16);
    matrix[0] = scale_x;
    matrix[5] = scale_y;
    matrix[10] = scale_z;
    matrix[12] = translation_x;
    matrix[13] = translation_y;
    matrix[14] = translation_z;
    matrix[15] = 1.0f;
    return true;
}

bool vulkan_build_perspective(float aspect, float *matrix)
{
    const float fov_y = 45.0f * (3.14159265358979323846f / 180.0f);
    float f;
    float tan_half_fov;

    if (matrix == NULL || !isfinite(aspect) || aspect <= 0.0f)
    {
        return false;
    }

    tan_half_fov = tanf(fov_y * 0.5f);
    f = 1.0f / tan_half_fov;
    memset(matrix, 0, sizeof(float) * 16);
    matrix[0] = f / aspect;
    matrix[5] = -f;
    matrix[10] = 100.1f / 99.9f;
    matrix[11] = 1.0f;
    matrix[14] = -(2.0f * 100.0f * 0.1f) / 99.9f;
    return true;
}

bool vulkan_get_fixed_camera(VulkanFixedCamera *camera)
{
    if (camera == NULL)
    {
        return false;
    }
    camera->eye[0] = 7.5f;
    camera->eye[1] = 7.5f;
    camera->eye[2] = 7.5f;
    camera->target[0] = 0.0f;
    camera->target[1] = 0.0f;
    camera->target[2] = 0.0f;
    camera->world_up[0] = 0.0f;
    camera->world_up[1] = 1.0f;
    camera->world_up[2] = 0.0f;
    camera->fov_y_degrees = 45.0f;
    camera->near_plane = 0.1f;
    camera->far_plane = 100.0f;
    return true;
}

bool vulkan_get_fixed_level_bounds(VulkanLevelBounds *bounds)
{
    if (bounds == NULL)
    {
        return false;
    }
    bounds->min[0] = -4.0f;
    bounds->min[1] = VULKAN_LEVEL_GROUND_HEIGHT;
    bounds->min[2] = -4.0f;
    bounds->max[0] = 4.0f;
    bounds->max[1] = VULKAN_LEVEL_GROUND_HEIGHT +
                      VULKAN_LEVEL_WALL_HEIGHT;
    bounds->max[2] = 4.0f;
    return true;
}

bool vulkan_get_scene_center(float center[3])
{
    if (center == NULL)
    {
        return false;
    }
    center[0] = 0.0f;
    center[1] = 0.0f;
    center[2] = 0.0f;
    return true;
}

bool vulkan_clamp_camera_position(float position[3],
                                  const VulkanLevelBounds *bounds)
{
    uint32_t axis;
    if (position == NULL || bounds == NULL)
    {
        return false;
    }
    for (axis = 0; axis < 3; ++axis)
    {
        if (!isfinite(position[axis]) ||
            !isfinite(bounds->min[axis]) ||
            !isfinite(bounds->max[axis]) ||
            bounds->min[axis] > bounds->max[axis])
        {
            return false;
        }
        if (position[axis] < bounds->min[axis])
        {
            position[axis] = bounds->min[axis];
        }
        else if (position[axis] > bounds->max[axis])
        {
            position[axis] = bounds->max[axis];
        }
    }
    return true;
}

bool vulkan_get_level_dimensions(uint32_t *width, uint32_t *height)
{
    if (width == NULL || height == NULL)
    {
        return false;
    }
    *width = VULKAN_LEVEL_WIDTH;
    *height = VULKAN_LEVEL_HEIGHT;
    return true;
}

bool vulkan_tile_is_valid(uint32_t row, uint32_t column)
{
    return row < VULKAN_LEVEL_HEIGHT && column < VULKAN_LEVEL_WIDTH;
}

bool vulkan_tile_to_world(uint32_t row,
                          uint32_t column,
                          float world[3])
{
    if (!vulkan_tile_is_valid(row, column) || world == NULL)
    {
        return false;
    }
    world[0] =
        ((float)column - ((float)VULKAN_LEVEL_WIDTH - 1.0f) * 0.5f) *
        VULKAN_LEVEL_TILE_SIZE;
    world[1] = VULKAN_LEVEL_GROUND_HEIGHT;
    world[2] =
        ((float)row - ((float)VULKAN_LEVEL_HEIGHT - 1.0f) * 0.5f) *
        VULKAN_LEVEL_TILE_SIZE;
    return true;
}

bool vulkan_world_to_tile(const float world[3],
                          uint32_t *row,
                          uint32_t *column)
{
    float column_position;
    float row_position;
    int32_t column_index;
    int32_t row_index;

    if (world == NULL || row == NULL || column == NULL)
    {
        return false;
    }
    if (!isfinite(world[0]) || !isfinite(world[2]))
    {
        return false;
    }
    column_position =
        (world[0] / VULKAN_LEVEL_TILE_SIZE) +
        ((float)VULKAN_LEVEL_WIDTH - 1.0f) * 0.5f;
    row_position =
        (world[2] / VULKAN_LEVEL_TILE_SIZE) +
        ((float)VULKAN_LEVEL_HEIGHT - 1.0f) * 0.5f;
    column_index = (int32_t)floorf(column_position + 0.5f);
    row_index = (int32_t)floorf(row_position + 0.5f);
    if (row_index < 0 || column_index < 0 ||
        row_index >= (int32_t)VULKAN_LEVEL_HEIGHT ||
        column_index >= (int32_t)VULKAN_LEVEL_WIDTH)
    {
        return false;
    }
    *row = (uint32_t)row_index;
    *column = (uint32_t)column_index;
    return true;
}

bool vulkan_get_level_tile(uint32_t row,
                           uint32_t column,
                           VulkanLevelTile *tile)
{
    static const char layout[VULKAN_LEVEL_HEIGHT][VULKAN_LEVEL_WIDTH + 1] = {
        "########",
        "#......#",
        "#.##...#",
        "#......#",
        "#..#...#",
        "#...#..#",
        "#......#",
        "########"
    };
    bool obstacle;

    if (!vulkan_tile_is_valid(row, column) || tile == NULL)
    {
        return false;
    }
    obstacle =
        (row == 2 && column == 5) ||
        (row == 5 && column == 5);
    memset(tile, 0, sizeof(*tile));
    tile->walkable = layout[row][column] == '.' && !obstacle;
    tile->collision = !tile->walkable;
    tile->obstacle = obstacle;
    return true;
}

bool vulkan_is_world_position_walkable(const float world[3])
{
    uint32_t row;
    uint32_t column;
    VulkanLevelTile tile;

    if (world == NULL ||
        !vulkan_world_to_tile(world, &row, &column) ||
        !vulkan_get_level_tile(row, column, &tile))
    {
        return false;
    }
    return tile.walkable;
}

bool vulkan_get_player_spawn(float spawn[3])
{
    VulkanLevelTile tile;
    if (spawn == NULL)
    {
        return false;
    }
    if (!vulkan_tile_to_world(1, 1, spawn))
    {
        return false;
    }
    if (!vulkan_get_level_tile(1, 1, &tile) || !tile.walkable)
    {
        return false;
    }
    return true;
}

bool vulkan_build_level_scene(VulkanSceneObject *scene,
                              uint32_t capacity,
                              uint32_t *scene_count)
{
    uint32_t row;
    uint32_t column;
    uint32_t count = 0;

    if (scene == NULL || scene_count == NULL)
    {
        return false;
    }
    *scene_count = 0;
    memset(scene, 0,
           sizeof(VulkanSceneObject) *
           (capacity < VULKAN_LEVEL_MAX_SCENE_OBJECTS
                ? capacity
                : VULKAN_LEVEL_MAX_SCENE_OBJECTS));

    for (row = 0; row < VULKAN_LEVEL_HEIGHT; ++row)
    {
        for (column = 0; column < VULKAN_LEVEL_WIDTH; ++column)
        {
            float world[3];
            VulkanLevelTile tile;
            if (count >= capacity ||
                !vulkan_tile_to_world(row, column, world) ||
                !vulkan_get_level_tile(row, column, &tile))
            {
                return false;
            }
            if (!vulkan_build_model_matrix(
                    world[0], 0.0f, world[2],
                    0.125f, 1.0f, 0.125f,
                    scene[count].model))
            {
                return false;
            }
            scene[count].mesh_type = VULKAN_MESH_PLANE;
            if (tile.walkable)
            {
                scene[count].color[0] = 0.28f;
                scene[count].color[1] = 0.44f;
                scene[count].color[2] = 0.32f;
            }
            else if (tile.obstacle)
            {
                scene[count].color[0] = 0.30f;
                scene[count].color[1] = 0.34f;
                scene[count].color[2] = 0.30f;
            }
            else
            {
                scene[count].color[0] = 0.18f;
                scene[count].color[1] = 0.22f;
                scene[count].color[2] = 0.20f;
            }
            scene[count].color[3] = 1.0f;
            ++count;
        }
    }

    for (row = 0; row < VULKAN_LEVEL_HEIGHT; ++row)
    {
        for (column = 0; column < VULKAN_LEVEL_WIDTH; ++column)
        {
            float world[3];
            VulkanLevelTile tile;
            if (!vulkan_get_level_tile(row, column, &tile) ||
                !vulkan_tile_to_world(row, column, world))
            {
                return false;
            }
            if (tile.obstacle)
            {
                if (count >= capacity)
                {
                    return false;
                }
                world[1] =
                    VULKAN_LEVEL_GROUND_HEIGHT +
                    VULKAN_LEVEL_OBSTACLE_HEIGHT * 0.5f;
                if (!vulkan_build_model_matrix(
                        world[0], world[1], world[2],
                        0.714285f,
                        VULKAN_LEVEL_OBSTACLE_HEIGHT / 1.4f,
                        0.714285f,
                        scene[count].model))
                {
                    return false;
                }
                scene[count].mesh_type = VULKAN_MESH_CUBE;
                scene[count].color[0] = 0.95f;
                scene[count].color[1] = 0.58f;
                scene[count].color[2] = 0.42f;
                scene[count].color[3] = 1.0f;
                ++count;
            }
            else if (!tile.walkable)
            {
                if (count >= capacity)
                {
                    return false;
                }
                world[1] =
                    VULKAN_LEVEL_GROUND_HEIGHT +
                    VULKAN_LEVEL_WALL_HEIGHT * 0.5f;
                if (!vulkan_build_model_matrix(
                        world[0], world[1], world[2],
                        0.714285f,
                        VULKAN_LEVEL_WALL_HEIGHT / 1.4f,
                        0.714285f,
                        scene[count].model))
                {
                    return false;
                }
                scene[count].mesh_type = VULKAN_MESH_CUBE;
                scene[count].color[0] = 0.58f;
                scene[count].color[1] = 0.64f;
                scene[count].color[2] = 0.72f;
                scene[count].color[3] = 1.0f;
                ++count;
            }
        }
    }

    *scene_count = count;
    return true;
}

bool vulkan_build_player_scene(const App *app,
                               VulkanSceneObject *scene,
                               uint32_t capacity,
                               uint32_t *scene_count)
{
    float marker_x;
    float marker_z;
    uint32_t count = 0;

    if (app == NULL || scene == NULL || scene_count == NULL ||
        capacity < 5)
    {
        if (scene_count != NULL)
        {
            *scene_count = 0;
        }
        return false;
    }

    switch (app->player.facing)
    {
    case APP_FACING_NORTH:
        marker_x = 0.0f;
        marker_z = -0.38f;
        break;
    case APP_FACING_NORTHEAST:
        marker_x = 0.27f;
        marker_z = -0.27f;
        break;
    case APP_FACING_EAST:
        marker_x = 0.38f;
        marker_z = 0.0f;
        break;
    case APP_FACING_SOUTHEAST:
        marker_x = 0.27f;
        marker_z = 0.27f;
        break;
    case APP_FACING_SOUTH:
        marker_x = 0.0f;
        marker_z = 0.38f;
        break;
    case APP_FACING_SOUTHWEST:
        marker_x = -0.27f;
        marker_z = 0.27f;
        break;
    case APP_FACING_WEST:
        marker_x = -0.38f;
        marker_z = 0.0f;
        break;
    case APP_FACING_NORTHWEST:
        marker_x = -0.27f;
        marker_z = -0.27f;
        break;
    default:
        marker_x = 0.0f;
        marker_z = 0.38f;
        break;
    }

    scene[count].mesh_type = VULKAN_MESH_CUBE;
    if (!vulkan_build_model_matrix(
            app->player.position[0],
            VULKAN_LEVEL_GROUND_HEIGHT + 0.35f,
            app->player.position[2],
            0.36f, 0.70f, 0.36f,
            scene[count].model))
    {
        return false;
    }
    scene[count].color[0] = 0.16f;
    scene[count].color[1] = 0.48f;
    scene[count].color[2] = 0.86f;
    scene[count].color[3] = 1.0f;
    ++count;

    scene[count].mesh_type = VULKAN_MESH_CUBE;
    if (!vulkan_build_model_matrix(
            app->player.position[0],
            VULKAN_LEVEL_GROUND_HEIGHT + 0.93f,
            app->player.position[2],
            0.28f, 0.28f, 0.28f,
            scene[count].model))
    {
        return false;
    }
    scene[count].color[0] = 0.78f;
    scene[count].color[1] = 0.56f;
    scene[count].color[2] = 0.38f;
    scene[count].color[3] = 1.0f;
    ++count;

    scene[count].mesh_type = VULKAN_MESH_CUBE;
    if (!vulkan_build_model_matrix(
            app->player.position[0] - 0.13f,
            VULKAN_LEVEL_GROUND_HEIGHT - 0.36f,
            app->player.position[2],
            0.10f, 0.34f, 0.10f,
            scene[count].model))
    {
        return false;
    }
    scene[count].color[0] = 0.10f;
    scene[count].color[1] = 0.24f;
    scene[count].color[2] = 0.55f;
    scene[count].color[3] = 1.0f;
    ++count;

    scene[count].mesh_type = VULKAN_MESH_CUBE;
    if (!vulkan_build_model_matrix(
            app->player.position[0] + 0.13f,
            VULKAN_LEVEL_GROUND_HEIGHT - 0.36f,
            app->player.position[2],
            0.10f, 0.34f, 0.10f,
            scene[count].model))
    {
        return false;
    }
    scene[count].color[0] = 0.10f;
    scene[count].color[1] = 0.24f;
    scene[count].color[2] = 0.55f;
    scene[count].color[3] = 1.0f;
    ++count;

    scene[count].mesh_type = VULKAN_MESH_CUBE;
    if (!vulkan_build_model_matrix(
            app->player.position[0] + marker_x,
            VULKAN_LEVEL_GROUND_HEIGHT + 0.30f,
            app->player.position[2] + marker_z,
            0.10f, 0.10f, 0.10f,
            scene[count].model))
    {
        return false;
    }
    scene[count].color[0] = 1.0f;
    scene[count].color[1] = 0.82f;
    scene[count].color[2] = 0.16f;
    scene[count].color[3] = 1.0f;
    ++count;

    *scene_count = count;
    return true;
}

static void multiply_mat4(const float *a, const float *b, float *out);

bool vulkan_build_fixed_view(const VulkanFixedCamera *camera, float *matrix)
{
    float forward[3];
    float right[3];
    float up[3];
    float forward_length;
    float right_length;
    uint32_t axis;

    if (camera == NULL || matrix == NULL)
    {
        return false;
    }
    for (axis = 0; axis < 3; ++axis)
    {
        forward[axis] = camera->target[axis] - camera->eye[axis];
        if (!isfinite(forward[axis]) ||
            !isfinite(camera->world_up[axis]) ||
            !isfinite(camera->eye[axis]) ||
            !isfinite(camera->target[axis]))
        {
            return false;
        }
    }
    forward_length =
        sqrtf(forward[0] * forward[0] +
              forward[1] * forward[1] +
              forward[2] * forward[2]);
    if (!isfinite(forward_length) || forward_length <= 0.0f)
    {
        return false;
    }
    forward[0] /= forward_length;
    forward[1] /= forward_length;
    forward[2] /= forward_length;

    right[0] = forward[1] * camera->world_up[2] -
               forward[2] * camera->world_up[1];
    right[1] = forward[2] * camera->world_up[0] -
               forward[0] * camera->world_up[2];
    right[2] = forward[0] * camera->world_up[1] -
               forward[1] * camera->world_up[0];
    right_length =
        sqrtf(right[0] * right[0] +
              right[1] * right[1] +
              right[2] * right[2]);
    if (!isfinite(right_length) || right_length <= 0.0f)
    {
        return false;
    }
    right[0] /= right_length;
    right[1] /= right_length;
    right[2] /= right_length;

    up[0] = right[1] * forward[2] - right[2] * forward[1];
    up[1] = right[2] * forward[0] - right[0] * forward[2];
    up[2] = right[0] * forward[1] - right[1] * forward[0];

    memset(matrix, 0, sizeof(float) * 16);
    matrix[0] = right[0];
    matrix[4] = right[1];
    matrix[8] = right[2];
    matrix[1] = up[0];
    matrix[5] = up[1];
    matrix[9] = up[2];
    matrix[2] = -forward[0];
    matrix[6] = -forward[1];
    matrix[10] = -forward[2];
    matrix[12] = -(right[0] * camera->eye[0] +
                   right[1] * camera->eye[1] +
                   right[2] * camera->eye[2]);
    matrix[13] = -(up[0] * camera->eye[0] +
                   up[1] * camera->eye[1] +
                   up[2] * camera->eye[2]);
    matrix[14] = -(-forward[0] * camera->eye[0] +
                    -forward[1] * camera->eye[1] +
                    -forward[2] * camera->eye[2]);
    matrix[15] = 1.0f;
    return true;
}

bool vulkan_build_fixed_view_projection(float aspect,
                                        const VulkanFixedCamera *camera,
                                        float *view_projection)
{
    float perspective[16];
    float view[16];
    float fov_y;
    float tan_half_fov;
    float f;

    if (aspect <= 0.0f || !isfinite(aspect) ||
        camera == NULL || view_projection == NULL ||
        camera->near_plane <= 0.0f ||
        camera->far_plane <= camera->near_plane ||
        camera->fov_y_degrees <= 0.0f ||
        camera->fov_y_degrees >= 180.0f)
    {
        return false;
    }
    if (!vulkan_build_fixed_view(camera, view))
    {
        return false;
    }

    fov_y = camera->fov_y_degrees *
            (3.14159265358979323846f / 180.0f);
    tan_half_fov = tanf(fov_y * 0.5f);
    f = 1.0f / tan_half_fov;
    memset(perspective, 0, sizeof(perspective));
    perspective[0] = f / aspect;
    perspective[5] = -f;
    perspective[10] =
        camera->far_plane / (camera->far_plane - camera->near_plane);
    perspective[11] = 1.0f;
    perspective[14] =
        -(camera->far_plane * camera->near_plane) /
        (camera->far_plane - camera->near_plane);

    multiply_mat4(perspective, view, view_projection);
    return true;
}

#ifndef _WIN32
AppResult vulkan_init(VulkanState **state, void *native_window)
{
    (void)state;
    (void)native_window;
    fprintf(stderr,
            "Vulkan initialization failed: Step 3 requires a native Win32 window.\n");
    return APP_RESULT_INITIALIZATION_FAILED;
}

AppResult vulkan_recreate_surface(VulkanState *state)
{
    (void)state;
    return APP_RESULT_RUNTIME_FAILED;
}

AppResult vulkan_render_frame(VulkanState *state, const App *app)
{
    (void)app;
    (void)state;
    return APP_RESULT_RUNTIME_FAILED;
}

void vulkan_shutdown(VulkanState *state)
{
    (void)state;
}
#else

static VkResult create_debug_messenger(VkInstance instance,
                                       VkDebugUtilsMessengerEXT *messenger)
{
    VkDebugUtilsMessengerCreateInfoEXT info;
    PFN_vkCreateDebugUtilsMessengerEXT create_messenger;

    memset(&info, 0, sizeof(info));
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    info.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    info.pfnUserCallback = vulkan_debug_callback;

    create_messenger = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (create_messenger == NULL)
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    return create_messenger(instance, &info, NULL, messenger);
}

static void destroy_debug_messenger(VkInstance instance,
                                    VkDebugUtilsMessengerEXT messenger)
{
    PFN_vkDestroyDebugUtilsMessengerEXT destroy_messenger =
        (PFN_vkDestroyDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance,
                                  "vkDestroyDebugUtilsMessengerEXT");
    if (destroy_messenger != NULL)
    {
        destroy_messenger(instance, messenger, NULL);
    }
}

static AppResult create_surface(VulkanState *state)
{
    VkWin32SurfaceCreateInfoKHR create_info;
    VkResult result;

    memset(&create_info, 0, sizeof(create_info));
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hinstance = GetModuleHandleW(NULL);
    create_info.hwnd = (HWND)state->native_window;

    result = vkCreateWin32SurfaceKHR(state->instance,
                                     &create_info,
                                     NULL,
                                     &state->surface);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "Vulkan surface creation failed with VkResult %d. "
                "Verify the Win32 window and Vulkan display support.\n",
                (int)result);
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    return APP_RESULT_OK;
}

static AppResult select_device_and_queues(VulkanState *state)
{
    uint32_t device_count = 0;
    VkPhysicalDevice *devices = NULL;
    VkResult result;
    uint32_t i;

    result = vkEnumeratePhysicalDevices(state->instance,
                                        &device_count,
                                        NULL);
    if (result != VK_SUCCESS || device_count == 0)
    {
        fprintf(stderr,
                "Vulkan physical-device enumeration found no usable "
                "device. Install a Vulkan-capable GPU and graphics "
                "driver, then retry.\n");
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    devices = (VkPhysicalDevice *)malloc(
        (size_t)device_count * sizeof(VkPhysicalDevice));
    if (devices == NULL)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    result = vkEnumeratePhysicalDevices(state->instance,
                                        &device_count,
                                        devices);
    if (result != VK_SUCCESS)
    {
        free(devices);
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    for (i = 0; i < device_count; ++i)
    {
        uint32_t family_count = 0;
        VkQueueFamilyProperties *families = NULL;
        uint32_t extension_count = 0;
        VkExtensionProperties *extensions = NULL;
        bool swapchain_supported = false;
        uint32_t graphics = VK_QUEUE_FAMILY_IGNORED;
        uint32_t present = VK_QUEUE_FAMILY_IGNORED;
        uint32_t f;

        if (vkEnumerateDeviceExtensionProperties(
                devices[i], NULL, &extension_count, NULL) == VK_SUCCESS &&
            extension_count > 0)
        {
            extensions = (VkExtensionProperties *)malloc(
                (size_t)extension_count * sizeof(VkExtensionProperties));
            if (extensions != NULL &&
                vkEnumerateDeviceExtensionProperties(
                    devices[i], NULL, &extension_count, extensions) ==
                    VK_SUCCESS)
            {
                swapchain_supported =
                    has_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                  extensions,
                                  extension_count);
            }
        }
        free(extensions);
        if (!swapchain_supported)
        {
            continue;
        }

        vkGetPhysicalDeviceQueueFamilyProperties(
            devices[i], &family_count, NULL);
        if (family_count == 0)
        {
            continue;
        }
        families = (VkQueueFamilyProperties *)malloc(
            (size_t)family_count * sizeof(VkQueueFamilyProperties));
        if (families == NULL)
        {
            free(devices);
            return APP_RESULT_INITIALIZATION_FAILED;
        }
        vkGetPhysicalDeviceQueueFamilyProperties(
            devices[i], &family_count, families);

        for (f = 0; f < family_count; ++f)
        {
            VkBool32 supports_present = VK_FALSE;
            VkResult present_result =
                vkGetPhysicalDeviceSurfaceSupportKHR(
                    devices[i], f, state->surface,
                    &supports_present);
            if (present_result == VK_SUCCESS &&
                supports_present == VK_TRUE &&
                (families[f].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
            {
                graphics = f;
                present = f;
                break;
            }
        }

        if (graphics != VK_QUEUE_FAMILY_IGNORED)
        {
            state->physical_device = devices[i];
            state->graphics_queue_family = graphics;
            state->present_queue_family = present;
            free(families);
            break;
        }
        free(families);
    }
    free(devices);

    if (state->physical_device == VK_NULL_HANDLE)
    {
        fprintf(stderr,
                "Vulkan device selection failed: no physical device "
                "provides both graphics and presentation queue support "
                "for this surface. Update the graphics driver or use "
                "a supported Vulkan GPU.\n");
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    return APP_RESULT_OK;
}

static AppResult select_surface_configuration(VulkanState *state)
{
    uint32_t format_count = 0;
    uint32_t mode_count = 0;
    VkSurfaceFormatKHR *formats = NULL;
    VkPresentModeKHR *modes = NULL;
    VkSurfaceCapabilitiesKHR capabilities;
    VkResult result;
    AppResult selected_result = APP_RESULT_INITIALIZATION_FAILED;

    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        state->physical_device, state->surface, &capabilities);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "Vulkan surface capabilities could not be queried "
                "(VkResult %d).\n", (int)result);
        return selected_result;
    }
    if (capabilities.supportedUsageFlags == 0)
    {
        fprintf(stderr,
                "Vulkan surface configuration is unavailable: the "
                "physical device reports no usable formats or usage.\n");
        return selected_result;
    }

    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        state->physical_device, state->surface,
        &format_count, NULL);
    if (result != VK_SUCCESS || format_count == 0)
    {
        fprintf(stderr,
                "Vulkan surface format enumeration found no supported "
                "formats.\n");
        return selected_result;
    }
    formats = (VkSurfaceFormatKHR *)malloc(
        (size_t)format_count * sizeof(VkSurfaceFormatKHR));
    if (formats == NULL)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(
            state->physical_device, state->surface,
            &format_count, formats) != VK_SUCCESS ||
        !vulkan_choose_surface_format(formats, format_count,
                                      &state->surface_format))
    {
        fprintf(stderr,
                "Vulkan surface format selection failed: no format was "
                "reported by the physical device.\n");
        free(formats);
        return selected_result;
    }
    free(formats);

    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        state->physical_device, state->surface,
        &mode_count, NULL);
    if (result != VK_SUCCESS || mode_count == 0)
    {
        fprintf(stderr,
                "Vulkan present-mode enumeration found no supported "
                "present modes.\n");
        return selected_result;
    }
    modes = (VkPresentModeKHR *)malloc(
        (size_t)mode_count * sizeof(VkPresentModeKHR));
    if (modes == NULL)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(
            state->physical_device, state->surface,
            &mode_count, modes) != VK_SUCCESS ||
        !vulkan_choose_present_mode(modes, mode_count,
                                    &state->present_mode))
    {
        fprintf(stderr,
                "Vulkan present-mode selection failed: neither mailbox "
                "nor FIFO presentation is available.\n");
        free(modes);
        return selected_result;
    }
    free(modes);
    return APP_RESULT_OK;
}

static AppResult create_logical_device(VulkanState *state)
{
    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_infos[2];
    VkPhysicalDeviceFeatures features;
    VkDeviceCreateInfo device_info;
    const char *device_extensions[1];
    VkResult result;
    uint32_t queue_info_count = 1;

    memset(queue_infos, 0, sizeof(queue_infos));
    memset(&features, 0, sizeof(features));
    memset(&device_info, 0, sizeof(device_info));

    queue_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_infos[0].queueFamilyIndex = state->graphics_queue_family;
    queue_infos[0].queueCount = 1;
    queue_infos[0].pQueuePriorities = &queue_priority;

    if (state->present_queue_family != state->graphics_queue_family)
    {
        queue_infos[1].sType =
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_infos[1].queueFamilyIndex = state->present_queue_family;
        queue_infos[1].queueCount = 1;
        queue_infos[1].pQueuePriorities = &queue_priority;
        queue_info_count = 2;
    }

    device_extensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = queue_info_count;
    device_info.pQueueCreateInfos = queue_infos;
    device_info.enabledExtensionCount = 1;
    device_info.ppEnabledExtensionNames = device_extensions;
    device_info.pEnabledFeatures = &features;

    result = vkCreateDevice(state->physical_device,
                            &device_info,
                            NULL,
                            &state->device);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "Vulkan logical-device creation failed with VkResult %d.\n",
                (int)result);
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    vkGetDeviceQueue(state->device,
                     state->graphics_queue_family,
                     0,
                     &state->graphics_queue);
    vkGetDeviceQueue(state->device,
                     state->present_queue_family,
                     0,
                     &state->present_queue);
    return APP_RESULT_OK;
}

static uint32_t find_memory_type(VulkanState *state,
                                 uint32_t type_bits,
                                 VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    uint32_t i;

    vkGetPhysicalDeviceMemoryProperties(state->physical_device,
                                        &memory_properties);
    for (i = 0; i < memory_properties.memoryTypeCount; ++i)
    {
        if ((type_bits & (1u << i)) != 0 &&
            (memory_properties.memoryTypes[i].propertyFlags &
             properties) == properties)
        {
            return i;
        }
    }
    return UINT32_MAX;
}

static AppResult find_depth_format(VulkanState *state)
{
    const VkFormat candidates[4] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM
    };
    VkFormat selected = VK_FORMAT_UNDEFINED;
    uint32_t i;

    for (i = 0; i < 4; ++i)
    {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(state->physical_device,
                                            candidates[i],
                                            &properties);
        if ((properties.optimalTilingFeatures &
             VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0)
        {
            selected = candidates[i];
            break;
        }
    }
    if (selected == VK_FORMAT_UNDEFINED)
    {
        fprintf(stderr,
                "Vulkan depth-format selection failed: no supported "
                "depth-stencil attachment format is available.\n");
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    state->depth_format = selected;
    return APP_RESULT_OK;
}

static AppResult create_buffer(VulkanState *state,
                               VkDeviceSize size,
                               VkBufferUsageFlags usage,
                               VkMemoryPropertyFlags properties,
                               VkBuffer *buffer,
                               VkDeviceMemory *memory)
{
    VkBufferCreateInfo buffer_info;
    VkMemoryRequirements requirements;
    VkMemoryAllocateInfo allocate_info;
    uint32_t memory_type;

    memset(&buffer_info, 0, sizeof(buffer_info));
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(state->device, &buffer_info, NULL, buffer) != VK_SUCCESS)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    vkGetBufferMemoryRequirements(state->device, *buffer, &requirements);
    memory_type = find_memory_type(state, requirements.memoryTypeBits,
                                   properties);
    if (memory_type == UINT32_MAX)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    memset(&allocate_info, 0, sizeof(allocate_info));
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = requirements.size;
    allocate_info.memoryTypeIndex = memory_type;
    if (vkAllocateMemory(state->device, &allocate_info, NULL, memory) !=
        VK_SUCCESS ||
        vkBindBufferMemory(state->device, *buffer, *memory, 0) != VK_SUCCESS)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    return APP_RESULT_OK;
}

static AppResult upload_host_visible_buffer(VulkanState *state,
                                            VkDeviceSize size,
                                            VkBufferUsageFlags usage,
                                            const void *data,
                                            VkBuffer *buffer,
                                            VkDeviceMemory *memory)
{
    void *mapped;
    if (create_buffer(state,
                      size,
                      usage,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      buffer,
                      memory) != APP_RESULT_OK ||
        vkMapMemory(state->device, *memory, 0, size, 0, &mapped) !=
            VK_SUCCESS)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    memcpy(mapped, data, (size_t)size);
    vkUnmapMemory(state->device, *memory);
    return APP_RESULT_OK;
}

static void add_cube_vertex(VulkanVertex *vertices,
                            uint32_t *index,
                            float px, float py, float pz,
                            float nx, float ny, float nz)
{
    vertices[*index].position[0] = px;
    vertices[*index].position[1] = py;
    vertices[*index].position[2] = pz;
    vertices[*index].normal[0] = nx;
    vertices[*index].normal[1] = ny;
    vertices[*index].normal[2] = nz;
    *index += 1;
}

static void fill_cube_vertices(VulkanVertex *vertices)
{
    uint32_t i = 0;
    add_cube_vertex(vertices, &i, -0.7f, -0.7f,  0.7f, 0,0,1);
    add_cube_vertex(vertices, &i,  0.7f, -0.7f,  0.7f, 0,0,1);
    add_cube_vertex(vertices, &i,  0.7f,  0.7f,  0.7f, 0,0,1);
    add_cube_vertex(vertices, &i, -0.7f,  0.7f,  0.7f, 0,0,1);
    add_cube_vertex(vertices, &i,  0.7f, -0.7f, -0.7f, 0,0,-1);
    add_cube_vertex(vertices, &i, -0.7f, -0.7f, -0.7f, 0,0,-1);
    add_cube_vertex(vertices, &i, -0.7f,  0.7f, -0.7f, 0,0,-1);
    add_cube_vertex(vertices, &i,  0.7f,  0.7f, -0.7f, 0,0,-1);
    add_cube_vertex(vertices, &i, -0.7f, -0.7f, -0.7f, -1,0,0);
    add_cube_vertex(vertices, &i, -0.7f, -0.7f,  0.7f, -1,0,0);
    add_cube_vertex(vertices, &i, -0.7f,  0.7f,  0.7f, -1,0,0);
    add_cube_vertex(vertices, &i, -0.7f,  0.7f, -0.7f, -1,0,0);
    add_cube_vertex(vertices, &i,  0.7f, -0.7f,  0.7f, 1,0,0);
    add_cube_vertex(vertices, &i,  0.7f, -0.7f, -0.7f, 1,0,0);
    add_cube_vertex(vertices, &i,  0.7f,  0.7f, -0.7f, 1,0,0);
    add_cube_vertex(vertices, &i,  0.7f,  0.7f,  0.7f, 1,0,0);
    add_cube_vertex(vertices, &i, -0.7f,  0.7f,  0.7f, 0,1,0);
    add_cube_vertex(vertices, &i,  0.7f,  0.7f,  0.7f, 0,1,0);
    add_cube_vertex(vertices, &i,  0.7f,  0.7f, -0.7f, 0,1,0);
    add_cube_vertex(vertices, &i, -0.7f,  0.7f, -0.7f, 0,1,0);
    add_cube_vertex(vertices, &i, -0.7f, -0.7f, -0.7f, 0,-1,0);
    add_cube_vertex(vertices, &i,  0.7f, -0.7f, -0.7f, 0,-1,0);
    add_cube_vertex(vertices, &i,  0.7f, -0.7f,  0.7f, 0,-1,0);
    add_cube_vertex(vertices, &i, -0.7f, -0.7f,  0.7f, 0,-1,0);
}

static void fill_cube_indices(uint16_t *indices)
{
    const uint16_t values[36] = {
        0,1,2, 0,2,3,
        4,5,6, 4,6,7,
        8,9,10, 8,10,11,
        12,13,14, 12,14,15,
        16,17,18, 16,18,19,
        20,21,22, 20,22,23
    };
    memcpy(indices, values, sizeof(values));
}

static void fill_plane_vertices(VulkanVertex *vertices)
{
    memset(vertices, 0, sizeof(VulkanVertex) * 4);
    vertices[0].position[0] = -4.0f;
    vertices[0].position[1] = -0.7f;
    vertices[0].position[2] = -4.0f;
    vertices[1].position[0] = 4.0f;
    vertices[1].position[1] = -0.7f;
    vertices[1].position[2] = -4.0f;
    vertices[2].position[0] = 4.0f;
    vertices[2].position[1] = -0.7f;
    vertices[2].position[2] = 4.0f;
    vertices[3].position[0] = -4.0f;
    vertices[3].position[1] = -0.7f;
    vertices[3].position[2] = 4.0f;
    vertices[0].normal[1] = 1.0f;
    vertices[1].normal[1] = 1.0f;
    vertices[2].normal[1] = 1.0f;
    vertices[3].normal[1] = 1.0f;
}

static void fill_plane_indices(uint16_t *indices)
{
    const uint16_t values[6] = {0,1,2, 0,2,3};
    memcpy(indices, values, sizeof(values));
}

static void multiply_mat4(const float *a, const float *b, float *out)
{
    uint32_t column;
    uint32_t row;
    memset(out, 0, sizeof(float) * 16);
    for (column = 0; column < 4; ++column)
    {
        for (row = 0; row < 4; ++row)
        {
            out[column * 4 + row] =
                a[row + 0] * b[column * 4 + 0] +
                a[row + 4] * b[column * 4 + 1] +
                a[row + 8] * b[column * 4 + 2] +
                a[row + 12] * b[column * 4 + 3];
        }
    }
}

static AppResult create_shader_module(VulkanState *state,
                                      const char *relative_path,
                                      VkShaderModule *module)
{
    char executable_path[MAX_PATH];
    char shader_path[MAX_PATH];
    DWORD length;
    FILE *file;
    long size;
    char *code;
    VkShaderModuleCreateInfo info;
    char *slash;

    length = GetModuleFileNameA(NULL, executable_path, MAX_PATH);
    if (length == 0 || length >= MAX_PATH)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    slash = strrchr(executable_path, '\\');
    if (slash == NULL)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    *slash = '\0';
    snprintf(shader_path, MAX_PATH, "%s\\shaders\\%s",
             executable_path, relative_path);
    if (fopen_s(&file, shader_path, "rb") != 0 || file == NULL)
    {
        fprintf(stderr, "Unable to open compiled shader: %s\n", shader_path);
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    size = ftell(file);
    if (size <= 0 || (size % 4) != 0)
    {
        fclose(file);
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    rewind(file);
    code = (char *)malloc((size_t)size);
    if (code == NULL || fread(code, 1, (size_t)size, file) != (size_t)size)
    {
        free(code);
        fclose(file);
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    fclose(file);
    memset(&info, 0, sizeof(info));
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = (size_t)size;
    info.pCode = (const uint32_t *)code;
    if (vkCreateShaderModule(state->device, &info, NULL, module) !=
        VK_SUCCESS)
    {
        free(code);
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    free(code);
    return APP_RESULT_OK;
}

static void cleanup_swapchain_resources(VulkanState *state)
{
    uint32_t i;
    if (state == NULL || state->device == VK_NULL_HANDLE)
    {
        return;
    }
    vkDeviceWaitIdle(state->device);

    if (state->graphics_pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(state->device, state->graphics_pipeline, NULL);
        state->graphics_pipeline = VK_NULL_HANDLE;
    }
    if (state->pipeline_layout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(state->device, state->pipeline_layout, NULL);
        state->pipeline_layout = VK_NULL_HANDLE;
    }
    if (state->descriptor_pool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(state->device, state->descriptor_pool, NULL);
        state->descriptor_pool = VK_NULL_HANDLE;
    }
    if (state->descriptor_set_layout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(state->device,
                                     state->descriptor_set_layout, NULL);
        state->descriptor_set_layout = VK_NULL_HANDLE;
    }
    if (state->uniform_buffers != NULL)
    {
        for (i = 0; i < state->in_flight_count; ++i)
        {
            if (state->uniform_mappings != NULL &&
                state->uniform_mappings[i] != NULL)
            {
                vkUnmapMemory(state->device, state->uniform_memories[i]);
            }
            if (state->uniform_memories[i] != VK_NULL_HANDLE)
            {
                vkFreeMemory(state->device, state->uniform_memories[i], NULL);
            }
            if (state->uniform_buffers[i] != VK_NULL_HANDLE)
            {
                vkDestroyBuffer(state->device, state->uniform_buffers[i], NULL);
            }
        }
        free(state->uniform_mappings);
        free(state->uniform_memories);
        free(state->uniform_buffers);
        state->uniform_mappings = NULL;
        state->uniform_memories = NULL;
        state->uniform_buffers = NULL;
    }
    if (state->cube_index_buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(state->device, state->cube_index_buffer, NULL);
        state->cube_index_buffer = VK_NULL_HANDLE;
    }
    if (state->cube_index_memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(state->device, state->cube_index_memory, NULL);
        state->cube_index_memory = VK_NULL_HANDLE;
    }
    if (state->cube_vertex_buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(state->device, state->cube_vertex_buffer, NULL);
        state->cube_vertex_buffer = VK_NULL_HANDLE;
    }
    if (state->cube_vertex_memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(state->device, state->cube_vertex_memory, NULL);
        state->cube_vertex_memory = VK_NULL_HANDLE;
    }
    if (state->plane_index_buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(state->device, state->plane_index_buffer, NULL);
        state->plane_index_buffer = VK_NULL_HANDLE;
    }
    if (state->plane_index_memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(state->device, state->plane_index_memory, NULL);
        state->plane_index_memory = VK_NULL_HANDLE;
    }
    if (state->plane_vertex_buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(state->device, state->plane_vertex_buffer, NULL);
        state->plane_vertex_buffer = VK_NULL_HANDLE;
    }
    if (state->plane_vertex_memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(state->device, state->plane_vertex_memory, NULL);
        state->plane_vertex_memory = VK_NULL_HANDLE;
    }
    free(state->descriptor_sets);
    state->descriptor_sets = NULL;
    if (state->vertex_shader_module != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(state->device, state->vertex_shader_module, NULL);
        state->vertex_shader_module = VK_NULL_HANDLE;
    }
    if (state->fragment_shader_module != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(state->device, state->fragment_shader_module, NULL);
        state->fragment_shader_module = VK_NULL_HANDLE;
    }

    if (state->framebuffers != NULL)
    {
        for (i = 0; i < state->swapchain_image_count; ++i)
        {
            if (state->framebuffers[i] != VK_NULL_HANDLE)
            {
                vkDestroyFramebuffer(state->device,
                                     state->framebuffers[i],
                                     NULL);
            }
        }
        free(state->framebuffers);
        state->framebuffers = NULL;
    }
    if (state->swapchain_image_views != NULL)
    {
        for (i = 0; i < state->swapchain_image_count; ++i)
        {
            if (state->swapchain_image_views[i] != VK_NULL_HANDLE)
            {
                vkDestroyImageView(state->device,
                                   state->swapchain_image_views[i],
                                   NULL);
            }
        }
        free(state->swapchain_image_views);
        state->swapchain_image_views = NULL;
    }
    free(state->swapchain_images);
    state->swapchain_images = NULL;
    state->swapchain_image_count = 0;

    if (state->depth_image_view != VK_NULL_HANDLE)
    {
        vkDestroyImageView(state->device, state->depth_image_view, NULL);
        state->depth_image_view = VK_NULL_HANDLE;
    }
    if (state->depth_image != VK_NULL_HANDLE)
    {
        vkDestroyImage(state->device, state->depth_image, NULL);
        state->depth_image = VK_NULL_HANDLE;
    }
    if (state->depth_memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(state->device, state->depth_memory, NULL);
        state->depth_memory = VK_NULL_HANDLE;
    }
    if (state->render_pass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(state->device, state->render_pass, NULL);
        state->render_pass = VK_NULL_HANDLE;
    }
    if (state->command_pool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(state->device, state->command_pool, NULL);
        state->command_pool = VK_NULL_HANDLE;
    }
    if (state->image_available_semaphores != NULL)
    {
        for (i = 0; i < state->in_flight_count; ++i)
        {
            if (state->image_available_semaphores[i] != VK_NULL_HANDLE)
            {
                vkDestroySemaphore(state->device,
                                   state->image_available_semaphores[i],
                                   NULL);
            }
        }
        free(state->image_available_semaphores);
        state->image_available_semaphores = NULL;
    }
    if (state->render_finished_semaphores != NULL)
    {
        for (i = 0; i < state->in_flight_count; ++i)
        {
            if (state->render_finished_semaphores[i] != VK_NULL_HANDLE)
            {
                vkDestroySemaphore(state->device,
                                   state->render_finished_semaphores[i],
                                   NULL);
            }
        }
        free(state->render_finished_semaphores);
        state->render_finished_semaphores = NULL;
    }
    if (state->in_flight_fences != NULL)
    {
        for (i = 0; i < state->in_flight_count; ++i)
        {
            if (state->in_flight_fences[i] != VK_NULL_HANDLE)
            {
                vkDestroyFence(state->device,
                               state->in_flight_fences[i],
                               NULL);
            }
        }
        free(state->in_flight_fences);
        state->in_flight_fences = NULL;
    }
    free(state->command_buffers);
    state->command_buffers = NULL;
    state->in_flight_count = 0;
    state->current_frame = 0;
}

static AppResult create_render_pass(VulkanState *state)
{
    VkAttachmentDescription attachments[2];
    VkAttachmentReference color_reference;
    VkAttachmentReference depth_reference;
    VkSubpassDescription subpass;
    VkRenderPassCreateInfo render_pass_info;
    VkResult result;

    memset(attachments, 0, sizeof(attachments));
    attachments[0].format = state->surface_format.format;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachments[1].format = state->depth_format;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    memset(&color_reference, 0, sizeof(color_reference));
    color_reference.attachment = 0;
    color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    memset(&depth_reference, 0, sizeof(depth_reference));
    depth_reference.attachment = 1;
    depth_reference.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    memset(&subpass, 0, sizeof(subpass));
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_reference;
    subpass.pDepthStencilAttachment = &depth_reference;

    memset(&render_pass_info, 0, sizeof(render_pass_info));
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 2;
    render_pass_info.pAttachments = attachments;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;

    result = vkCreateRenderPass(state->device,
                                &render_pass_info,
                                NULL,
                                &state->render_pass);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "Vulkan render-pass creation failed with VkResult %d.\n",
                (int)result);
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    return APP_RESULT_OK;
}

static AppResult create_depth_resources(VulkanState *state)
{
    VkImageCreateInfo image_info;
    VkMemoryRequirements memory_requirements;
    VkMemoryAllocateInfo allocate_info;
    VkImageViewCreateInfo view_info;
    VkResult result;
    uint32_t memory_type;

    memset(&image_info, 0, sizeof(image_info));
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = state->depth_format;
    image_info.extent.width = state->swapchain_extent.width;
    image_info.extent.height = state->swapchain_extent.height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    result = vkCreateImage(state->device, &image_info, NULL,
                           &state->depth_image);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "Vulkan depth-image creation failed with VkResult %d.\n",
                (int)result);
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    vkGetImageMemoryRequirements(state->device,
                                 state->depth_image,
                                 &memory_requirements);
    memory_type = find_memory_type(
        state, memory_requirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (memory_type == UINT32_MAX)
    {
        fprintf(stderr,
                "Vulkan depth-image allocation failed: no device-local "
                "memory type is available.\n");
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    memset(&allocate_info, 0, sizeof(allocate_info));
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = memory_type;
    result = vkAllocateMemory(state->device, &allocate_info, NULL,
                              &state->depth_memory);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "Vulkan depth-memory allocation failed with VkResult %d.\n",
                (int)result);
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    if (vkBindImageMemory(state->device, state->depth_image,
                          state->depth_memory, 0) != VK_SUCCESS)
    {
        fprintf(stderr,
                "Vulkan depth-image memory binding failed.\n");
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    memset(&view_info, 0, sizeof(view_info));
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = state->depth_image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = state->depth_format;
    view_info.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_DEPTH_BIT |
        (state->depth_format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
         state->depth_format == VK_FORMAT_D24_UNORM_S8_UINT
             ? VK_IMAGE_ASPECT_STENCIL_BIT
             : 0);
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.layerCount = 1;
    result = vkCreateImageView(state->device, &view_info, NULL,
                               &state->depth_image_view);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "Vulkan depth-image view creation failed with VkResult %d.\n",
                (int)result);
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    return APP_RESULT_OK;
}

static AppResult create_pipeline_resources(VulkanState *state)
{
    VkDescriptorSetLayoutBinding descriptor_binding;
    VkDescriptorSetLayoutCreateInfo descriptor_layout_info;
    VkDescriptorPoolSize pool_size;
    VkDescriptorPoolCreateInfo pool_info;
    VkDescriptorSetAllocateInfo descriptor_allocate_info;
    VkPushConstantRange push_constant;
    VkPipelineLayoutCreateInfo pipeline_layout_info;
    VkPipelineShaderStageCreateInfo shader_stages[2];
    VkVertexInputBindingDescription vertex_binding;
    VkVertexInputAttributeDescription vertex_attributes[2];
    VkPipelineVertexInputStateCreateInfo vertex_input;
    VkPipelineInputAssemblyStateCreateInfo input_assembly;
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineViewportStateCreateInfo viewport_state;
    VkPipelineRasterizationStateCreateInfo rasterization;
    VkPipelineMultisampleStateCreateInfo multisampling;
    VkPipelineDepthStencilStateCreateInfo depth_stencil;
    VkPipelineColorBlendAttachmentState color_blend_attachment;
    VkPipelineColorBlendStateCreateInfo color_blending;
    VkGraphicsPipelineCreateInfo pipeline_info;
    VkDescriptorBufferInfo descriptor_buffer_info;
    VkWriteDescriptorSet descriptor_write;
    VulkanVertex cube_vertices[24];
    uint16_t cube_indices[36];
    VulkanVertex plane_vertices[4];
    uint16_t plane_indices[6];
    uint32_t i;
    VkResult result;

    if (create_shader_module(state, "cube.vert.spv",
                             &state->vertex_shader_module) != APP_RESULT_OK ||
        create_shader_module(state, "cube.frag.spv",
                             &state->fragment_shader_module) != APP_RESULT_OK)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    fill_cube_vertices(cube_vertices);
    fill_cube_indices(cube_indices);
    fill_plane_vertices(plane_vertices);
    fill_plane_indices(plane_indices);
    if (upload_host_visible_buffer(
            state, sizeof(cube_vertices),
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            cube_vertices, &state->cube_vertex_buffer,
            &state->cube_vertex_memory) != APP_RESULT_OK ||
        upload_host_visible_buffer(
            state, sizeof(cube_indices),
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            cube_indices, &state->cube_index_buffer,
            &state->cube_index_memory) != APP_RESULT_OK ||
        upload_host_visible_buffer(
            state, sizeof(plane_vertices),
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            plane_vertices, &state->plane_vertex_buffer,
            &state->plane_vertex_memory) != APP_RESULT_OK ||
        upload_host_visible_buffer(
            state, sizeof(plane_indices),
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            plane_indices, &state->plane_index_buffer,
            &state->plane_index_memory) != APP_RESULT_OK)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    memset(&descriptor_binding, 0, sizeof(descriptor_binding));
    descriptor_binding.binding = 0;
    descriptor_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_binding.descriptorCount = 1;
    descriptor_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    memset(&descriptor_layout_info, 0, sizeof(descriptor_layout_info));
    descriptor_layout_info.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout_info.bindingCount = 1;
    descriptor_layout_info.pBindings = &descriptor_binding;
    if (vkCreateDescriptorSetLayout(
            state->device, &descriptor_layout_info, NULL,
            &state->descriptor_set_layout) != VK_SUCCESS)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    memset(&push_constant, 0, sizeof(push_constant));
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constant.offset = 0;
    push_constant.size = sizeof(VulkanPushConstants);
    memset(&pipeline_layout_info, 0, sizeof(pipeline_layout_info));
    pipeline_layout_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &state->descriptor_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_constant;
    if (vkCreatePipelineLayout(state->device, &pipeline_layout_info, NULL,
                               &state->pipeline_layout) != VK_SUCCESS)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    state->uniform_buffers =
        (VkBuffer *)calloc(state->in_flight_count, sizeof(VkBuffer));
    state->uniform_memories =
        (VkDeviceMemory *)calloc(state->in_flight_count,
                                 sizeof(VkDeviceMemory));
    state->uniform_mappings =
        (void **)calloc(state->in_flight_count, sizeof(void *));
    state->descriptor_sets =
        (VkDescriptorSet *)calloc(state->in_flight_count,
                                  sizeof(VkDescriptorSet));
    if (state->uniform_buffers == NULL ||
        state->uniform_memories == NULL ||
        state->uniform_mappings == NULL ||
        state->descriptor_sets == NULL)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    for (i = 0; i < state->in_flight_count; ++i)
    {
        if (create_buffer(state,
                          sizeof(VulkanUniformBufferObject),
                          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          &state->uniform_buffers[i],
                          &state->uniform_memories[i]) != APP_RESULT_OK ||
            vkMapMemory(state->device, state->uniform_memories[i], 0,
                        sizeof(VulkanUniformBufferObject), 0,
                        &state->uniform_mappings[i]) != VK_SUCCESS)
        {
            return APP_RESULT_INITIALIZATION_FAILED;
        }
    }

    memset(&pool_size, 0, sizeof(pool_size));
    pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size.descriptorCount = state->in_flight_count;
    memset(&pool_info, 0, sizeof(pool_info));
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    pool_info.maxSets = state->in_flight_count;
    if (vkCreateDescriptorPool(state->device, &pool_info, NULL,
                               &state->descriptor_pool) != VK_SUCCESS)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    memset(&descriptor_allocate_info, 0,
           sizeof(descriptor_allocate_info));
    descriptor_allocate_info.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_allocate_info.descriptorPool = state->descriptor_pool;
    descriptor_allocate_info.descriptorSetCount = 1;
    descriptor_allocate_info.pSetLayouts =
        &state->descriptor_set_layout;
    for (i = 0; i < state->in_flight_count; ++i)
    {
        if (vkAllocateDescriptorSets(
                state->device, &descriptor_allocate_info,
                &state->descriptor_sets[i]) != VK_SUCCESS)
        {
            return APP_RESULT_INITIALIZATION_FAILED;
        }
        memset(&descriptor_buffer_info, 0,
               sizeof(descriptor_buffer_info));
        descriptor_buffer_info.buffer = state->uniform_buffers[i];
        descriptor_buffer_info.offset = 0;
        descriptor_buffer_info.range =
            sizeof(VulkanUniformBufferObject);
        memset(&descriptor_write, 0, sizeof(descriptor_write));
        descriptor_write.sType =
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = state->descriptor_sets[i];
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType =
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.pBufferInfo = &descriptor_buffer_info;
        vkUpdateDescriptorSets(state->device, 1, &descriptor_write,
                               0, NULL);
    }

    memset(&shader_stages, 0, sizeof(shader_stages));
    shader_stages[0].sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages[0].module = state->vertex_shader_module;
    shader_stages[0].pName = "main";
    shader_stages[1].sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stages[1].module = state->fragment_shader_module;
    shader_stages[1].pName = "main";

    memset(&vertex_binding, 0, sizeof(vertex_binding));
    vertex_binding.binding = 0;
    vertex_binding.stride = sizeof(VulkanVertex);
    vertex_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    memset(vertex_attributes, 0, sizeof(vertex_attributes));
    vertex_attributes[0].location = 0;
    vertex_attributes[0].binding = 0;
    vertex_attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_attributes[0].offset = 0;
    vertex_attributes[1].location = 1;
    vertex_attributes[1].binding = 0;
    vertex_attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_attributes[1].offset = offsetof(VulkanVertex, normal);
    memset(&vertex_input, 0, sizeof(vertex_input));
    vertex_input.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input.vertexBindingDescriptionCount = 1;
    vertex_input.pVertexBindingDescriptions = &vertex_binding;
    vertex_input.vertexAttributeDescriptionCount = 2;
    vertex_input.pVertexAttributeDescriptions = vertex_attributes;

    memset(&input_assembly, 0, sizeof(input_assembly));
    input_assembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)state->swapchain_extent.width;
    viewport.height = (float)state->swapchain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = state->swapchain_extent;
    memset(&viewport_state, 0, sizeof(viewport_state));
    viewport_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    memset(&rasterization, 0, sizeof(rasterization));
    rasterization.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization.lineWidth = 1.0f;
    rasterization.cullMode = VK_CULL_MODE_NONE;
    rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    memset(&multisampling, 0, sizeof(multisampling));
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    memset(&depth_stencil, 0, sizeof(depth_stencil));
    depth_stencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    memset(&color_blend_attachment, 0,
           sizeof(color_blend_attachment));
    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    memset(&color_blending, 0, sizeof(color_blending));
    color_blending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;

    memset(&pipeline_info, 0, sizeof(pipeline_info));
    pipeline_info.sType =
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterization;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.layout = state->pipeline_layout;
    pipeline_info.renderPass = state->render_pass;
    pipeline_info.subpass = 0;
    result = vkCreateGraphicsPipelines(
        state->device, VK_NULL_HANDLE, 1, &pipeline_info, NULL,
        &state->graphics_pipeline);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "Vulkan graphics-pipeline creation failed with VkResult %d.\n",
                (int)result);
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    return APP_RESULT_OK;
}

static AppResult create_swapchain_resources(VulkanState *state)
{
    VkSurfaceCapabilitiesKHR capabilities;
    VkSwapchainCreateInfoKHR swapchain_info;
    VkImageViewCreateInfo view_info;
    VkCommandPoolCreateInfo pool_info;
    VkCommandBufferAllocateInfo allocate_info;
    VkSemaphoreCreateInfo semaphore_info;
    VkFenceCreateInfo fence_info;
    VkResult result;
    VkSwapchainKHR old_swapchain;
    uint32_t i;

    cleanup_swapchain_resources(state);
    if (find_depth_format(state) != APP_RESULT_OK)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        state->physical_device, state->surface, &capabilities);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "Vulkan swap-chain recreation failed while querying "
                "surface capabilities (VkResult %d).\n", (int)result);
        return APP_RESULT_RUNTIME_FAILED;
    }
    state->swapchain_extent = capabilities.currentExtent;
    if (state->swapchain_extent.width == UINT32_MAX)
    {
        state->swapchain_extent.width =
            capabilities.minImageExtent.width;
    }
    if (state->swapchain_extent.height == UINT32_MAX)
    {
        state->swapchain_extent.height =
            capabilities.minImageExtent.height;
    }
    if (state->swapchain_extent.width <
            capabilities.minImageExtent.width)
    {
        state->swapchain_extent.width =
            capabilities.minImageExtent.width;
    }
    else if (state->swapchain_extent.width >
             capabilities.maxImageExtent.width)
    {
        state->swapchain_extent.width =
            capabilities.maxImageExtent.width;
    }
    if (state->swapchain_extent.height <
            capabilities.minImageExtent.height)
    {
        state->swapchain_extent.height =
            capabilities.minImageExtent.height;
    }
    else if (state->swapchain_extent.height >
             capabilities.maxImageExtent.height)
    {
        state->swapchain_extent.height =
            capabilities.maxImageExtent.height;
    }

    if (capabilities.minImageCount == 0)
    {
        fprintf(stderr,
                "Vulkan swap-chain recreation failed: the surface "
                "reports no usable image count.\n");
        return APP_RESULT_RUNTIME_FAILED;
    }
    old_swapchain = state->swapchain;
    memset(&swapchain_info, 0, sizeof(swapchain_info));
    swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_info.surface = state->surface;
    swapchain_info.minImageCount = capabilities.minImageCount;
    if (capabilities.maxImageCount != 0 &&
        swapchain_info.minImageCount > capabilities.maxImageCount)
    {
        swapchain_info.minImageCount = capabilities.maxImageCount;
    }
    swapchain_info.imageFormat = state->surface_format.format;
    swapchain_info.imageColorSpace = state->surface_format.colorSpace;
    swapchain_info.imageExtent = state->swapchain_extent;
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_info.preTransform = capabilities.currentTransform;
    swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_info.presentMode = state->present_mode;
    swapchain_info.clipped = VK_TRUE;
    swapchain_info.oldSwapchain = old_swapchain;

    result = vkCreateSwapchainKHR(state->device,
                                  &swapchain_info,
                                  NULL,
                                  &state->swapchain);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "Vulkan swap-chain creation failed with VkResult %d.\n",
                (int)result);
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    if (old_swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(state->device, old_swapchain, NULL);
    }

    result = vkGetSwapchainImagesKHR(state->device,
                                     state->swapchain,
                                     &state->swapchain_image_count,
                                     NULL);
    if (result != VK_SUCCESS || state->swapchain_image_count == 0)
    {
        fprintf(stderr,
                "Vulkan swap-chain image enumeration failed "
                "(VkResult %d).\n", (int)result);
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    state->swapchain_images = (VkImage *)calloc(
        state->swapchain_image_count, sizeof(VkImage));
    state->swapchain_image_views = (VkImageView *)calloc(
        state->swapchain_image_count, sizeof(VkImageView));
    state->framebuffers = (VkFramebuffer *)calloc(
        state->swapchain_image_count, sizeof(VkFramebuffer));
    if (state->swapchain_images == NULL ||
        state->swapchain_image_views == NULL ||
        state->framebuffers == NULL)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    if (vkGetSwapchainImagesKHR(state->device,
                                state->swapchain,
                                &state->swapchain_image_count,
                                state->swapchain_images) != VK_SUCCESS)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    for (i = 0; i < state->swapchain_image_count; ++i)
    {
        memset(&view_info, 0, sizeof(view_info));
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = state->swapchain_images[i];
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = state->surface_format.format;
        view_info.subresourceRange.aspectMask =
            VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.layerCount = 1;
        if (vkCreateImageView(state->device, &view_info, NULL,
                              &state->swapchain_image_views[i]) !=
            VK_SUCCESS)
        {
            fprintf(stderr,
                    "Vulkan swap-chain image-view creation failed "
                    "for image %u.\n", i);
            return APP_RESULT_INITIALIZATION_FAILED;
        }
    }

    if (create_render_pass(state) != APP_RESULT_OK ||
        create_depth_resources(state) != APP_RESULT_OK)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    for (i = 0; i < state->swapchain_image_count; ++i)
    {
        VkImageView attachments[2];
        VkFramebufferCreateInfo framebuffer_info;
        attachments[0] = state->swapchain_image_views[i];
        attachments[1] = state->depth_image_view;
        memset(&framebuffer_info, 0, sizeof(framebuffer_info));
        framebuffer_info.sType =
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = state->render_pass;
        framebuffer_info.attachmentCount = 2;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = state->swapchain_extent.width;
        framebuffer_info.height = state->swapchain_extent.height;
        framebuffer_info.layers = 1;
        if (vkCreateFramebuffer(state->device,
                                &framebuffer_info,
                                NULL,
                                &state->framebuffers[i]) != VK_SUCCESS)
        {
            fprintf(stderr,
                    "Vulkan frame-buffer creation failed for "
                    "swap-chain image %u.\n", i);
            return APP_RESULT_INITIALIZATION_FAILED;
        }
    }

    state->in_flight_count = vulkan_choose_in_flight_limit(
        state->swapchain_image_count, 2);
    if (create_pipeline_resources(state) != APP_RESULT_OK)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    memset(&pool_info, 0, sizeof(pool_info));
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = state->graphics_queue_family;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (vkCreateCommandPool(state->device, &pool_info, NULL,
                            &state->command_pool) != VK_SUCCESS)
    {
        fprintf(stderr,
                "Vulkan graphics command-pool creation failed.\n");
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    state->command_buffers = (VkCommandBuffer *)calloc(
        state->in_flight_count, sizeof(VkCommandBuffer));
    state->image_available_semaphores =
        (VkSemaphore *)calloc(state->in_flight_count,
                              sizeof(VkSemaphore));
    state->render_finished_semaphores =
        (VkSemaphore *)calloc(state->in_flight_count,
                              sizeof(VkSemaphore));
    state->in_flight_fences =
        (VkFence *)calloc(state->in_flight_count, sizeof(VkFence));
    if (state->command_buffers == NULL ||
        state->image_available_semaphores == NULL ||
        state->render_finished_semaphores == NULL ||
        state->in_flight_fences == NULL)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    memset(&allocate_info, 0, sizeof(allocate_info));
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = state->command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = state->in_flight_count;
    if (vkAllocateCommandBuffers(state->device, &allocate_info,
                                 state->command_buffers) != VK_SUCCESS)
    {
        fprintf(stderr,
                "Vulkan command-buffer allocation failed.\n");
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    memset(&semaphore_info, 0, sizeof(semaphore_info));
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    memset(&fence_info, 0, sizeof(fence_info));
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (i = 0; i < state->in_flight_count; ++i)
    {
        if (vkCreateSemaphore(state->device, &semaphore_info, NULL,
                              &state->image_available_semaphores[i]) !=
                VK_SUCCESS ||
            vkCreateSemaphore(state->device, &semaphore_info, NULL,
                              &state->render_finished_semaphores[i]) !=
                VK_SUCCESS ||
            vkCreateFence(state->device, &fence_info, NULL,
                          &state->in_flight_fences[i]) != VK_SUCCESS)
        {
            fprintf(stderr,
                    "Vulkan frame synchronization object creation "
                    "failed for in-flight frame %u.\n", i);
            return APP_RESULT_INITIALIZATION_FAILED;
        }
    }
    state->current_frame = 0;
    return APP_RESULT_OK;
}

AppResult vulkan_init(VulkanState **state, void *native_window)
{
    VkApplicationInfo app_info;
    VkInstanceCreateInfo instance_info;
    uint32_t extension_count = 0;
#ifdef GAME_ENABLE_VULKAN_VALIDATION
    uint32_t layer_count = 0;
#endif
    VkExtensionProperties *extensions = NULL;
#ifdef GAME_ENABLE_VULKAN_VALIDATION
    VkLayerProperties *layers = NULL;
#endif
    const char *required_extensions[3];
#ifdef GAME_ENABLE_VULKAN_VALIDATION
    const char *required_layers[1];
#endif
    VkResult result;
    VulkanState *new_state;

    if (state == NULL || native_window == NULL)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    new_state = (VulkanState *)calloc(1, sizeof(VulkanState));
    if (new_state == NULL)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    new_state->native_window = native_window;

    result = vkEnumerateInstanceExtensionProperties(
        NULL, &extension_count, NULL);
    if (result == VK_SUCCESS && extension_count > 0)
    {
        extensions = (VkExtensionProperties *)malloc(
            (size_t)extension_count * sizeof(VkExtensionProperties));
        if (extensions != NULL)
        {
            result = vkEnumerateInstanceExtensionProperties(
                NULL, &extension_count, extensions);
        }
    }

    required_extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
    required_extensions[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
    required_extensions[2] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    if (extensions == NULL ||
        result != VK_SUCCESS ||
        !has_extension(required_extensions[0], extensions,
                       extension_count) ||
        !has_extension(required_extensions[1], extensions,
                       extension_count)
        )
    {
        fprintf(stderr,
                "Vulkan instance initialization failed: required "
                "instance extensions VK_KHR_surface and "
                "VK_KHR_win32_surface are unavailable. Install or "
                "update the Vulkan runtime.\n");
        free(extensions);
        free(new_state);
        return APP_RESULT_INITIALIZATION_FAILED;
    }
#ifdef GAME_ENABLE_VULKAN_VALIDATION
    if (!has_extension(required_extensions[2], extensions,
                       extension_count))
    {
        fprintf(stderr,
                "Debug Vulkan validation cannot be enabled reliably: "
                "required instance extension VK_EXT_debug_utils is "
                "unavailable. Install or update the Vulkan SDK runtime "
                "and retry the Debug build.\n");
        free(extensions);
        free(new_state);
        return APP_RESULT_INITIALIZATION_FAILED;
    }
#endif
    free(extensions);

#ifdef GAME_ENABLE_VULKAN_VALIDATION
    result = vkEnumerateInstanceLayerProperties(
        &layer_count, NULL);
    if (result == VK_SUCCESS && layer_count > 0)
    {
        layers = (VkLayerProperties *)malloc(
            (size_t)layer_count * sizeof(VkLayerProperties));
        if (layers != NULL)
        {
            result = vkEnumerateInstanceLayerProperties(
                &layer_count, layers);
        }
    }
    required_layers[0] = "VK_LAYER_KHRONOS_validation";
    if (layers == NULL ||
        result != VK_SUCCESS ||
        !has_layer(required_layers[0], layers, layer_count))
    {
        fprintf(stderr,
                "Debug Vulkan validation is unavailable: "
                "VK_LAYER_KHRONOS_validation was not found. Install "
                "the Vulkan SDK validation layers and run the Debug "
                "build again.\n");
        free(layers);
        free(new_state);
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    free(layers);
#endif

    memset(&app_info, 0, sizeof(app_info));
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Vulkan Isometric RPG";
    app_info.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
    app_info.apiVersion = VK_API_VERSION_1_1;

    memset(&instance_info, 0, sizeof(instance_info));
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &app_info;
    instance_info.enabledExtensionCount =
#ifdef GAME_ENABLE_VULKAN_VALIDATION
        3;
#else
        2;
#endif
    instance_info.ppEnabledExtensionNames = required_extensions;
#ifdef GAME_ENABLE_VULKAN_VALIDATION
    instance_info.enabledLayerCount = 1;
    instance_info.ppEnabledLayerNames = required_layers;
#endif

    result = vkCreateInstance(&instance_info, NULL,
                              &new_state->instance);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "Vulkan instance creation failed with VkResult %d.\n",
                (int)result);
        free(new_state);
        return APP_RESULT_INITIALIZATION_FAILED;
    }

#ifdef GAME_ENABLE_VULKAN_VALIDATION
    if (create_debug_messenger(new_state->instance,
                               &new_state->debug_messenger) !=
        VK_SUCCESS)
    {
        fprintf(stderr,
                "Debug Vulkan validation messenger creation failed. "
                "Install compatible Vulkan SDK validation support and "
                "retry.\n");
        vkDestroyInstance(new_state->instance, NULL);
        free(new_state);
        return APP_RESULT_INITIALIZATION_FAILED;
    }
#endif

    if (create_surface(new_state) != APP_RESULT_OK)
    {
        if (new_state->debug_messenger != VK_NULL_HANDLE)
        {
            destroy_debug_messenger(new_state->instance,
                                    new_state->debug_messenger);
        }
        vkDestroyInstance(new_state->instance, NULL);
        free(new_state);
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    if (select_device_and_queues(new_state) != APP_RESULT_OK ||
        create_logical_device(new_state) != APP_RESULT_OK ||
        select_surface_configuration(new_state) != APP_RESULT_OK)
    {
        vulkan_shutdown(new_state);
        return APP_RESULT_INITIALIZATION_FAILED;
    }
    if (create_swapchain_resources(new_state) != APP_RESULT_OK)
    {
        vulkan_shutdown(new_state);
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    *state = new_state;
    fprintf(stdout,
            "Vulkan initialized: instance, physical/logical devices, "
            "graphics/presentation queues, and Win32 surface ready.\n");
    return APP_RESULT_OK;
}

AppResult vulkan_recreate_surface(VulkanState *state)
{
    if (state == NULL)
    {
        return APP_RESULT_RUNTIME_FAILED;
    }
    if (select_surface_configuration(state) != APP_RESULT_OK)
    {
        return APP_RESULT_RUNTIME_FAILED;
    }
    if (create_swapchain_resources(state) != APP_RESULT_OK)
    {
        return APP_RESULT_RUNTIME_FAILED;
    }
    return APP_RESULT_OK;
}

AppResult vulkan_render_frame(VulkanState *state, const App *app)
{
    VkSemaphore wait_semaphores[1];
    VkPipelineStageFlags wait_stages[1];
    VkSubmitInfo submit_info;
    VkPresentInfoKHR present_info;
    VkClearValue clear_values[2];
    VkCommandBufferBeginInfo begin_info;
    VkRenderPassBeginInfo render_pass_info;
    VkRect2D render_area;
    VulkanUniformBufferObject uniform;
    VulkanPushConstants push_constants;
    VulkanFixedCamera camera;
    float view_projection[16];
    VkDeviceSize vertex_offset;
    VulkanSceneObject scene[VULKAN_LEVEL_MAX_SCENE_OBJECTS];
    VulkanSceneObject player_scene[5];
    uint32_t scene_index;
    uint32_t scene_count = 0;
    uint32_t player_scene_count = 0;
    uint32_t player_scene_index;
    uint32_t image_index = 0;
    VkResult result;

    if (state == NULL || state->device == VK_NULL_HANDLE ||
        state->swapchain == VK_NULL_HANDLE)
    {
        return APP_RESULT_RUNTIME_FAILED;
    }

    result = vkWaitForFences(
        state->device, 1, &state->in_flight_fences[state->current_frame],
        VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "Vulkan frame fence wait failed with VkResult %d.\n",
                (int)result);
        return APP_RESULT_RUNTIME_FAILED;
    }

    result = vkAcquireNextImageKHR(
        state->device,
        state->swapchain,
        UINT64_MAX,
        state->image_available_semaphores[state->current_frame],
        VK_NULL_HANDLE,
        &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR ||
        result == VK_SUBOPTIMAL_KHR)
    {
        if (vulkan_recreate_surface(state) != APP_RESULT_OK)
        {
            return APP_RESULT_RUNTIME_FAILED;
        }
        return APP_RESULT_OK;
    }
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "Vulkan image acquisition failed with VkResult %d.\n",
                (int)result);
        return APP_RESULT_RUNTIME_FAILED;
    }

    if (vkResetFences(state->device, 1,
                      &state->in_flight_fences[state->current_frame]) !=
        VK_SUCCESS)
    {
        fprintf(stderr, "Vulkan frame fence reset failed.\n");
        return APP_RESULT_RUNTIME_FAILED;
    }
    if (vkResetCommandBuffer(
            state->command_buffers[state->current_frame], 0) !=
        VK_SUCCESS)
    {
        fprintf(stderr, "Vulkan command-buffer reset failed.\n");
        return APP_RESULT_RUNTIME_FAILED;
    }

    memset(&begin_info, 0, sizeof(begin_info));
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(
            state->command_buffers[state->current_frame],
            &begin_info) != VK_SUCCESS)
    {
        fprintf(stderr, "Vulkan command-buffer begin failed.\n");
        return APP_RESULT_RUNTIME_FAILED;
    }

    if (!vulkan_get_fixed_camera(&camera))
    {
        return APP_RESULT_RUNTIME_FAILED;
    }
    if (!vulkan_build_fixed_view_projection(
            (float)state->swapchain_extent.width /
                (float)state->swapchain_extent.height,
            &camera,
            view_projection))
    {
        return APP_RESULT_RUNTIME_FAILED;
    }
    memcpy(uniform.view_projection, view_projection,
           sizeof(uniform.view_projection));
    memcpy(state->uniform_mappings[state->current_frame],
           &uniform, sizeof(uniform));

    memset(clear_values, 0, sizeof(clear_values));
    clear_values[0].color.float32[0] = 0.08f;
    clear_values[0].color.float32[1] = 0.12f;
    clear_values[0].color.float32[2] = 0.20f;
    clear_values[0].color.float32[3] = 1.0f;
    clear_values[1].depthStencil.depth = 1.0f;
    clear_values[1].depthStencil.stencil = 0;

    memset(&render_area, 0, sizeof(render_area));
    render_area.extent = state->swapchain_extent;
    memset(&render_pass_info, 0, sizeof(render_pass_info));
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = state->render_pass;
    render_pass_info.framebuffer = state->framebuffers[image_index];
    render_pass_info.renderArea = render_area;
    render_pass_info.clearValueCount = 2;
    render_pass_info.pClearValues = clear_values;
    vkCmdBeginRenderPass(
        state->command_buffers[state->current_frame],
        &render_pass_info,
        VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(
        state->command_buffers[state->current_frame],
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        state->graphics_pipeline);
    vkCmdBindDescriptorSets(
        state->command_buffers[state->current_frame],
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        state->pipeline_layout,
        0, 1, &state->descriptor_sets[state->current_frame],
        0, NULL);
    if (!vulkan_build_level_scene(
            scene, VULKAN_LEVEL_MAX_SCENE_OBJECTS, &scene_count))
    {
        return APP_RESULT_RUNTIME_FAILED;
    }
    if (app != NULL &&
        vulkan_build_player_scene(
            app, player_scene, 5, &player_scene_count))
    {
        for (player_scene_index = 0;
             player_scene_index < player_scene_count &&
             scene_count < VULKAN_LEVEL_MAX_SCENE_OBJECTS;
             ++player_scene_index)
        {
            scene[scene_count] = player_scene[player_scene_index];
            ++scene_count;
        }
    }

    vertex_offset = 0;
    for (scene_index = 0; scene_index < scene_count; ++scene_index)
    {
        memcpy(push_constants.model, scene[scene_index].model,
               sizeof(push_constants.model));
        memcpy(push_constants.color, scene[scene_index].color,
               sizeof(push_constants.color));
        vkCmdPushConstants(
            state->command_buffers[state->current_frame],
            state->pipeline_layout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0, sizeof(push_constants), &push_constants);
        if (scene[scene_index].mesh_type == VULKAN_MESH_PLANE)
        {
            vkCmdBindVertexBuffers(
                state->command_buffers[state->current_frame],
                0, 1, &state->plane_vertex_buffer, &vertex_offset);
            vkCmdBindIndexBuffer(
                state->command_buffers[state->current_frame],
                state->plane_index_buffer, 0,
                VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(
                state->command_buffers[state->current_frame],
                vulkan_plane_index_count(), 1, 0, 0, 0);
        }
        else
        {
            vkCmdBindVertexBuffers(
                state->command_buffers[state->current_frame],
                0, 1, &state->cube_vertex_buffer, &vertex_offset);
            vkCmdBindIndexBuffer(
                state->command_buffers[state->current_frame],
                state->cube_index_buffer, 0,
                VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(
                state->command_buffers[state->current_frame],
                vulkan_cube_index_count(), 1, 0, 0, 0);
        }
    }
    vkCmdEndRenderPass(
        state->command_buffers[state->current_frame]);
    if (vkEndCommandBuffer(
            state->command_buffers[state->current_frame]) != VK_SUCCESS)
    {
        fprintf(stderr, "Vulkan command-buffer recording failed.\n");
        return APP_RESULT_RUNTIME_FAILED;
    }

    wait_semaphores[0] =
        state->image_available_semaphores[state->current_frame];
    wait_stages[0] =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    memset(&submit_info, 0, sizeof(submit_info));
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers =
        &state->command_buffers[state->current_frame];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores =
        &state->render_finished_semaphores[state->current_frame];
    if (vkQueueSubmit(state->graphics_queue, 1, &submit_info,
                      state->in_flight_fences[state->current_frame]) !=
        VK_SUCCESS)
    {
        fprintf(stderr, "Vulkan frame submission failed.\n");
        return APP_RESULT_RUNTIME_FAILED;
    }

    memset(&present_info, 0, sizeof(present_info));
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores =
        &state->render_finished_semaphores[state->current_frame];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &state->swapchain;
    present_info.pImageIndices = &image_index;
    result = vkQueuePresentKHR(state->present_queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR ||
        result == VK_SUBOPTIMAL_KHR)
    {
        if (vulkan_recreate_surface(state) != APP_RESULT_OK)
        {
            return APP_RESULT_RUNTIME_FAILED;
        }
    }
    else if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "Vulkan presentation failed with VkResult %d.\n",
                (int)result);
        return APP_RESULT_RUNTIME_FAILED;
    }

    state->current_frame =
        (state->current_frame + 1) % state->in_flight_count;
    return APP_RESULT_OK;
}

void vulkan_shutdown(VulkanState *state)
{
    if (state == NULL)
    {
        return;
    }
    cleanup_swapchain_resources(state);
    if (state->swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(state->device,
                              state->swapchain,
                              NULL);
        state->swapchain = VK_NULL_HANDLE;
    }
    if (state->device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(state->device, NULL);
        state->device = VK_NULL_HANDLE;
    }
    if (state->surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(state->instance, state->surface, NULL);
        state->surface = VK_NULL_HANDLE;
    }
    if (state->debug_messenger != VK_NULL_HANDLE)
    {
        destroy_debug_messenger(state->instance,
                                state->debug_messenger);
        state->debug_messenger = VK_NULL_HANDLE;
    }
    if (state->instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(state->instance, NULL);
        state->instance = VK_NULL_HANDLE;
    }
    free(state);
}
#endif
