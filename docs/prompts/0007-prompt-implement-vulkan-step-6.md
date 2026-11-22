# Implement Vulkan Isometric RPG Step 6

## Goal

Implement procedural mesh support for the Windows C and Vulkan game.

Implement only Step 6 in `docs/implementation/vulkan-isometric-rpg-plan.md`.
Stop before the Step 7 isometric camera work.

## Confirmed requirements

- Implement Step 6 after the existing application, Vulkan rendering, and Step 5 single-cube pipeline.
- Keep C, CMake, Windows, Vulkan, and the existing project structure.
- Preserve the existing startup, shutdown, fixed/variable frame loop, resize recreation, submission, presentation, and error paths.
- Add reusable cube and plane mesh definitions.
- Generate cube and plane vertices and indices in source code.
- Do not load external model assets.
- Add reusable vertex and index buffer upload functions.
- Add mesh draw commands.
- Add per-object transforms.
- Add per-object color or material data.
- Render several cubes and planes in one scene.
- Verify transform order, world units, and depth order.
- Verify behavior in Debug and Release builds.
- Do not add third-party libraries.

## Scope and exclusions

Create or change only files required for procedural mesh support and directly related tests and documentation.

Exclude:

- A new fixed or follow camera.
- Level layout, player, enemy, or collision data.
- Gameplay input or state.
- External model or texture assets.
- Audio, HUD, inventory, quests, combat, and progression.
- Changes outside `game/` and directly related documentation.

## Implementation requirements

- Extend the existing Step 5 pipeline with reusable mesh data.
- Keep the existing perspective fixed-isometric view until Step 7 changes the camera.
- Define vertex and index data for one cube and one plane in source code.
- Include normal or winding information required for stable depth rendering.
- Upload mesh vertices and indices through shared Vulkan buffer functions.
- Track vertex and index buffers without leaking resources during shutdown or swap-chain recreation.
- Add a scene object data structure that identifies a mesh and contains transform and color inputs.
- Apply transform matrices in a consistent and documented order.
- Reuse command recording for all scene objects where practical.
- Draw cubes and planes with the existing pipeline unless existing conventions require a small compatible extension.
- Place several objects where the fixed camera can render them clearly.
- Preserve stable depth behavior for intersecting, separated, and ground-plane objects.
- Add environment-independent tests for mesh counts, indices, transforms, and scene-object data where practical.

## Acceptance criteria

- The executable creates reusable procedural cube and plane mesh definitions.
- The executable creates index and vertex buffers through reusable upload functions.
- The executable renders several distinct cubes and planes in a scene.
- Each scene object has an independent transform and color or material value.
- Transform order is consistent and documented.
- Depth order remains stable in the fixed isometric view.
- Debug and Release builds contain no blocking validation errors or compiler warnings.
- Window resize and Vulkan swap-chain recreation retain and render the procedural scene.
- Mesh resources are released on shutdown.
- The implementation adds no work from plan Steps 7 through 20.

## Open questions or investigation

- Inspect the existing implementation before changing Vulkan resource ownership.
- Determine the smallest scene size and object layout that clearly verifies cube and plane rendering.
- Inspect existing tests and extend them with environment-independent mesh and transform checks.
- If a mesh or transform choice conflicts with the existing Step 5 pipeline, ask before making a consequential compatibility change.
- Do not select third-party libraries or external asset formats.

## Validation

- Inspect the repository and follow existing C, CMake, Vulkan, test, and documentation conventions.
- Configure and build the `game` target in Debug and Release modes.
- Run the project-standard CTest command.
- Run the executable on Windows.
- Verify that multiple cubes and planes are visible.
- Verify that the object colors differ where used.
- Verify transform order and world-unit scale through visual inspection and environment-independent tests.
- Resize the window and verify mesh rendering and swap-chain recreation.
- Verify clean shutdown and no Vulkan validation errors.
- Update the implementation map only for files and behavior directly added by Step 6.
