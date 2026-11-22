# Verify Vulkan Scene Rendering

## Goal

Run the Windows Vulkan desktop game and confirm that the complete scene is rendered correctly.

Fix rendering defects that the verification finds.
Rerun the verification after each fix.

## Confirmed requirements

- Check the existing `game` executable in both Debug and Release builds when the Windows environment supports both.
- Check the native desktop window rather than an HTML, mobile, or browser viewport.
- Compare rendered output with the scene described by the existing source code and project documentation.
- Confirm that renderable environment and player elements appear on screen.
- The verification must include ground, walls, decorative obstacles, and the procedural player.
- Include the facing marker as part of the procedural player verification.
- Confirm that movement remains visually correct when the player moves.
- Confirm that the scene remains visible after a window resize.
- Confirm that no blocking Vulkan validation errors or presentation failures appear.
- Fix defects found by verification and rerun the checks.

## Acceptance criteria

- The application starts, creates a native window, and presents a frame without a crash or blocking error.
- The 8 by 8 ground is visible in the fixed isometric camera view.
- Boundary walls and interior walls are visible.
- Decorative obstacles are visible.
- The procedural player is visible at the fixed spawn.
- The player has a humanoid-like body and a visually distinct facing marker.
- Ground, walls, obstacles, and the player have visually distinct appearances.
- Player movement changes the screen position for WASD and arrow-key input.
- The player remains visible and correctly orientated after movement.
- The player cannot visually enter a wall or obstacle.
- Window resize recreates or updates the scene without disappearing elements or blocking Vulkan errors.
- Movement, resize, pause, restart, and close paths do not introduce rendering regressions.
- CMake build commands and project-standard CTest commands pass in the tested configurations.
- A verification report lists the rendered elements, test actions, observed result, screenshots or captures when available, and any remaining defect.

## Scope and exclusions

Inspect and change only the files required to verify or fix the rendering behavior.

Do not add new gameplay systems.
Do not add external models or art assets.
Do not change the fixed level layout unless verification proves that the implementation differs from the documented level.
Do not change unrelated application behavior.

Update directly related tests or documentation only when it supports the rendering verification or records a discovered rendering defect.

## Constraints and compatibility

- Use C, CMake, Windows, and Vulkan conventions already present in the repository.
- Preserve the fixed isometric camera and the existing depth, swap-chain, resize, and presentation behavior.
- Use the existing procedural cube and plane mesh infrastructure.
- Treat Vulkan validation output and application crashes as blocking failures.
- Do not make a consequential compatibility or behavior change when repository investigation can determine the intended behavior.

## Open questions or investigation

- Determine the exact launch executable and available build configuration from `game/CMakeLists.txt` and `game/README.md`.
- Determine the complete set of ground, wall, obstacle, and player scene objects from the existing scene-building code and tests.
- Determine whether a reference screenshot exists in the repository.
- If no reference screenshot exists, verify against the procedural scene definitions and project documentation.
- Ask for clarification before making a consequential change that cannot be resolved through repository investigation.

## Validation

1. Inspect `game/README.md`, CMake configuration, scene-building code, rendering code, and existing tests.
2. Configure and build the Debug and Release `game` targets with the repository-standard Visual Studio commands.
3. Run the project-standard CTest commands in the available configurations.
4. Launch `build/game-debug/Debug/game.exe` and `build/game-release/Release/game.exe` when available.
5. Inspect the live rendered frame with the strongest visual method available to the implementing AI.
6. Capture a screenshot or equivalent visual evidence when the available environment supports it.
7. Verify the initial spawn position and all scene categories:
   - 64 ground tiles.
   - Boundary and interior walls.
   - Decorative obstacles.
   - Procedural player body and facing marker.
8. Exercise W, A, S, D and the four arrow keys.
9. Exercise diagonal movement.
10. Move toward a boundary, wall, and obstacle.
11. Resize the desktop window.
12. Toggle pause, restart the application, and close the application.
13. Fix verified rendering defects.
14. Rebuild and rerun CTest after fixes.
15. Relaunch the affected build and repeat the relevant visual checks.
16. Report pass or fail for every acceptance criterion and every visual element.
