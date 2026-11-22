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

- `P` toggles pause and resume.
- `R` restarts the application-owned state and timer.
- `Escape` or the window close button closes the application.

The executable creates a native Windows window, initializes Vulkan instance, validation,
device, queue, and surface state, creates the swap chain and depth resources, compiles and
loads minimal cube shaders, and runs a fixed-update/variable-render loop with one visible
perspective scene. Step 6 adds procedural indexed cube and plane meshes, reusable host-visible
vertex/index buffer upload, per-object scale/translation matrices, independent object colors,
and one plane with four cubes.
Window resize and presentation failure recreate the swap-chain resources. No level layout,
player, enemy, or gameplay systems are created.

Step 7 uses one fixed isometric camera at `(6, 6, 6)` looking at the procedural scene
center `(0, 0, 0)`, with world-up `(0, 1, 0)`, a 45-degree vertical field of view, and
`0.1`/`100.0` near/far planes. Camera-preview level bounds are `[-6, 6]` on X/Z and
`[-1, 6]` on Y. The fixed view and aspect-dependent perspective are rebuilt in projection-
view order each frame, and the same fixed camera is retained when swap-chain resources
recreate after resize.

Run the foundation tests with:

```bat
ctest --test-dir build/game-debug -C Debug --output-on-failure
ctest --test-dir build/game-release -C Release --output-on-failure
```
