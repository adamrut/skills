# Vulkan Isometric RPG Implementation Plan

This document is a future implementation sequence for a small playable Windows RPG. It contains planning guidance only; it does not implement source code or change build configuration.

## Repository and open decisions

- The current repository contains prompt and skill documentation; it does not contain a C, CMake, or Vulkan game implementation.
- Before implementation starts, create the game project in a location approved by repository maintainers and preserve repository conventions found at that time.
- Confirmed: C, CMake, Vulkan, Windows, isometric/top-down presentation, procedural placeholders, and one complete small playable level.
- Open: final level name and narrative context.
- Open: exact enemy types.
- Open: exact health, damage, movement, experience, and progression values.
- No consequential third-party library should be selected without explicit approval.

## 1. Prepare the development environment

- Confirm that the target is Windows and the development machine has a Vulkan-compatible GPU and current graphics driver.
- Install a C compiler supported by CMake, CMake, the Vulkan SDK, and the Windows build tools.
- Verify `vulkaninfo`, SDK shader tools, the compiler, CMake, and validation-layer availability.
- Create only the initial CMake project structure and one executable target.
- Add separate Debug and Release build configurations.
- Enable compiler warnings and treat blocking warnings as errors in the implementation phase.
- Add Vulkan SDK discovery through CMake.
- Print a minimal console message and define a clean executable startup and exit path.
- Build and run the executable on Windows.
- Stop if the toolchain or SDK cannot build and start an executable.

## 2. Create the application foundation

- Create `main`, initialize application-owned state, and enter a deterministic loop.
- Create and destroy the native window through the platform startup and window-integration boundary.
- Handle activation, close, resize, and shutdown messages.
- Track elapsed time with a monotonic performance timer.
- Maintain a fixed update step and a variable rendering step.
- Route required initialization, runtime, and shutdown failures through a consistent error path.
- Ensure repeated start, close, pause, and restart actions do not leak resources.
- Stop if the application cannot start or close cleanly.

## 3. Initialize Vulkan

- Create the Vulkan instance and enumerate instance extensions.
- Enable validation layers in Debug builds and report missing or unsupported features clearly.
- Enumerate physical devices and select the first suitable device by required queue and presentation support.
- Create a logical device and graphics/presentation queues.
- Create a Windows `VkSurfaceKHR` for the application window.
- Select a supported surface format and present mode.
- Stop if validation cannot be enabled reliably in Debug or a required device capability is unavailable.
- Add a `VkSurfaceKHR` recreation path for window or device changes.

## 4. Create the rendering infrastructure

- Create the swap chain and swap-chain image views.
- Create a render pass and depth-capable frame buffers.
- Create a command pool and allocate command buffers.
- Create image-acquisition, render, and presentation synchronization objects.
- Implement frame submission and safe in-flight frame limits.
- Recreate swap-chain resources after window resize or presentation failure.
- Render a clear color using Vulkan synchronization.
- Stop if the clear color flickers, hangs, or produces validation errors.

## 5. Add a minimal 3D pipeline

- Define documented vertex and uniform data structures.
- Add a minimal vertex shader and fragment shader.
- Compile shaders into required Vulkan shader modules or establish a reproducible build step for them.
- Create pipeline layout, descriptor, and graphics pipeline objects.
- Add a depth buffer and a perspective or orthographic projection matrix.
- Create a fixed isometric view matrix.
- Create model transform and color inputs.
- Render one cube or plane visible from the fixed camera.
- Stop if the shape is not visible from the isometric camera or depth behavior is unstable.

## 6. Create procedural mesh support

- Add reusable cube and plane mesh definitions.
- Generate vertices and indices in source code rather than external model assets.
- Create reusable vertex and index buffer upload functions.
- Add mesh draw commands and per-object transforms.
- Add per-object color or material data.
- Render several cubes and planes in a scene.
- Verify transform order, world units, and depth order in both Debug and Release builds.

## 7. Add the isometric camera

- Define one fixed isometric/top-down camera for the complete level.
- Position the camera above and diagonally away from the playable area.
- Point the camera at the player or level center.
- Add bounded smooth follow behavior only after direct fixed-follow behavior works.
- Prevent the camera view from moving beyond fixed level boundaries.
- Test each screen edge, supported window size, and resize recreation.
- Stop if the level is not readable or the camera loses depth correctness.

## 8. Build the first level

- Choose a fixed small rectangular level layout after repository setup.
- Represent the level in a small C data structure or configuration data.
- Generate the ground from procedural tiles.
- Add procedural walls and collision boundaries.
- Add decorative obstacles made from cubes or planes.
- Define one consistent world-unit scale and a player spawn position.
- Verify walkable areas and boundaries with debug overlays or assertions.
- Stop when the level is readable and its collision extents are stable.

## 9. Add input and player movement

- Add keyboard input handling at the Windows/message or application-input boundary.
- Map directional input to isometric world axes.
- Add direct movement first, then acceleration only if it improves control.
- Restrict movement to level boundaries.
- Detect and resolve collisions with walls and obstacles.
- Track idle and movement states.
- Add facing or four-directional rotation.
- Render the player from procedural simple shapes.
- Test movement in all directions, against every boundary, and in narrow corridors.

