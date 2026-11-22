# Implementation Map

- [Vulkan isometric RPG plan](./vulkan-isometric-rpg-plan.md) is the implementation sequence for the Windows C and Vulkan game.
- [Initial implementation prompt](../prompts/0002-prompt-implement-vulkan-step-1.md) defines the completed Step 1 scope.
- The `game/` project owns the initial executable build.
  - [CMake configuration](../../game/CMakeLists.txt) defines the C project, discovers Vulkan, creates the `game` target, and enables warning-as-error builds.
  - [C entry point](../../game/src/main.c) owns executable startup and prints the Step 1 startup message.
  - [Build instructions](../../game/README.md) describe Debug and Release build commands.
