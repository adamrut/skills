# Implement Vulkan Isometric RPG Step 9

## Goal

Add keyboard input and player movement to the Windows C and Vulkan game.

Implement only Step 9 in `docs/implementation/vulkan-isometric-rpg-plan.md`.
Stop before Step 10 combat primitives.

## Confirmed requirements

- Implement Step 9 after the existing application, Vulkan rendering, procedural mesh scene, fixed isometric camera, and fixed first level.
- Keep C, CMake, Windows, Vulkan, and the existing project structure.
- Preserve the startup, shutdown, fixed/variable frame loop, fixed camera, level scene, resize recreation, submission, presentation, depth, and error paths.
- Handle keyboard input at the Win32 message or application-input boundary.
- Support both W/A/S/D and the four arrow keys.
- Map screen-style directional input to isometric world axes.
- Use direct speed-based movement.
- Do not add acceleration unless a later implementation explicitly requires it.
- Restrict movement to the level boundaries.
- Detect and resolve collisions with walls and obstacles.
- Track idle and movement states.
- Support eight facing directions, including diagonal directions.
- Render a simple procedural humanoid-like player from existing procedural mesh types.
- Do not add external models or art assets.
- Test movement in all directions, at every boundary, and in narrow corridors.

## Scope and exclusions

Create or change only files required for the Step 9 input and movement behavior and directly related tests and documentation.

Exclude:

- Melee attacks, attack states, cooldowns, health, damage, enemies, or AI.
- Game menus, pause logic, inventory, quests, HUD, and progression.
- Audio and external art or model assets.
- Changes outside `game/` and directly related documentation.

## Implementation requirements

- Inspect the existing application loop, Win32 message handling, level data, collision extents, camera mapping, mesh objects, and test conventions.
- Route held-key state through the application input boundary.
- Normalize combined horizontal and vertical input.
- Use the fixed update step for movement.
- Convert directional input into world-space movement that is readable with the isometric camera.
- Keep player movement and collision logic in environment-independent application or level helpers where practical.
- Use a small, documented, configurable player movement speed.
- Use a simple collision shape consistent with the level tile and world-unit scale.
- Prevent the player from entering or passing through walls, obstacles, or level boundaries.
- Resolve collision axis separately when practical.
- Preserve the player position outside collision state transitions.
- Track a movement state when input produces movement.
- Track an idle state when input produces no movement.
- Map eight direction combinations to a stable facing direction.
- Update the facing state only from movement input.
- Render the procedural humanoid-like player with distinct procedural meshes.
- Keep the player renderable through the existing scene-object and mesh infrastructure.
- Reuse or extend the existing level data instead of duplicating collision information.
- Add environment-independent tests for direction mapping, facing, boundary checks, collision resolution, idle/movement state, and narrow-corridor behavior.
- Update the implementation map only for files and behavior directly added by Step 9.

## Acceptance criteria

- The executable accepts WASD and arrow-key movement without interfering with window close or restart behavior.
- Each supported key and each combination of diagonal keys changes player movement in a consistent world direction.
- The player cannot leave the level boundary.
- The player cannot enter wall or obstacle collision extents.
- Collision resolution prevents tunneling in the fixed update loop.
- The player changes between idle and movement states correctly.
- The player has a stable facing direction for eight movement directions.
- The player is visibly distinct and uses procedural mesh shapes.
- Movement remains stable in Debug and Release builds.
- Environment-independent tests cover all directions, level boundaries, walls, obstacles, and narrow corridors.
- Debug and Release builds contain no blocking Vulkan validation errors or compiler warnings.
- The implementation adds no work from plan Steps 10 through 20.

## Open questions or investigation

- Choose the exact movement speed and collision shape from the existing level scale and control tests.
- Inspect the existing Win32 input and application event patterns before adding held-key state.
- Inspect the existing fixed-level spawn and collision extents before defining the player collision shape.
- Determine the simple procedural humanoid-like shape proportions from existing mesh dimensions.
- Keep movement constants configurable.
- Ask before making a consequential compatibility or control change that cannot be resolved through repository investigation.

## Validation

- Inspect the repository and follow existing C, CMake, Windows, Vulkan, test, and documentation conventions.
- Configure and build the `game` target in Debug and Release modes.
- Run the project-standard CTest command.
- Run the executable on Windows.
- Verify WASD and arrow-key movement in all eight directions.
- Verify movement against each level boundary.
- Verify movement against every wall and obstacle.
- Verify movement through narrow corridors.
- Verify idle, movement, and facing behavior.
- Verify resize recreation, clean shutdown, and no Vulkan validation errors.
