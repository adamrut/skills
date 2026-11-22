#include "vulkan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        uint32_t graphics = VK_QUEUE_FAMILY_IGNORED;
        uint32_t present = VK_QUEUE_FAMILY_IGNORED;
        uint32_t f;

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

    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = queue_info_count;
    device_info.pQueueCreateInfos = queue_infos;
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
    if (create_surface(state) != APP_RESULT_OK)
    {
        return APP_RESULT_RUNTIME_FAILED;
    }

    if (select_device_and_queues(state) != APP_RESULT_OK)
    {
        return APP_RESULT_RUNTIME_FAILED;
    }
    if (select_surface_configuration(state) != APP_RESULT_OK)
    {
        return APP_RESULT_RUNTIME_FAILED;
    }
    if (create_logical_device(state) != APP_RESULT_OK)
    {
        return APP_RESULT_RUNTIME_FAILED;
    }
    return APP_RESULT_OK;
}

void vulkan_shutdown(VulkanState *state)
{
    if (state == NULL)
    {
        return;
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
