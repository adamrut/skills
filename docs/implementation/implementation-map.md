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
