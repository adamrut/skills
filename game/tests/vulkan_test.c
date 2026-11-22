#include "../src/vulkan.h"

#include <stdio.h>
#include <math.h>

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
    VulkanFixedCamera camera;
    VulkanLevelBounds bounds;
    float scene_center[3];
    float camera_position[3];
    float fixed_view[16];
    float fixed_view_projection[16];
    uint32_t level_width;
    uint32_t level_height;
    uint32_t row;
    uint32_t column;
    VulkanLevelTile level_tile;
    float level_world[3];
    float player_spawn[3];
    VulkanSceneObject level_scene[VULKAN_LEVEL_MAX_SCENE_OBJECTS];
    uint32_t level_scene_count;
    VulkanSceneObject player_scene[5];
    uint32_t player_scene_count;
    App app;

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
        vulkan_scene_object_count() == 98,
        "level scene contains 64 ground tiles and 34 wall or obstacle cubes");
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
    failures |= expect_true(
        vulkan_get_fixed_camera(&camera) &&
            camera.eye[0] == 7.5f &&
            camera.eye[1] == 7.5f &&
            camera.eye[2] == 7.5f &&
            camera.target[0] == 0.0f &&
            camera.target[1] == 0.0f &&
            camera.target[2] == 0.0f &&
            camera.world_up[1] == 1.0f &&
            camera.near_plane == 0.1f &&
            camera.far_plane == 100.0f,
        "fixed camera uses a documented isometric eye, target, up, and depth range");
    failures |= expect_true(
        vulkan_get_scene_center(scene_center) &&
            scene_center[0] == 0.0f &&
            scene_center[1] == 0.0f &&
            scene_center[2] == 0.0f &&
            vulkan_get_fixed_camera(&camera) &&
            camera.target[0] == scene_center[0] &&
            camera.target[1] == scene_center[1] &&
            camera.target[2] == scene_center[2],
        "fixed camera target matches the existing procedural scene center");
    failures |= expect_true(
        vulkan_get_fixed_level_bounds(&bounds) &&
            vulkan_build_fixed_view(&camera, fixed_view) &&
            fixed_view[15] == 1.0f &&
            vulkan_build_fixed_view_projection(
                16.0f / 9.0f, &camera, fixed_view_projection),
        "fixed view and projection construct valid separate camera matrices");
    camera_position[0] = 20.0f;
    camera_position[1] = -20.0f;
    camera_position[2] = 20.0f;
    failures |= expect_true(
        vulkan_get_fixed_level_bounds(&bounds) &&
            vulkan_clamp_camera_position(camera_position, &bounds) &&
            camera_position[0] == bounds.max[0] &&
            camera_position[1] == bounds.min[1] &&
            camera_position[2] == bounds.max[2],
        "camera preview positions are clamped to fixed level boundaries");
    failures |= expect_true(
        vulkan_build_fixed_view_projection(
            0.5f, &camera, fixed_view_projection) &&
            vulkan_build_fixed_view_projection(
                2.0f, &camera, fixed_view_projection) &&
            isfinite(fixed_view_projection[0]) &&
            isfinite(fixed_view_projection[5]) &&
            isfinite(fixed_view_projection[10]) &&
            isfinite(fixed_view_projection[14]),
        "fixed view projection remains finite across supported aspect ratios");
    failures |= expect_true(
        vulkan_get_level_dimensions(&level_width, &level_height) &&
            level_width == 8 &&
            level_height == 8,
        "level dimensions are one documented 8 by 8 tile grid");
    failures |= expect_true(
        vulkan_get_player_spawn(player_spawn) &&
            vulkan_is_world_position_walkable(player_spawn) &&
            player_spawn[0] == -2.5f &&
            player_spawn[1] == VULKAN_LEVEL_GROUND_HEIGHT &&
            player_spawn[2] == -2.5f,
        "documented player spawn lies on a walkable tile");
    failures |= expect_true(
        vulkan_build_level_scene(level_scene,
                                 VULKAN_LEVEL_MAX_SCENE_OBJECTS,
                                 &level_scene_count) &&
            level_scene_count == 98,
        "level scene helper builds ground, walls, and decorative obstacles");

    failures |= expect_true(app_init(&app) == APP_RESULT_OK,
                            "player scene test initializes application state");
    failures |= expect_true(vulkan_get_player_spawn(player_spawn),
                            "player scene test obtains the fixed spawn");
    app_set_player_spawn(&app, player_spawn);
    failures |= expect_true(
        vulkan_build_player_scene(&app, player_scene, 5,
                                  &player_scene_count) &&
            player_scene_count == 5 &&
            player_scene[0].mesh_type == VULKAN_MESH_CUBE &&
            player_scene[1].mesh_type == VULKAN_MESH_CUBE &&
            player_scene[4].color[0] == 1.0f,
        "procedural player scene builds distinct humanoid meshes and facing marker");

    for (row = 0; row < 8; ++row)
    {
        for (column = 0; column < 8; ++column)
        {
            failures |= expect_true(
                vulkan_get_level_tile(row, column, &level_tile) &&
                    vulkan_tile_to_world(row, column, level_world) &&
                    vulkan_is_world_position_walkable(level_world) ==
                        level_tile.walkable,
                "every tile has consistent level coordinates and walkability");
        }
    }
    failures |= expect_true(
        vulkan_get_level_tile(0, 0, &level_tile) &&
            level_tile.collision &&
            !level_tile.walkable &&
            vulkan_get_level_tile(1, 1, &level_tile) &&
            level_tile.walkable &&
            !level_tile.collision &&
            vulkan_get_level_tile(2, 5, &level_tile) &&
            level_tile.obstacle &&
            level_tile.collision &&
            !level_tile.walkable,
        "boundaries, open spawn area, and obstacle collision states are consistent");

    if (failures != 0)
    {
        return 1;
    }
    puts("Game Vulkan tests passed.");
    return 0;
}
