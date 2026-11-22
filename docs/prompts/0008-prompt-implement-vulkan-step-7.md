# Implement Vulkan Isometric RPG Step 7

## Goal

Add a fixed isometric or top-down camera to the Windows C and Vulkan game.

Implement only Step 7 in `docs/implementation/vulkan-isometric-rpg-plan.md`.
Stop before the Step 8 level-building work.

## Confirmed requirements

- Implement Step 7 after the existing application, Vulkan rendering, and Step 6 procedural mesh scene.
- Keep C, CMake, Windows, Vulkan, and the existing project structure.
- Preserve the existing startup, shutdown, fixed/variable frame loop, resize recreation, submission, presentation, depth, and error paths.
- Define one fixed isometric or top-down camera for the complete level.
- Position the camera above and diagonally away from the playable area.
- Point the camera at the player or level center.
- Keep the direct fixed camera working before adding any follow behavior.
- Prevent the camera view from moving beyond fixed level boundaries.
- Test supported window sizes and resize recreation.
- Do not add third-party libraries.

## Scope and exclusions

Create or change only files required for the Step 7 camera and directly related tests and documentation.

Exclude:

- Level layout, level-name, and narrative data.
- Player movement, input mapping, player state, or combat.
- Enemy types, enemy movement, and AI.
- Smooth follow behavior until direct fixed-follow behavior works.
- Collision, inventory, quests, HUD, menus, audio, and progression.
- Changes outside `game/` and directly related documentation.

## Implementation requirements

- Extend the existing view and projection code with documented fixed-camera data.
- Define the fixed camera eye position, target position, near plane, far plane, and world-up direction.
- Use the existing fixed-isometric view convention unless repository evidence requires a compatibility change.
- Add explicit fixed level boundaries that can be reused by Step 8.
- Choose the initial target from the existing procedural scene center.
- Keep the view and projection matrices separate or expose one clear view-projection result.
- Apply camera matrices in a consistent, documented order.
- Recalculate the projection correctly after window size or aspect-ratio changes.
- Prevent camera position and target clamping from changing depth correctness or revealing outside the fixed bounds.
- If a player position is not yet represented, do not invent player movement or follow logic.
- Add environment-independent tests for camera matrix construction, aspect handling, target selection, and boundary clamping where practical.
- Update the implementation map only for files and behavior directly added by Step 7.

## Acceptance criteria

- The executable uses one fixed isometric or top-down camera.
- The camera points at the existing scene center or documented level center.
- The complete procedural scene remains readable at supported window sizes.
- The camera view does not move beyond fixed level boundaries.
- The depth result remains stable after a fixed camera change.
- Resize recreation updates the aspect ratio and retains the fixed camera behavior.
- Debug and Release builds contain no blocking validation errors or compiler warnings.
- Environment-independent tests pass for valid camera and boundary behavior.
- The implementation adds no work from plan Steps 8 through 20.

## Open questions or investigation

- Inspect the existing procedural scene and determine its usable center and fixed boundary extents.
- Inspect the existing matrix and test conventions before changing the Vulkan view code.
- Define Step 7 boundaries as camera-preview bounds around the existing scene.
- Do not infer or implement the final level layout.
- Ask before making a consequential compatibility change if the existing view convention conflicts with the fixed-camera requirement.
- Do not select third-party libraries or external asset formats.

## Validation

- Inspect the repository and follow existing C, CMake, Vulkan, test, and documentation conventions.
- Configure and build the `game` target in Debug and Release modes.
- Run the project-standard CTest command.
- Run the executable on Windows.
- Verify the procedural scene remains readable from the fixed isometric or top-down camera.
- Test each screen edge and supported window size.
- Resize the window and verify camera projection and swap-chain recreation.
- Verify clean shutdown and no Vulkan validation errors.
