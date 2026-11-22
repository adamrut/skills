# Implementation Map

- [Vulkan isometric RPG plan](./vulkan-isometric-rpg-plan.md) is the implementation sequence for the Windows C and Vulkan game.
- [Initial implementation prompt](../prompts/0002-prompt-implement-vulkan-step-1.md) defines the completed Step 1 scope.
- [Step 2 implementation prompt](../prompts/0003-prompt-implement-vulkan-step-2.md) defines the native application foundation scope.
- The `game/` project owns the executable and its foundation tests.
  - [CMake configuration](../../game/CMakeLists.txt) defines the C project, discovers Vulkan, creates the `game` target, and enables warning-as-error Debug and Release builds.
  - [C entry point](../../game/src/main.c) owns executable startup, shutdown, and the consistent application error path.
  - [Application state](../../game/src/app.h) and [application loop logic](../../game/src/app.c) own pause, close, restart, fixed-update timing, variable-frame bookkeeping, and reset behavior.
  - [Platform boundary](../../game/src/platform_win32.c) owns the Win32 window, message handling, monotonic performance-counter timing, and native window cleanup.
  - [Foundation test](../../game/tests/foundation_test.c) verifies initialization, pause, fixed-step progress, activation, restart, and close state transitions.
  - [Build instructions](../../game/README.md) describe Debug and Release build commands.
- [Vulkan integration](../../game/src/vulkan.c) owns instance and validation setup, physical/logical device selection, graphics/presentation queues, the Win32 surface, surface format/present-mode selection, surface recreation, and Vulkan cleanup.
- [Vulkan interface](../../game/src/vulkan.h) exposes Vulkan initialization/shutdown/recreation and environment-independent format, present-mode, and queue-selection helpers.
- Debug builds enable `VK_LAYER_KHRONOS_validation` and `VK_EXT_debug_utils`; missing validation support is a hard initialization failure.
- The Win32 platform boundary creates the Vulkan state after native window creation and recreates the surface path on non-empty window resize; all Vulkan resources are destroyed before window cleanup.
- [Vulkan test](../../game/tests/vulkan_test.c) checks queue, surface-format, and present-mode selection without requiring a live Vulkan device.
