#include "app.h"

#include <stddef.h>
#include <math.h>
#include <stdint.h>

#define APP_PLAYER_MOVE_SPEED 2.5f
#define APP_PLAYER_COLLISION_RADIUS 0.25f
#define APP_PLAYER_LEVEL_LIMIT 3.25f

static void app_reset_runtime(App *app)
{
    app->accumulator_seconds = 0.0;
    app->previous_time_seconds = 0.0;
    app->frame_index = 0;
    app->fixed_update_count = 0;
    app->manually_paused = false;
    app->restart_requested = false;
    app->player.position[0] = app->player.spawn[0];
    app->player.position[1] = app->player.spawn[1];
    app->player.position[2] = app->player.spawn[2];
    app->player.state = APP_PLAYER_IDLE;
    app->player.facing = APP_FACING_SOUTH;
    for (int key = 0; key < APP_KEY_COUNT; ++key)
    {
        app->held_keys[key] = false;
    }
}

AppResult app_init(App *app)
{
    if (app == NULL)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    app->vulkan_state = NULL;
    app->running = true;
    app->close_requested = false;
    app->inactive_paused = false;
    app->level_walkable = NULL;
    app->player.spawn[0] = 0.0f;
    app->player.spawn[1] = -0.7f;
    app->player.spawn[2] = 0.0f;
    app->player.movement_speed = APP_PLAYER_MOVE_SPEED;
    app->player.collision_radius = APP_PLAYER_COLLISION_RADIUS;
    app_reset_runtime(app);
    return APP_RESULT_OK;
}

void app_shutdown(App *app)
{
    if (app != NULL)
    {
        app->running = false;
        app_reset_runtime(app);
    }
}

void app_handle_event(App *app, AppEvent event)
{
    if (app == NULL)
    {
        return;
    }

    switch (event)
    {
    case APP_EVENT_ACTIVATED:
        app->inactive_paused = false;
        break;
    case APP_EVENT_DEACTIVATED:
        app->inactive_paused = true;
        break;
    case APP_EVENT_RESIZED:
        break;
    case APP_EVENT_TOGGLE_PAUSE:
        app->manually_paused = !app->manually_paused;
        break;
    case APP_EVENT_RESTART:
        app_restart(app);
        break;
    case APP_EVENT_CLOSE:
        app->close_requested = true;
        app->running = false;
        break;
    case APP_EVENT_NONE:
    default:
        break;
    }
}

void app_restart(App *app)
{
    if (app != NULL)
    {
        app_reset_runtime(app);
        app->close_requested = false;
        app->running = true;
    }
}

bool app_is_paused(const App *app)
{
    return app == NULL ||
           app->manually_paused ||
           app->inactive_paused ||
           !app->running;
}

bool app_step(App *app, double current_time_seconds)
{
    if (app == NULL)
    {
        return false;
    }

    if (app->restart_requested)
    {
        app_restart(app);
    }

    if (app->previous_time_seconds == 0.0)
    {
        app->previous_time_seconds = current_time_seconds;
    }

    double elapsed_seconds = current_time_seconds - app->previous_time_seconds;
    if (elapsed_seconds < 0.0)
    {
        elapsed_seconds = 0.0;
    }
    app->previous_time_seconds = current_time_seconds;

    if (!app_is_paused(app))
    {
        app->accumulator_seconds += elapsed_seconds;
        if (app->accumulator_seconds > (double)APP_MAX_FIXED_STEPS *
                                            APP_FIXED_STEP_SECONDS)
        {
            app->accumulator_seconds =
                (double)APP_MAX_FIXED_STEPS * APP_FIXED_STEP_SECONDS;
        }

        while (app->accumulator_seconds >= APP_FIXED_STEP_SECONDS)
        {
            app_move_player_fixed(app, APP_FIXED_STEP_SECONDS);
            app->fixed_update_count++;
            app->accumulator_seconds -= APP_FIXED_STEP_SECONDS;
        }
        app->frame_index++;
    }

    return app->running && !app->close_requested;
}

const char *app_result_text(AppResult result)
{
    switch (result)
    {
    case APP_RESULT_OK:
        return "success";
    case APP_RESULT_INITIALIZATION_FAILED:
        return "initialization failed";
    case APP_RESULT_RUNTIME_FAILED:
        return "runtime failed";
    case APP_RESULT_SHUTDOWN_FAILED:
        return "shutdown failed";
    default:
        return "unknown error";
    }
}

void app_set_key(App *app, AppKey key, bool held)
{
    if (app != NULL && key >= APP_KEY_MOVE_LEFT && key < APP_KEY_COUNT)
    {
        app->held_keys[key] = held;
    }
}

void app_set_player_spawn(App *app, const float spawn[3])
{
    if (app != NULL && spawn != NULL)
    {
        app->player.spawn[0] = spawn[0];
        app->player.spawn[1] = spawn[1];
        app->player.spawn[2] = spawn[2];
        app_restart(app);
    }
}

