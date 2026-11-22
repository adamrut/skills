# Implement Vulkan Isometric RPG Step 5

## Goal

Implement the minimal 3D pipeline for the Windows C and Vulkan game.

Implement only Step 5 in `docs/implementation/vulkan-isometric-rpg-plan.md`.
Stop before the Step 6 mesh and scene work.

## Confirmed requirements

- Implement Step 5 after the existing application and Vulkan rendering foundation.
- Keep C, CMake, Windows, Vulkan, and the existing project structure.
- Keep the existing startup, shutdown, fixed/variable frame loop, resize, and error paths.
- Define documented vertex and uniform data structures.
- Add a minimal vertex shader and fragment shader.
- Compile shaders into Vulkan shader modules or establish a reproducible build step.
- Create the pipeline layout, descriptors, and graphics pipeline.
- Add a depth buffer and depth testing.
- Use a perspective projection.
- Use a fixed isometric view.
- Create model transform and color inputs.
- Render one visible cube.
- Do not add third-party libraries.
- Do not add reusable mesh support or multiple scene objects.

## Scope and exclusions

Create or change only files required for the Step 5 pipeline and directly related tests and documentation.

Exclude:

- Reusable cube and plane mesh support.
- Multiple meshes or scene objects.
- A first-level layout.
- Camera following or movement.
- Player, enemies, combat, gameplay states, or HUD.
- Third-party libraries.
- Changes outside `game/` and directly related documentation.

## Confirmed implementation choices

- Render one procedural cube.
- Use a perspective projection.
- Use a conventional fixed isometric camera view.
- Do not select an exact alternate shape, projection, or camera model without implementing-agent evidence and approval.

## Open questions or investigation

- Inspect existing project conventions and select the smallest reproducible shader build approach.
- Inspect existing CMake conventions before adding shader compilation or copying generated modules.
- Determine conventional perspective field of view and fixed isometric camera parameters from the repository and existing Vulkan conventions.
- Do not invent a fallback for unavailable Vulkan features.
- Inspect existing tests and add environment-independent checks where practical.

## Acceptance criteria

- The implementation creates and uses shader modules or a reproducible shader build step.
- The implementation creates a pipeline layout, descriptors, and graphics pipeline.
- The implementation renders one cube from a fixed, conventional isometric camera.
- The implementation uses a perspective projection matrix.
- Depth testing produces stable depth behavior.
- The shape is visible in both Debug and Release builds.
- Debug validation reports no blocking errors.
- The implementation preserves clean startup, resize recreation, submission, presentation, and shutdown behavior.
- The implementation adds no work from plan Steps 6 through 20.

## Validation

- Configure and build the `game` target in Debug and Release modes.
- Run the project-standard test command.
- Run the executable on Windows.
- Verify the visible shape and stable depth behavior.
- Resize the window and verify pipeline recreation remains stable.
- Verify clean shutdown and no Vulkan validation errors.
