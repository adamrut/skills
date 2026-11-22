#include "../src/app.h"

#include <stdio.h>
#include <math.h>

static bool foundation_walkable(const float world[3])
{
    return world != NULL &&
           world[0] >= -3.25f && world[0] <= 3.25f &&
           world[2] >= -3.25f && world[2] <= 3.25f &&
           !(world[0] > 0.25f && world[0] < 1.25f &&
             world[2] > -0.25f && world[2] < 0.25f);
}

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
    App app;
    int failures = 0;
    float spawn[3] = {-2.5f, -0.7f, -2.5f};
    float candidate[3];

    failures |= expect_true(app_init(&app) == APP_RESULT_OK,
                            "application initialization");
    failures |= expect_true(app.running,
                            "application starts running");

    app_handle_event(&app, APP_EVENT_TOGGLE_PAUSE);
    failures |= expect_true(app_is_paused(&app),
                            "pause event pauses simulation");

    app_step(&app, 1.0);
    failures |= expect_true(app.fixed_update_count == 0,
                            "pause prevents fixed updates");

    app_handle_event(&app, APP_EVENT_TOGGLE_PAUSE);
    app_step(&app, 1.0);
    app_step(&app, 1.02);
    failures |= expect_true(app.fixed_update_count == 1,
                            "fixed updates use the configured step");

    app_handle_event(&app, APP_EVENT_DEACTIVATED);
    failures |= expect_true(app_is_paused(&app),
                            "deactivation pauses simulation");
    app_handle_event(&app, APP_EVENT_ACTIVATED);
    failures |= expect_true(!app_is_paused(&app),
                            "activation resumes simulation");

    app_handle_event(&app, APP_EVENT_RESTART);
    failures |= expect_true(app.fixed_update_count == 0 &&
                                app.accumulator_seconds == 0.0 &&
                                app.frame_index == 0,
                            "restart resets application-owned state");

    app_set_player_spawn(&app, spawn);
    app_set_level_walkable(&app, foundation_walkable);
    failures |= expect_true(
        app.player.position[0] == -2.5f &&
            app.player.position[2] == -2.5f &&
            app.player.state == APP_PLAYER_IDLE,
        "movement initialization restores the spawn and idle state");

    app_set_key(&app, APP_KEY_MOVE_RIGHT, true);
    app_move_player_fixed(&app, APP_FIXED_STEP_SECONDS);
    failures |= expect_true(
        app.player.position[0] > -2.5f &&
            app.player.position[2] < -2.5f &&
            app.player.state == APP_PLAYER_MOVING &&
            app.player.facing == APP_FACING_NORTHEAST,
        "right input maps consistently to isometric world movement");
    app_set_key(&app, APP_KEY_MOVE_RIGHT, false);
    app_move_player_fixed(&app, APP_FIXED_STEP_SECONDS);
    failures |= expect_true(app.player.state == APP_PLAYER_IDLE,
                            "released input restores idle state");

    app_set_key(&app, APP_KEY_MOVE_UP, true);
    app_move_player_fixed(&app, APP_FIXED_STEP_SECONDS);
    failures |= expect_true(
        app.player.position[0] < -2.0f &&
            app.player.position[2] < -2.0f &&
            app.player.facing == APP_FACING_NORTHWEST,
        "up input maps to the north-west isometric world direction");
    app_set_key(&app, APP_KEY_MOVE_UP, false);

    app_set_key(&app, APP_KEY_MOVE_DOWN, true);
    app_move_player_fixed(&app, APP_FIXED_STEP_SECONDS);
    failures |= expect_true(
        app.player.position[0] > -2.55f &&
            app.player.position[2] > -2.55f &&
            app.player.facing == APP_FACING_SOUTHEAST,
        "down input maps to the south-east isometric world direction");
    app_set_key(&app, APP_KEY_MOVE_DOWN, false);

    app_set_key(&app, APP_KEY_MOVE_LEFT, true);
    app_move_player_fixed(&app, APP_FIXED_STEP_SECONDS);
    failures |= expect_true(
        app.player.state == APP_PLAYER_MOVING &&
            app.player.facing == APP_FACING_SOUTHWEST,
        "left input maps to the south-west isometric world direction");
    app_set_key(&app, APP_KEY_MOVE_LEFT, false);

    failures |= expect_true(
        app_facing_from_world_direction(0.0f, -1.0f) == APP_FACING_NORTH &&
            app_facing_from_world_direction(1.0f, -1.0f) == APP_FACING_NORTHEAST &&
            app_facing_from_world_direction(1.0f, 0.0f) == APP_FACING_EAST &&
            app_facing_from_world_direction(1.0f, 1.0f) == APP_FACING_SOUTHEAST &&
            app_facing_from_world_direction(0.0f, 1.0f) == APP_FACING_SOUTH &&
            app_facing_from_world_direction(-1.0f, 1.0f) == APP_FACING_SOUTHWEST &&
            app_facing_from_world_direction(-1.0f, 0.0f) == APP_FACING_WEST &&
            app_facing_from_world_direction(-1.0f, -1.0f) == APP_FACING_NORTHWEST,
        "world directions map to all eight stable facing directions");

    candidate[0] = 4.0f;
    candidate[1] = -0.7f;
    candidate[2] = 0.0f;
    failures |= expect_true(
        !app_resolve_player_collision(app.player.position,
                                      candidate,
                                      app.player.collision_radius,
                                      foundation_walkable),
        "player collision rejects level boundaries");

    candidate[0] = 0.75f;
    candidate[1] = -0.7f;
    candidate[2] = 0.0f;
    failures |= expect_true(
        !app_resolve_player_collision(app.player.position,
                                      candidate,
                                      app.player.collision_radius,
                                      foundation_walkable),
        "player collision rejects wall and obstacle extents");

    app_handle_event(&app, APP_EVENT_CLOSE);
    failures |= expect_true(!app.running &&
                                app.close_requested,
                            "close event stops the application");

    if (failures != 0)
    {
        return 1;
    }

    puts("Game foundation tests passed.");
    return 0;
}