## 10. Add combat primitives

- Add a melee attack input and attack state.
- Add a configurable attack cooldown and range.
- Define targets in terms of circular or axis-aligned collision data.
- Apply damage once to each valid target during an attack.
- Add hit and brief damage states without advanced animation.
- Expose and update health for player and enemy objects.
- Test against stationary targets, out-of-range targets, and missed attacks.

## 11. Add enemies and AI

- Add enemy identity, position, health, and state data.
- Spawn configured procedural enemies in the level.
- Render enemies from procedural shapes.
- Detect the player within a configurable awareness range.
- Move aware enemies toward the player while avoiding walls.
- Stop movement outside awareness range or after a defeat state.
- Add a basic enemy melee attack with range and cooldown.
- Test collision, pursuit, retreat, defeat, and removal behavior.

## 12. Add the game state

- Implement menu, playing, paused, won, lost, and optionally confirmation states.
- Stop simulation in paused and menu states.
- Track player health and defeat transitions.
- Add game-over, level-complete, restart, and resume actions.
- Reset or preserve state explicitly for each transition.
- Prevent stale timers or command buffers from changing state after restart.
- Test all state transitions and repeated restart cycles.

## 13. Add stats and progression

- Define health, damage, defense, movement, experience, and level fields.
- Keep progression constants or data configurable in C.
- Define a small, explicit damage formula.
- Award experience only for valid enemy defeats.
- Use a simple level-up threshold.
- Increase a limited set of player attributes after level-up.
- Test damage, healing or respawn, experience, and level-up boundaries.

## 14. Add minimal inventory and equipment

- Define a small item structure and bounded inventory list.
- Add a collection action for placed procedural items.
- Show the item list through code and later through the HUD.
- Add equip and unequip actions and one or more equipment slots.
- Apply equipment modifiers only through the stats/damage ownership point.
- Report a full inventory or unsupported slot clearly.
- Test collection, equip, unequip, inventory limit, and item-reset behavior.

## 15. Add quests and objectives

- Define a small quest structure with objective type, target, and progress.
- Add a required objective to defeat a configured number of enemies.
- Add an optional objective only if it does not delay the core playable build.
- Update objective progress only from gameplay events.
- Display objective completion and progress in the HUD.
- Mark the level complete only after the main objective succeeds.
- Test objective progress, completion, defeat, and restart behavior.

## 16. Create the heads-up display

- Render health, level, experience, active objective, and interaction prompts.
- Use Vulkan 2D drawing or a deliberately small text rendering approach.
- Keep the display readable at supported resolutions and avoid external art assets.
- Update HUD values only from owned gameplay state.
- Test at the minimum and maximum supported resolutions and on window resize.
- Keep HUD failures from preventing gameplay where practical.

## 17. Add menus and user feedback

- Create main menu, pause menu, game-over, and level-complete screens.
- Add restart and exit actions for the appropriate screens.
- Confirm each unavailable Vulkan device or feature with actionable validation messages.
- Add minimal pause/resume feedback.
- Add minimal audio only after the core loop is stable, and only if complexity remains low.
- Test keyboard navigation or direct input controls and every menu transition.

## 18. Improve stability and performance

- Profile frame time on the target Windows system.
- Separate CPU and GPU bottlenecks and correct measured issues first.
- Use simple batching for meshes with compatible state and data.
- Reuse CPU staging and GPU buffers, descriptor sets, and command resources where practical.
- Exercise window resize and device-loss recovery repeatedly.
- Remove blocking Debug validation errors and new compiler warnings.
- Stop if device recovery or frame stability remains unreliable.

## 19. Package the game

- Add a CMake install target for the executable and required Vulkan/runtime files.
- Copy only runtime configuration or procedural resources required for execution.
- Produce Debug and Release Windows builds.
- Run the executable outside the development shell and working-directory assumptions.
- Document prerequisites, configure/build commands, run command, and clean rebuild steps.
- Test an installed release build on a clean Windows development environment.

## 20. Complete a release checklist

- Start from a clean Windows environment.
- Complete the level, defeat enemies, and satisfy the main objective.
- Verify pause, resume, restart, and game-over behavior.
- Verify menu, equipment, inventory, and quest state transitions.
- Verify window resize and Vulkan device recovery.
- Verify procedural ground, player, enemies, obstacles, and HUD rendering.
- Complete a release build and an installed executable test.
- Fix blocking errors before adding any non-required feature.

## Deferred choices

- Final level name and narrative context require repository-owner or user input.
- Enemy type selection is deferred until movement and melee combat are stable.
- Exact enemy and player stats are deferred until combat balance is measurable.
- Optional quest and equipment choices should remain outside the core sequence until the required features pass tests.
- Third-party libraries are excluded unless a future implementation explicitly demonstrates substantial complexity reduction and receives approval.

