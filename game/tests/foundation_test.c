#include "../src/app.h"

#include <stdio.h>

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