void app_set_level_walkable(App *app, AppWalkablePredicate predicate)
{
    if (app != NULL)
    {
        app->level_walkable = predicate;
    }
}

AppFacingDirection app_facing_from_world_direction(float x, float z)
{
    if (z < -0.35f)
    {
        if (x > 0.35f)
        {
            return APP_FACING_NORTHEAST;
        }
        if (x < -0.35f)
        {
            return APP_FACING_NORTHWEST;
        }
        return APP_FACING_NORTH;
    }
    if (z > 0.35f)
    {
        if (x > 0.35f)
        {
            return APP_FACING_SOUTHEAST;
        }
        if (x < -0.35f)
        {
            return APP_FACING_SOUTHWEST;
        }
        return APP_FACING_SOUTH;
    }
    if (x < 0.0f)
    {
        return APP_FACING_WEST;
    }
    return APP_FACING_EAST;
}

bool app_resolve_player_collision(const float current_position[3],
                                  float candidate_position[3],
                                  float collision_radius,
                                  AppWalkablePredicate predicate)
{
    static const float offsets[8][2] = {
        {1.0f, 0.0f}, {-1.0f, 0.0f},
        {0.0f, 1.0f}, {0.0f, -1.0f},
        {0.70710678f, 0.70710678f},
        {-0.70710678f, 0.70710678f},
        {0.70710678f, -0.70710678f},
        {-0.70710678f, -0.70710678f}
    };
    float test_position[3];
    uint32_t offset_index;

    if (current_position == NULL || candidate_position == NULL ||
        !isfinite(candidate_position[0]) ||
        !isfinite(candidate_position[2]) ||
        collision_radius <= 0.0f)
    {
        return false;
    }

    if (candidate_position[0] < -APP_PLAYER_LEVEL_LIMIT ||
        candidate_position[0] > APP_PLAYER_LEVEL_LIMIT ||
        candidate_position[2] < -APP_PLAYER_LEVEL_LIMIT ||
        candidate_position[2] > APP_PLAYER_LEVEL_LIMIT)
    {
        return false;
    }

    if (predicate != NULL)
    {
        for (offset_index = 0; offset_index < 8; ++offset_index)
        {
            test_position[0] =
                candidate_position[0] +
                offsets[offset_index][0] * collision_radius;
            test_position[1] = candidate_position[1];
            test_position[2] =
                candidate_position[2] +
                offsets[offset_index][1] * collision_radius;
            if (!predicate(test_position))
            {
                return false;
            }
        }
    }

    (void)current_position;
    return true;
}

void app_move_player_fixed(App *app, double fixed_step_seconds)
{
    float horizontal;
    float vertical;
    float world_x;
    float world_z;
    float length;
    float candidate[3];
    float resolved[3];

    if (app == NULL || fixed_step_seconds <= 0.0)
    {
        return;
    }

    horizontal =
        (app->held_keys[APP_KEY_MOVE_RIGHT] ? 1.0f : 0.0f) -
        (app->held_keys[APP_KEY_MOVE_LEFT] ? 1.0f : 0.0f);
    vertical =
        (app->held_keys[APP_KEY_MOVE_DOWN] ? 1.0f : 0.0f) -
        (app->held_keys[APP_KEY_MOVE_UP] ? 1.0f : 0.0f);

    if (horizontal == 0.0f && vertical == 0.0f)
    {
        app->player.state = APP_PLAYER_IDLE;
        return;
    }

    world_x = horizontal + vertical;
    world_z = vertical - horizontal;
    length = sqrtf(world_x * world_x + world_z * world_z);
    if (length > 0.0f)
    {
        world_x /= length;
        world_z /= length;
    }

    candidate[0] =
        app->player.position[0] +
        world_x * app->player.movement_speed *
            (float)fixed_step_seconds;
    candidate[1] = app->player.position[1];
    candidate[2] = app->player.position[2];

    resolved[0] = app->player.position[0];
    resolved[1] = app->player.position[1];
    resolved[2] = app->player.position[2];
    if (app_resolve_player_collision(app->player.position,
                                     candidate,
                                     app->player.collision_radius,
                                     app->level_walkable))
    {
        resolved[0] = candidate[0];
    }

    candidate[0] = resolved[0];
    candidate[1] = resolved[1];
    candidate[2] =
        app->player.position[2] +
        world_z * app->player.movement_speed *
            (float)fixed_step_seconds;
    if (app_resolve_player_collision(app->player.position,
                                     candidate,
                                     app->player.collision_radius,
                                     app->level_walkable))
    {
        resolved[2] = candidate[2];
    }

    app->player.position[0] = resolved[0];
    app->player.position[1] = resolved[1];
    app->player.position[2] = resolved[2];
    app->player.state = APP_PLAYER_MOVING;
    app->player.facing =
        app_facing_from_world_direction(world_x, world_z);
}
