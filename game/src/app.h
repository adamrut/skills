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
} App;

AppResult app_init(App *app);
void app_shutdown(App *app);
AppResult app_run_platform(App *app);
void app_handle_event(App *app, AppEvent event);
void app_restart(App *app);
bool app_is_paused(const App *app);
bool app_step(App *app, double current_time_seconds);
const char *app_result_text(AppResult result);

#endif
