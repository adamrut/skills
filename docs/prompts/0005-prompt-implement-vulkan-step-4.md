# Implement Vulkan Isometric RPG Step 4

## Goal

Create the Vulkan rendering infrastructure for the Windows C and Vulkan game.

Implement only Step 4 in `docs/implementation/vulkan-isometric-rpg-plan.md`.
Stop before the Step 5 3D pipeline, shaders, meshes, and camera work.

## Confirmed requirements

- Keep the existing CMake project in `game/`.
- Keep C, CMake, Windows, and Vulkan.
- Use the existing application foundation and Vulkan initialization from Steps 1 through 3.
- Create the swap chain and swap-chain image views.
- Create a render pass.
- Add a depth-capable frame buffer for each swap-chain image.
- Create a command pool and allocate command buffers.
- Create image-acquisition, render, and presentation synchronization objects.
- Implement frame submission.
- Use a safe in-flight frame limit.
- Recreate swap-chain resources after window resize.
- Recreate swap-chain resources after presentation failure.
- Render a clear color through the Vulkan synchronization path.
- Stop if the clear color flickers, hangs, or produces validation errors.
- Preserve the existing clean startup, shutdown, and error paths.
- Do not add third-party libraries.

## Acceptance criteria

- The Debug build creates a swap chain that is compatible with the selected surface format and present mode.
- The swap chain creates one image view for every swap-chain image.
- The render pass is compatible with the selected surface format and with a depth attachment.
- One depth buffer and one depth-capable frame buffer are available for each swap-chain image.
- The command pool supports the graphics queue.
- The implementation allocates the command buffers needed for the in-flight frame limit.
- Image acquisition, command submission, and presentation are synchronized correctly.
- The in-flight frame limit does not exceed the available swap-chain resources.
- The implementation submits frames without resource reuse before synchronization.
- A presentation failure or window resize recreates the required swap-chain resources safely.
- The window can resize repeatedly without stale resources or validation errors.
- The application renders a stable clear color in Debug and Release builds.
- Validation layers report no blocking errors during initialization, rendering, resize, presentation failure, and shutdown.
- Initialization, recreate, present, and shutdown failures use the existing actionable error path.
- Vulkan and native resources are destroyed cleanly before application exit.
- The executable builds and passes project-standard tests in Debug and Release configurations.
- No work from plan Steps 5 through 20 is added.

## Scope and exclusions

Create or change only files required for:

- Swap-chain creation and cleanup.
- Swap-chain image views.
- Render-pass setup.
- Depth-image setup and depth buffer creation.
- Frame-buffer setup.
- Command-pool and command-buffer setup.
- Frame synchronization and in-flight frame state.
- Frame submission and presentation.
- Window resize and presentation-failure recreation.
- Clear-color rendering.
- Directly related tests and documentation.

Exclude:

- Vertex or uniform data structures.
- Vertex or fragment shaders.
- Shader modules or shader build steps.
- Pipeline layouts or graphics pipelines.
- Meshes, cubes, planes, or indexed draw calls.
- Projection or view matrices.
- Isometric cameras.
- Level geometry, player movement, combat, enemies, inventory, quests, HUD, menus, or audio.
- Third-party libraries.
- Changes outside `game/` and directly related documentation.

## Constraints and compatibility

- Inspect the current `game/` project and follow its established C, CMake, platform-boundary, Vulkan, error, cleanup, and test patterns.
- Keep the Win32 window integration and resize notification in the platform boundary.
- Keep Vulkan ownership and recreation state clear.
- Preserve any existing Step 3 surface recreation behavior.
- Reuse the existing application startup, fixed/variable frame loop, shutdown, and error paths.
- Use the existing Vulkan state and queue handles.
- Define resource ownership for each swap-chain image and each in-flight frame.
- Use the selected surface format, present mode, graphics queue, and presentation queue.
- Do not add an unsupported depth format without a clear capability check and error path.
- Do not add a graphics pipeline or draw call solely for the clear render.
- Report resize, swap-chain recreation, image-acquisition, submission, presentation, and depth-format errors explicitly.
- Stop if the current hardware or environment cannot validate rendering or presentation.
- Ask before making a remaining consequential choice that repository evidence cannot resolve.

## Open questions or investigation

- Inspect the current Vulkan state and resource layout in `game/src/vulkan.c` and `game/src/vulkan.h`.
- Determine the existing test conventions for environment-independent recreation and synchronization checks.
- Determine the supported depth format and presentation configuration on the development system.
- Determine whether the existing fixed/variable frame loop can provide a safe render hook without changing behavior from Steps 1 through 3.
- Add environment-independent tests where live Vulkan device behavior cannot be exercised.
- Do not invent a fallback when a required Vulkan or depth capability is unavailable.

## Validation

- Configure and build the `game` target in Debug and Release modes.
- Run the project-standard test command.
- Run the executable on Windows.
- Verify that the swap chain and every swap-chain image view are created.
- Verify that the render pass and depth-capable frame buffers are created successfully.
- Verify that command pools and command buffers are allocated on the graphics queue.
- Verify that acquisition, submission, and presentation synchronization prevents in-flight resource reuse.
- Verify a stable clear color appears in the window.
- Resize the window repeatedly and verify clean resource recreation.
- Exercise presentation-failure recreation if supported by the existing test or runtime path.
- Verify clean shutdown and no Vulkan resource leaks.
- Verify Debug validation reports no blocking errors.
- Verify that no pipeline, shader, mesh, camera, or gameplay code was added.
