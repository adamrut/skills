# Implement Vulkan Isometric RPG Step 2

## Goal

Create the application foundation for the Windows C and Vulkan game.

Implement only Step 2 in `docs/implementation/vulkan-isometric-rpg-plan.md`.
Stop before Vulkan instance creation and later rendering or gameplay work.

## Confirmed requirements

- Keep the existing CMake project in `game/`.
- Keep C, CMake, and the Windows target.
- Replace the Step 1 console-only entry path with a native window application foundation.
- Create and destroy the native window through a platform integration boundary.
- Handle window activation, close, resize, and shutdown messages.
- Use a monotonic performance timer.
- Maintain a fixed update step and a variable rendering step.
- Route initialization, runtime, and shutdown failures through one consistent error path.
- Support repeated start, close, pause, and restart actions without resource leaks.
- Use clean startup and shutdown behavior.
- Stop if the application cannot start or close cleanly.
- Do not add Vulkan devices, surfaces, swap chains, rendering, gameplay, or third-party libraries.

## Acceptance criteria

- The `game` executable creates a native Windows window.
- The application enters a deterministic fixed-update and variable-render loop.
- Window activation, resize, close, and shutdown events change application state correctly.
- Pause stops simulation work without stopping event or window processing.
- Restart restores application-owned state and timer state safely.
- The application exits without stale handles, threads, timers, or other resource leaks.
- Initialization and runtime failures end through the consistent error path.
- A successful start and close cycle is repeatable.
- The executable builds in Debug and Release configurations.
- No work from plan Steps 3 through 20 is added.

## Scope and exclusions

Create or change only files required for:

- Native Windows window creation and destruction.
- Application-owned state.
- The fixed-update and variable-render loop.
- Monotonic timing.
- Activation, close, resize, pause, restart, and shutdown handling.
- Consistent initialization, runtime, and shutdown errors.
- Directly related documentation and tests.

Exclude:

- Vulkan instance or device creation.
- Vulkan surface, swap chain, pipeline, shader, buffer, or synchronization work.
- Rendering beyond an existing console or no-render placeholder.
- Input systems beyond the controls required to exercise pause and restart.
- Player movement, combat, enemies, menus, HUD, audio, and assets.
- Third-party libraries.

## Constraints and compatibility

- Inspect existing repository conventions and use the established `game/` location.
- Follow the existing CMake target and compiler warning settings.
- Keep Windows-specific integration at an explicit platform boundary.
- Keep application state and timing independent of Windows message details where practical.
- Use portable C for application and timing logic where a portable implementation is available.
- Keep no unresolved consequential behavior in the implementation.
- If a required Windows API or build feature is unavailable, stop and report the exact blocker.

## Open questions or investigation

- Inspect the current `game/` project before editing.
- Determine the current repository build and test conventions.
- Select a suitable initial window size and title without changing the confirmed project scope.
- Determine the minimal pause and restart controls that follow existing Windows patterns.
- Ask before selecting a behavior that cannot be resolved through repository evidence and is materially consequential.

## Validation

- Configure and build the `game` target in Debug and Release modes.
- Run the executable and verify that the native window appears.
- Verify activation, resize, close, and shutdown behavior.
- Verify pause and resume behavior.
- Verify repeated restart cycles.
- Verify that repeated start and close cycles do not leak resources.
- Run the project-standard warning and build checks.
- Verify that no Vulkan initialization or later gameplay code was added.
