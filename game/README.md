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
perspective cube.
Window resize and presentation failure recreate the swap-chain resources. No reusable mesh
support, scene objects, or gameplay systems are created.

Run the foundation tests with:

```bat
ctest --test-dir build/game-debug -C Debug --output-on-failure
ctest --test-dir build/game-release -C Release --output-on-failure
```
