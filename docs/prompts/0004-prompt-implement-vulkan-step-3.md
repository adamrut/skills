# Implement Vulkan Isometric RPG Step 3

## Goal

Initialize Vulkan for the Windows C and Vulkan game.

Implement only Step 3 in `docs/implementation/vulkan-isometric-rpg-plan.md`.
Stop before swap-chain and later rendering work.

## Confirmed requirements

- Keep the existing CMake project in `game/`.
- Keep C, CMake, Windows, and Vulkan.
- Create the Vulkan instance.
- Enumerate instance extensions.
- Enable Vulkan validation layers in Debug builds.
- Report missing or unsupported validation features clearly.
- Enumerate physical devices.
- Select the first suitable physical device that supports the required queue operations and presentation.
- Create a logical device.
- Create graphics and presentation queues.
- Create a `VkSurfaceKHR` for the native Windows application window.
- Select a supported surface format.
- Select a supported present mode.
- Stop if validation cannot be enabled reliably in Debug builds.
- Stop if a required device capability is unavailable.
- Add a `VkSurfaceKHR` recreation path for window or device changes.
- Do not add third-party libraries.

## Acceptance criteria

- The Debug build creates a Vulkan instance with validation enabled when validation is available and supported.
- The Debug build reports a clear, actionable error when required validation support is unavailable.
- The instance creation enumerates required and available instance extensions.
- The implementation enumerates physical devices and selects a suitable device.
- The selected device supports graphics and presentation queues.
- The implementation creates a logical device and obtains graphics and presentation queue handles.
- The implementation creates a Vulkan surface for the existing Win32 window.
- The implementation selects a surface format and present mode that the surface supports.
- Window or device changes can enter a supported surface recreation path.
- Initialization and shutdown failures use the existing consistent error path.
- Vulkan resources are destroyed cleanly before application exit.
- The executable builds and passes project-standard tests in Debug and Release configurations.
- No work from plan Steps 4 through 20 is added.

## Scope and exclusions

Create or change only files required for:

- Vulkan instance creation and extension enumeration.
- Debug validation-layer setup and diagnostics.
- Physical-device enumeration and suitability selection.
- Logical-device and queue creation.
- Win32 Vulkan surface creation.
- Surface format and present-mode selection.
- Surface recreation state and cleanup.
- Directly related tests and documentation.

Exclude:

- Swap-chain creation.
- Image views, render passes, frame buffers, or depth buffers.
- Command pools or command buffers.
- Frame synchronization and submission.
- Shaders, pipelines, meshes, cameras, or rendered output.
- Player movement, enemies, combat, inventory, quests, HUD, menus, or audio.
- Third-party libraries.
- Changes outside `game/` and directly related documentation.

## Constraints and compatibility

- Inspect the current `game/` project and follow its established C, CMake, platform-boundary, error, cleanup, and test patterns.
- Keep Win32-specific Vulkan surface code in the platform or integration boundary.
- Keep Vulkan device and instance ownership clear.
- Use the existing application startup, shutdown, and error paths.
- Enable validation by build mode rather than by a fixed runtime setting.
- Report missing instance extensions, validation layers, suitable devices, queue support, or presentation support explicitly.
- If a Vulkan SDK, device, surface, or required capability is unavailable, stop and report the exact blocker.
- Ask before making a remaining consequential choice that repository evidence cannot resolve.

## Open questions or investigation

- Inspect the installed Vulkan SDK and current Debug/Release build configuration.
- Determine the current repository conventions for testable Vulkan state and environment-independent checks.
- Determine whether the development machine supports all required Vulkan and Win32 surface capabilities.
- Add testable checks for device selection, queue selection, format selection, present-mode selection, and recreation handling where a real Vulkan device is unavailable.
- Do not invent fallback behavior for unavailable required device capabilities.

## Validation

- Configure and build the `game` target in Debug and Release modes.
- Run the project-standard test command.
- Run the executable on Windows.
- Verify that the Vulkan instance creates successfully.
- Verify that validation is enabled in Debug builds when supported.
- Verify that device and queue selection succeed or fail with an actionable message.
- Verify that the Win32 surface creates successfully.
- Verify that a supported surface format and present mode are selected.
- Trigger the existing window recreation path and verify that the surface can be recreated safely.
- Verify clean shutdown and no Vulkan resource leaks.
- Verify that no swap-chain, pipeline, mesh, or gameplay code was added.
