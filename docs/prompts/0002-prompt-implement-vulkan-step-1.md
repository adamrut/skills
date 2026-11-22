# Implement Vulkan Isometric RPG Step 1

## Goal

Prepare the development environment and create the initial runnable C, CMake, and Vulkan project for the Windows game.

Implement only Step 1 in `docs/implementation/vulkan-isometric-rpg-plan.md`.
Stop before application foundation, Vulkan initialization, or later gameplay work.

## Confirmed requirements

- Create the project in the `game/` subdirectory.
- Use C as the programming language.
- Use CMake as the build system.
- Target Windows.
- Create one executable target named `game`.
- Add separate Debug and Release build configurations.
- Enable compiler warnings.
- Treat blocking warnings as errors during implementation.
- Add Vulkan SDK discovery through CMake.
- Create an executable that prints a minimal console message.
- Add a clean startup and exit path.
- Build and run the executable on Windows.
- Stop if the toolchain or Vulkan SDK cannot build and start an executable.
- Do not add third-party libraries.

## Acceptance criteria

- `game/CMakeLists.txt` defines the C project and the `game` executable target.
- A C source file provides a minimal `main` function and prints a console message.
- CMake discovers a usable Vulkan SDK installation.
- Debug and Release build configurations are available.
- The executable starts, prints the message, and exits successfully.
- The implementation does not include work from later plan steps.

## Scope and exclusions

Create only:

- The initial CMake project structure in `game/`.
- The CMake executable target.
- A minimal C source file.
- Debug and Release build configuration support.
- Vulkan SDK discovery.
- A minimal console startup and exit.
- Build and run instructions required to verify Step 1.

Exclude:

- Native window creation.
- Vulkan instances, devices, surfaces, swap chains, or rendering.
- Game input, state, gameplay, enemies, HUD, assets, or menus.
- Third-party libraries.
- Changes outside `game/` and directly related documentation.

## Constraints and compatibility

- Use the Windows build target.
- Use an installed C compiler supported by CMake.
- Use a supported CMake version.
- Use the current Vulkan SDK.
- Use validation-layer availability where the installed SDK provides it.
- Preserve repository conventions found during investigation.
- Keep platform-specific code out of the initial source unless CMake or a Windows toolchain requires it.

## Open questions or investigation

- Inspect the machine for an installed C compiler, CMake, Vulkan SDK, graphics driver, and Vulkan validation layers.
- Detect and use an installed supported Windows toolchain.
- Do not require Visual Studio/MSVC, Clang, or MinGW-w64 GCC if another installed supported toolchain works.
- Stop and report a specific blocker if `vulkaninfo`, Vulkan SDK shader tools, CMake, the compiler, or a compatible GPU and driver is unavailable.

## Validation

- Verify that CMake configures the `game` project in Debug and Release modes.
- Build the `game` executable in Debug and Release modes.
- Run the executable and verify the minimal console message.
- Run a Vulkan tool check if the SDK provides `vulkaninfo`.
- Verify that no files outside `game/` and directly related build instructions changed.
- Verify that no later implementation step was started.
