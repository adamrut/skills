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
