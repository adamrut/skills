#include "app.h"
#include "platform.h"
#include "vulkan.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static App *platform_app_state;

static double platform_get_seconds(LARGE_INTEGER frequency,
                                   LARGE_INTEGER now)
{
    return (double)now.QuadPart / (double)frequency.QuadPart;
}

static LRESULT CALLBACK platform_window_proc(HWND window,
                                              UINT message,
                                              WPARAM w_param,
                                              LPARAM l_param)
{
    switch (message)
    {
    case WM_ACTIVATE:
        app_handle_event(platform_app_state,
                         LOWORD(w_param) == WA_INACTIVE
                             ? APP_EVENT_DEACTIVATED
                             : APP_EVENT_ACTIVATED);
        return 0;
    case WM_SIZE:
        app_handle_event(platform_app_state, APP_EVENT_RESIZED);
        if (platform_app_state != NULL &&
            LOWORD(l_param) != 0 &&
            HIWORD(l_param) != 0)
        {
            VulkanState *vulkan =
                (VulkanState *)platform_app_state->vulkan_state;
            if (vulkan != NULL)
            {
                if (vulkan_recreate_surface(vulkan) !=
                    APP_RESULT_OK)
                {
                    app_handle_event(platform_app_state, APP_EVENT_CLOSE);
                }
            }
        }
        return 0;
    case WM_CLOSE:
        app_handle_event(platform_app_state, APP_EVENT_CLOSE);
        return 0;
    case WM_DESTROY:
        app_handle_event(platform_app_state, APP_EVENT_CLOSE);
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        if (w_param == 'P')
        {
            app_handle_event(platform_app_state, APP_EVENT_TOGGLE_PAUSE);
        }
        else if (w_param == 'R')
        {
            app_handle_event(platform_app_state, APP_EVENT_RESTART);
        }
        else if (w_param == VK_ESCAPE)
        {
            app_handle_event(platform_app_state, APP_EVENT_CLOSE);
        }
        return 0;
    case WM_PAINT:
    {
        PAINTSTRUCT paint;
        BeginPaint(window, &paint);
        EndPaint(window, &paint);
        return 0;
    }
    default:
        return DefWindowProcW(window, message, w_param, l_param);
    }
}

static AppResult platform_destroy_window(HWND window)
{
    if (window != NULL)
    {
        if (DestroyWindow(window) == 0)
        {
            return APP_RESULT_SHUTDOWN_FAILED;
        }
    }
    return APP_RESULT_OK;
}

AppResult app_run_platform(App *app)
{
    if (app == NULL)
    {
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    platform_app_state = app;

    HINSTANCE instance = GetModuleHandleW(NULL);
    WNDCLASSEXW window_class;
    ZeroMemory(&window_class, sizeof(window_class));
    window_class.cbSize = sizeof(window_class);
    window_class.lpfnWndProc = platform_window_proc;
    window_class.hInstance = instance;
    window_class.hCursor =
        LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
    window_class.lpszClassName = L"VulkanIsometricRpgStep3";

    if (RegisterClassExW(&window_class) == 0)
    {
        DWORD error = GetLastError();
        if (error != ERROR_CLASS_ALREADY_EXISTS)
        {
            platform_app_state = NULL;
            return APP_RESULT_INITIALIZATION_FAILED;
        }
    }

    HWND window = CreateWindowExW(
        0,
        L"VulkanIsometricRpgStep3",
        L"Vulkan Isometric RPG",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1280,
        720,
        NULL,
        NULL,
        instance,
        NULL);
    if (window == NULL)
    {
        platform_app_state = NULL;
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    LARGE_INTEGER frequency;
    if (QueryPerformanceFrequency(&frequency) == 0 ||
        frequency.QuadPart == 0)
    {
        platform_destroy_window(window);
        UnregisterClassW(L"VulkanIsometricRpgStep3", instance);
        platform_app_state = NULL;
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    LARGE_INTEGER start_time;
    if (QueryPerformanceCounter(&start_time) == 0)
    {
        platform_destroy_window(window);
        UnregisterClassW(L"VulkanIsometricRpgStep3", instance);
        platform_app_state = NULL;
        return APP_RESULT_INITIALIZATION_FAILED;
    }

    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);

    VulkanState *vulkan_state = NULL;
    AppResult vulkan_result =
        vulkan_init(&vulkan_state, (void *)window);
    if (vulkan_result != APP_RESULT_OK)
    {
        platform_destroy_window(window);
        UnregisterClassW(L"VulkanIsometricRpgStep3", instance);
        platform_app_state = NULL;
        return vulkan_result;
    }
    app->vulkan_state = vulkan_state;

    AppResult result = APP_RESULT_OK;
    while (app->running)
    {
        MSG message;
        while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                app_handle_event(app, APP_EVENT_CLOSE);
                break;
            }
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        if (!app->running)
        {
            break;
        }

        LARGE_INTEGER now;
        if (QueryPerformanceCounter(&now) == 0)
        {
            result = APP_RESULT_RUNTIME_FAILED;
            app_handle_event(app, APP_EVENT_CLOSE);
            break;
        }

        app_step(app, platform_get_seconds(frequency, now) -
                          platform_get_seconds(frequency, start_time));

        Sleep(1);
    }

    VulkanState *active_vulkan =
        (VulkanState *)app->vulkan_state;
    if (active_vulkan != NULL)
    {
        vulkan_shutdown(active_vulkan);
        app->vulkan_state = NULL;
    }

    AppResult destroy_result = platform_destroy_window(window);
    if (destroy_result != APP_RESULT_OK)
    {
        result = destroy_result;
    }

    UnregisterClassW(L"VulkanIsometricRpgStep3", instance);
    platform_app_state = NULL;
    return result;
}
