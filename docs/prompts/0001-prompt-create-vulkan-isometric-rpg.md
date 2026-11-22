# Create a Vulkan Isometric RPG

## Goal

Create a complete small isometric top-down RPG for Windows.

Use C, CMake, and the Vulkan API.
Create only the implementation plan.
Do not implement or change the game source code.
Use procedural placeholders for art and other assets.

Write the implementation plan to `docs/implementation/vulkan-isometric-rpg-plan.md`.
Do not write a second plan or a second implementation document.

## Confirmed requirements

- Use C as the programming language.
- Use CMake as the build system.
- Target Windows.
- Use Vulkan for 3D rendering.
- Build an isometric or top-down RPG.
- Implement the work one feature at a time.
- Start with environment setup and choose the language before source code.
- Use procedural placeholders instead of external art assets.
- Keep each step runnable and testable before the next step starts.
- Create only a planning document.
- Do not create or modify source code.
- Do not create or modify build configuration.
- Write the plan to `docs/implementation/vulkan-isometric-rpg-plan.md`.

## Scope

Create only a step-by-step implementation plan for a small playable game.

Do not create or modify source code.
Do not create or modify build configuration.
Do not create assets or execute build and test commands.

Include:

- A fixed isometric camera.
- Player movement on a small 3D level.
- Procedural ground and environment geometry.
- Player and enemy models made from simple shapes.
- Basic melee combat.
- Enemy movement and basic enemy behavior.
- Health, damage, defeat, and respawn or game-over states.
- A small inventory or equipment system.
- One or more quest objectives.
- Character stats.
- A heads-up display for health and objectives.
- A pause menu and a restart option.
- One complete playable level.
- Windows build instructions.

Exclude:

- Network multiplayer.
- A large open world.
- Advanced character animation.
- A 3D modeling or content pipeline.
- Licensing or distribution of external art assets.
- Commercial-quality art and sound.

## Implementation plan

Write a detailed bullet-point plan.
Use the following order:

1. **Prepare the development environment**
   - Confirm the Windows build target and available hardware.
   - Install a C compiler and CMake.
   - Install the Vulkan SDK.
   - Confirm that Vulkan drivers and validation layers are available.
   - Create the initial CMake project structure.
   - Add an executable target.
   - Add a debug build configuration.
   - Add a release build configuration.
   - Print a minimal console message.
   - Build and run the executable.

2. **Create the application foundation**
   - Create a main application loop.
   - Create an application shutdown path.
   - Handle window creation and destruction.
   - Handle window resize events.
   - Track elapsed time.
   - Use a fixed update step for gameplay.
   - Add an error-handling path for required system failures.
   - Stop development when the application cannot start or close cleanly.

3. **Initialize Vulkan**
   - Create a Vulkan instance.
   - Enable required validation layers in debug builds.
   - Select a physical device.
   - Create a logical device.
   - Create graphics and presentation queues.
   - Create a surface for the Windows window.
   - Select a supported surface format.
   - Select a present mode.
   - Stop development when this step is unreliable.

4. **Create the rendering infrastructure**
   - Create the swap chain.
   - Create swap-chain image views.
   - Create a render pass.
   - Create frame buffers.
   - Create the command pool.
   - Allocate command buffers.
   - Create synchronization objects.
   - Handle resize and swap-chain recreation.
   - Render a clear color to a window.
   - Stop development when the clear color is stable.

5. **Add a minimal 3D pipeline**
   - Define vertex and uniform data structures.
   - Compile or load a basic vertex shader.
   - Compile or load a basic fragment shader.
   - Create pipeline layout objects.
   - Create the graphics pipeline.
   - Create a depth buffer.
   - Create a projection matrix.
   - Create a view matrix.
   - Create a model matrix.
   - Render one simple 3D shape.
   - Stop development when the shape is visible from the isometric camera.

6. **Create procedural mesh support**
   - Create a cube mesh.
   - Create a plane mesh.
   - Create vertex and index buffers.
   - Create reusable mesh upload functions.
   - Add draw commands for meshes.
   - Add per-object transform and color data.
   - Render several meshes in a scene.
   - Verify mesh transformations and depth order.

7. **Add the isometric camera**
   - Define an isometric or fixed top-down camera.
   - Position the camera above and away from the level.
   - Point the camera at the player area.
   - Smooth the camera follow behavior.
   - Keep the camera inside a fixed level boundary.
   - Test the camera in each screen edge and at resize.

8. **Build the first level**
   - Create a fixed level layout.
   - Create the ground from procedural tiles.
   - Add walls or collision boundaries.
   - Add decorative obstacles.
   - Keep the level small and readable.
   - Add a spawn position.
   - Verify that the level uses consistent world units.

9. **Add input and player movement**
   - Add keyboard input handling.
   - Map movement to isometric screen directions.
   - Restrict movement to the level.
   - Detect collisions with walls and obstacles.
   - Use acceleration or direct movement only after the basic movement works.
   - Add player facing or rotation.
   - Add idle and movement states.
   - Render the player as a simple procedural shape.

