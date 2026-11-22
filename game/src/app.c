#include "app.h"

#include <stddef.h>

static void app_reset_runtime(App *app)
{
    app->accumulator_seconds = 0.0;
    app->previous_time_seconds = 0.0;
    app->frame_index = 0;
    app->fixed_update_count = 0;
    app->manually_paused = false;
    app->restart_requested = false;
}

AppResult app_init(App *app)
{
    if (app == NULL)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    app->running = true;
    app->close_requested = false;
    app->inactive_paused = false;
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
