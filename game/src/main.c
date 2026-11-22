#include "app.h"
#include "platform.h"

#include <stdio.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

int WINAPI wWinMain(HINSTANCE instance,
                    HINSTANCE previous_instance,
                    PWSTR command_line,
                    int show_command)
{
    (void)instance;
    (void)previous_instance;
    (void)command_line;
    (void)show_command;
#else
int main(void)
{
#endif
    App app;
    AppResult init_result = app_init(&app);
    if (init_result != APP_RESULT_OK)
    {
        fprintf(stderr, "Application initialization failed: %s\n",
                app_result_text(init_result));
        return 1;
    }

    AppResult run_result = app_run_platform(&app);
    app_shutdown(&app);

    if (run_result != APP_RESULT_OK)
    {
        fprintf(stderr, "Application shutdown failed: %s\n",
                app_result_text(run_result));
        return 1;
    }

    return 0;
}
