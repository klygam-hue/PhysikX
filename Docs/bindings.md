# Language integration

Build the native library first. Match architecture (for example x64 with x64),
and keep the native binary discoverable by the platform loader. Bindings ship
as sources in this alpha; they are not published to PyPI, NuGet, crates.io or Maven.

## C and C++

Include `PhysikX/physikx.h` or `PhysikX/physikx.hpp` and link PhysikX.
After `cmake --install build --prefix /your/prefix`, consumers can use:

```cmake
find_package(PhysikX CONFIG REQUIRED)
target_link_libraries(your_app PRIVATE PhysikX::PhysikX)
```

Set `CMAKE_PREFIX_PATH` to that prefix. In C, always destroy the world and check
status codes. `Tests/c_smoke.c` is a complete C example.

## Python

```sh
python -m pip install ./Bindings/Python
export PHYSIKX_LIBRARY="$PWD/build/libPhysikX.so"
python Examples/falling_ball.py
```

Alternatively use `PYTHONPATH="$PWD/Bindings/Python"` without installation.
On macOS choose `.dylib`; on Windows use an absolute path to `PhysikX.dll`.
Use `with PhysikX.World(...)` for deterministic cleanup. Query and contact methods
copy native output; no raw native pointer is exposed by the public API.

## C# (.NET 8)

```sh
# Linux loader path:
export LD_LIBRARY_PATH="$PWD/build:${LD_LIBRARY_PATH:-}"
dotnet run --project Examples/CSharp/Demo.csproj -c Release
```

macOS uses `DYLD_LIBRARY_PATH`. Windows: put `PhysikX.dll` in the app output
folder or add its folder to PATH. Reference `Bindings/CSharp/PhysikX.csproj` from
your application. Use `using var world = new PhysikX.World(3);`.
Customize `World.Defaults()` and `World.BodyDefaults()` before construction.
All P/Invoke calls use the C calling convention and sequential-layout structures.

## Rust

```sh
export PHYSIKX_LIB_DIR="$PWD/build"
export LD_LIBRARY_PATH="$PWD/build:${LD_LIBRARY_PATH:-}"
cargo test --manifest-path Bindings/Rust/Cargo.toml
cargo run --manifest-path Bindings/Rust/Cargo.toml --example falling_ball
```

Depend with `physikx = { path = "/path/to/PhysikX/Bindings/Rust" }`.
The wrapper owns a non-null native world, returns `Result` and uses `Drop`.
It deliberately does not implement Send/Sync. On Windows provide the import
library directory and make the DLL available through PATH. Native descriptors
use `#[repr(C)]` and fixed-width integers.

## Java (JDK 17+ and JNI)

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DPHYSIKX_BUILD_JAVA=ON
cmake --build build -j
mkdir -p build/java
javac -d build/java Bindings/Java/src/main/java/physikx/World.java Examples/JavaSmoke.java
java -Djava.library.path=build -cp build/java JavaSmoke
```

Set JAVA_HOME if JNI detection cannot find the JDK. A JRE alone lacks the native
JNI headers required for building. Windows multi-config builds use
`-Djava.library.path=build/Release` and require both DLLs in that directory.
The JNI bridge links against the same PhysikX core; Java does not reimplement it.

Use `try (World world = new World(3)) { ... }`. Java exposes bodies, gravity,
forces, impulses, setters and constraints. Advanced world/material descriptor
settings and spatial queries are not exposed in this wrapper yet; use the C ABI
or extend the bridge until binding parity is delivered. Do not concurrently
access or close a world; do not retain native handles outside the wrapper.
