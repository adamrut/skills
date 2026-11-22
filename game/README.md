# Build and run

Open a Visual Studio developer command prompt.
Run the following commands from the repository root.

```bat
cmake -S game -B build/game-debug -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Debug
cmake --build build/game-debug --config Debug
build\game-debug\Debug\game.exe
```

For a Release build:

```bat
cmake -S game -B build/game-release -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build/game-release --config Release
build\game-release\Release\game.exe
```

## Controls

- `W/A/S/D` and arrow keys move the player.
- `P` toggles pause and resume.
- `R` restarts the application-owned state and timer.
- `Escape` or the window close button closes the application.

The executable creates a native Windows window, initializes Vulkan instance, validation,
device, queue, and surface state, creates the swap chain and depth resources, compiles and
loads minimal cube shaders, and runs a fixed-update/variable-render loop with one visible
perspective scene. Step 6 adds procedural indexed cube and plane meshes, reusable host-visible
vertex/index buffer upload, per-object scale/translation matrices, independent object colors,
and scene-object drawing.
Window resize and presentation failure recreate the swap-chain resources. No player, enemy,
or gameplay systems are created by Step 6.

Step 7 uses one fixed isometric camera at `(7.5, 7.5, 7.5)` looking at the level scene
center `(0, 0, 0)`, with world-up `(0, 1, 0)`, a 45-degree vertical field of view, and
`0.1`/`100.0` near/far planes. The level bounds are `[-4, 4]` on X/Z and `[-0.7, 0.5]` on Y for the
1.2-unit wall height. The fixed view and aspect-dependent perspective are rebuilt in
projection-view order each frame, and the same fixed camera is retained when swap-chain
resources recreate after resize.

Step 8 replaces the procedural preview scene with one fixed 8 by 8 tile level. Tile size
is `1.0` world unit, the ground is at Y `-0.7`, walls are `1.2` units high, and decorative
obstacles are `0.8` units high. Boundary and interior wall tiles are non-walkable, with
their collision extents kept separate from rendered geometry. The spawn position is
world `( -2.5, -0.7, -2.5 )`, corresponding to tile `(1, 1)`. The level exposes
environment-independent tile, world-coordinate, walkability, spawn, and scene-building
helpers for later movement work. It does not add player input or gameplay state.

Step 9 maps `W/A/S/D` and arrow-key input at the Win32 message boundary into held-key
state. The fixed-update loop uses a 2.5-unit-per-second direct movement speed, normalizes
diagonal input, and resolves X/Z movement separately against tile walkability and level
boundaries. The player has idle/moving state and one of eight stable facing directions.
Rendering adds a procedural humanoid-like cube player and directional facing marker while
reusing the existing mesh infrastructure. Restart restores the fixed spawn and clears held
movement input.

Run the foundation tests with:

```bat
ctest --test-dir build/game-debug -C Debug --output-on-failure
ctest --test-dir build/game-release -C Release --output-on-failure
```
