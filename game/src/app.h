#ifndef APP_H
#define APP_H

#include <stdbool.h>

#define APP_FIXED_STEP_SECONDS (1.0 / 60.0)
#define APP_MAX_FIXED_STEPS 5

typedef enum AppResult
{
    APP_RESULT_OK = 0,
    APP_RESULT_INITIALIZATION_FAILED,
    APP_RESULT_RUNTIME_FAILED,
    APP_RESULT_SHUTDOWN_FAILED
} AppResult;

typedef enum AppEvent
{
    APP_EVENT_NONE = 0,
    APP_EVENT_ACTIVATED,
    APP_EVENT_DEACTIVATED,
    APP_EVENT_RESIZED,
    APP_EVENT_TOGGLE_PAUSE,
    APP_EVENT_RESTART,
    APP_EVENT_CLOSE
} AppEvent;

typedef enum AppKey
{
    APP_KEY_MOVE_LEFT = 0,
    APP_KEY_MOVE_RIGHT,
    APP_KEY_MOVE_UP,
    APP_KEY_MOVE_DOWN,
    APP_KEY_COUNT
} AppKey;

typedef enum AppPlayerState
{
    APP_PLAYER_IDLE = 0,
    APP_PLAYER_MOVING
} AppPlayerState;

typedef enum AppFacingDirection
{
    APP_FACING_NORTH = 0,
    APP_FACING_NORTHEAST,
    APP_FACING_EAST,
    APP_FACING_SOUTHEAST,
    APP_FACING_SOUTH,
    APP_FACING_SOUTHWEST,
    APP_FACING_WEST,
    APP_FACING_NORTHWEST
} AppFacingDirection;

typedef bool (*AppWalkablePredicate)(const float world[3]);

typedef struct AppPlayer
{
    float position[3];
    float spawn[3];
    float movement_speed;
    float collision_radius;
    AppPlayerState state;
    AppFacingDirection facing;
} AppPlayer;

typedef struct App
{
    void *vulkan_state;
    bool running;
    bool close_requested;
    bool manually_paused;
    bool inactive_paused;
    bool restart_requested;
    double accumulator_seconds;
    double previous_time_seconds;
    unsigned long long frame_index;
    unsigned long long fixed_update_count;
    bool held_keys[APP_KEY_COUNT];
    AppWalkablePredicate level_walkable;
    AppPlayer player;
} App;

AppResult app_init(App *app);
void app_shutdown(App *app);
AppResult app_run_platform(App *app);
void app_handle_event(App *app, AppEvent event);
void app_restart(App *app);
bool app_is_paused(const App *app);
bool app_step(App *app, double current_time_seconds);
const char *app_result_text(AppResult result);
void app_set_key(App *app, AppKey key, bool held);
void app_set_player_spawn(App *app, const float spawn[3]);
void app_set_level_walkable(App *app, AppWalkablePredicate predicate);
void app_move_player_fixed(App *app, double fixed_step_seconds);
bool app_resolve_player_collision(const float current_position[3],
                                  float candidate_position[3],
                                  float collision_radius,
                                  AppWalkablePredicate predicate);
AppFacingDirection app_facing_from_world_direction(float x,
                                                   float z);

#endif