10. **Add combat primitives**
    - Add a basic melee attack action.
    - Add an attack cooldown.
    - Define an attack range.
    - Apply damage to targets in the range.
    - Add a hit or damage state.
    - Prevent repeated damage during the same attack.
    - Expose health for player and enemy objects.
    - Test combat against stationary targets.

11. **Add enemies and AI**
    - Create enemy data and enemy state.
    - Spawn enemies in the level.
    - Render enemies as procedural shapes.
    - Detect the player within an awareness range.
    - Move enemies toward the player.
    - Stop enemy movement outside the awareness range.
    - Prevent enemies from crossing walls.
    - Add a basic melee attack for enemies.
    - Test enemy defeat and removal.

12. **Add the game state**
    - Add menu, playing, paused, won, and lost states.
    - Add health and defeat handling.
    - Add a game-over state.
    - Add a level-complete state.
    - Add restart behavior.
    - Add pause behavior.
    - Preserve or reset game data according to the selected state.

13. **Add stats and progression**
    - Define health, damage, and movement attributes.
    - Define a damage formula.
    - Add experience points.
    - Add a simple level-up rule.
    - Increase a small set of player attributes after a level-up.
    - Keep progression values configurable in source or data.

14. **Add a minimal inventory and equipment**
    - Define inventory items.
    - Add a small item collection action.
    - Add an item list.
    - Add equip and unequip actions.
    - Add one or more equipment slots.
    - Apply equipment changes to combat or defense.
    - Handle a full or limited inventory clearly.

15. **Add quests and objectives**
    - Define a small quest structure.
    - Add one objective to defeat a required number of enemies.
    - Add one optional objective if it improves the small game.
    - Update objectives from gameplay events.
    - Show quest completion.
    - Mark the level complete after the main objective succeeds.

16. **Create the heads-up display**
    - Render health information.
    - Render player level or experience information.
    - Render the active quest objective.
    - Render the interaction prompt when needed.
    - Keep the display readable at supported resolutions.
    - Draw the display with Vulkan-compatible 2D rendering or a simple text method.

17. **Add menus and user feedback**
    - Create a main menu.
    - Create a pause menu.
    - Create a game-over screen.
    - Create a level-complete screen.
    - Add restart and exit actions.
    - Display validation messages for unavailable devices or features.
    - Add minimal audio only when the core game is stable.

18. **Improve stability and performance**
    - Profile frame time on the target Windows system.
    - Measure CPU and GPU work.
    - Limit draw calls through simple batching.
    - Reuse buffers and descriptors where practical.
    - Handle device loss and window resize correctly.
    - Remove known validation-layer errors.
    - Remove known debug build warnings.

19. **Package the game**
    - Add a CMake install target.
    - Copy required runtime files or assets.
    - Produce a Windows debug build.
    - Produce a Windows release build.
    - Test the executable outside the development shell.
    - Document build and run instructions.

20. **Complete a release checklist**
    - Start the game from a clean Windows environment.
    - Complete the playable level.
    - Defeat enemies and complete the objective.
    - Verify pause, resume, restart, and game-over behavior.
    - Verify window resize and device recovery.
    - Verify that all procedural content renders correctly.
    - Fix blocking errors before adding non-required features.

## Acceptance criteria

- The plan exists at `docs/implementation/vulkan-isometric-rpg-plan.md`.
- The plan covers the work from Windows environment setup to a complete small game.
- The plan uses C, CMake, Vulkan, and Windows as the confirmed technologies.
- The plan uses small, sequential, runnable, and testable steps.
- The plan includes procedural placeholder assets.
- The plan distinguishes confirmed requirements from open decisions.
- The plan has no source-code or build-system changes.
- The plan has no duplicate implementation document.

## Constraints and compatibility

- Use no language or framework outside C unless CMake or Vulkan requires it.
- Use only APIs that the target Windows build environment supports.
- Use procedural shapes for all visual placeholders.
- Keep platform-specific code limited to the Windows startup and window integration areas.
- Enable Vulkan validation in debug builds.
- Ask before selecting a consequential third-party library.
- Add a third-party library only when it removes substantial project complexity.

## Open questions or investigation

- Resolve the final level name and narrative context with the user.
- Decide the exact enemy types after the core combat loop works.
- Decide the exact stats and progression values after the combat balance is testable.
- The planning agent must inspect the repository before writing the plan.
- The planning agent must identify repository-specific build and validation conventions.
- The planning agent must not implement unresolved decisions.
- The planning agent must leave consequential unresolved decisions as open questions.

## Validation

- Verify that the output file exists at `docs/implementation/vulkan-isometric-rpg-plan.md`.
- Verify that the plan is a bullet-point implementation sequence.
- Verify that each planned step has a clear outcome.
- Verify that the plan includes validation steps for a future implementation agent.
- Verify that no source code or build configuration changed.
- Verify that no external art asset is required for the initial implementation.
