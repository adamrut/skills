# Implement Vulkan Isometric RPG Step 8

## Goal

Build the first fixed level for the Windows C and Vulkan game.

Implement only Step 8 in `docs/implementation/vulkan-isometric-rpg-plan.md`.
Stop before Step 9 input and player movement.

## Confirmed requirements

- Implement Step 8 after the existing application, Vulkan rendering, procedural mesh scene, and fixed isometric camera.
- Keep C, CMake, Windows, Vulkan, and the existing project structure.
- Preserve the startup, shutdown, fixed/variable frame loop, fixed camera, resize recreation, submission, presentation, depth, and error paths.
- Choose one fixed, small, rectangular level layout.
- Represent the level in a small C data structure or configuration data.
- Generate the ground from procedural tiles.
- Add procedural walls and collision boundaries.
- Add decorative obstacles made from the existing procedural cubes or planes.
- Define one consistent world-unit scale.
- Define one player spawn position.
- Verify walkable areas and boundaries with debug overlays or assertions.
- Stop when the level is readable and its collision extents are stable.
- Do not add third-party libraries or external level assets.
- Do not choose a final level name or narrative context.

## Scope and exclusions

Create or change only files required for the Step 8 level and directly related tests and documentation.

Exclude:

- Keyboard input and player movement.
- Player and enemy state, health, combat, and AI.
- Menu, pause, inventory, quests, HUD, and progression.
- Audio and external art or model assets.
- Changes outside `game/` and directly related documentation.

## Implementation requirements

- Inspect the existing mesh, camera, scene-object, matrix, and test conventions before implementation.
- Replace or extend the Step 7 procedural preview scene with one coherent level scene.
- Use a bounded rectangular tile grid or equivalent small C configuration.
- Keep the level size small enough to remain readable from the existing fixed isometric camera.
- Define the tile size, ground height, wall height, obstacle height, and bounds from one documented world-unit scale.
- Use the existing cube and plane mesh types to generate ground tiles, walls, and obstacles.
- Add an explicit walkability representation for every ground tile.
- Mark rectangular boundary and wall collision extents.
- Store one consistent player spawn location that lies on a walkable tile.
- Use the existing fixed camera target or update it only to the actual level center.
- Use the existing level-bounds types or add closely related level data that can be reused by later steps.
- Provide environment-independent helpers for level coordinate conversion, tile validation, collision queries, spawn validation, and boundary checks where practical.
- Avoid dynamic allocation unless the existing project patterns require it.
- Keep collision extents separate from decorative geometry.
- Add assertions or debug reporting for invalid tile indices, spawn positions, enclosed obstacles, and inconsistent bounds.
- Do not implement enemy identity or enemy placement.
- Update the implementation map only for files and behavior directly added by Step 8.

## Acceptance criteria

- The executable displays one fixed, small, rectangular level.
- The ground is composed of procedural tiles.
- Walls and boundaries form stable collision extents.
- Decorative obstacles are visibly distinct and use procedural meshes.
- The level contains a documented walkable spawn position.
- The level scene remains readable with the fixed isometric camera.
- Environment-independent tests verify level dimensions, walkable tiles, collision boundaries, and spawn validity.
- The level has no walkability or collision inconsistency that can block later player movement.
- Debug and Release builds contain no blocking validation errors or compiler warnings.
- The implementation adds no work from plan Steps 9 through 20.

## Open questions or investigation

- Choose a fixed small rectangular layout without adding narrative or a final level name.
- Verify the current Step 7 preview bounds and the existing mesh dimensions before selecting a tile count and world scale.
- Inspect existing test conventions and add environment-independent level checks.
- Do not add enemy types or exact combat values.
- Ask before making any consequential compatibility change that cannot be resolved through repository inspection.

## Validation

- Inspect the repository and follow existing C, CMake, Vulkan, test, and documentation conventions.
- Configure and build the `game` target in Debug and Release modes.
- Run the project-standard CTest command.
- Run the executable on Windows.
- Verify the ground, walls, decorative obstacles, boundaries, and spawn area are visually readable.
- Verify each screen edge and supported window size.
- Verify resize recreation retains the level and camera behavior.
- Verify clean shutdown and no Vulkan validation errors.
