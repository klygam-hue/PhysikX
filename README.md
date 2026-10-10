# PhysikX

**A renderer-independent 2D/3D physics library by Mistrul.**

Version: **0.1.0-alpha.2**. This is a working foundation with regression tests,
not a production-complete replacement for mature rigid-body engines.

One C++17 core exposes a C ABI. C++, C, C#, Rust, Python and Java use the same
simulation. All code, API names, examples and documentation are in English.

## Implemented

- 2D circles / axis-aligned rectangles and 3D spheres / axis-aligned boxes.
- Dynamic and static bodies; gravity, forces, impulses, velocity and position setters.
- Fixed timestep accumulator with an explicit overload budget and dropped-time statistics.
- Impulse collision response, restitution, Coulomb-style friction and position correction.
- XPBD distance constraints with configurable compliance; build ropes and particle networks.
- Collision layers and masks, contact snapshots, AABB overlap queries, nearest-hit raycasts.
- Double-precision state, input validation, owning language wrappers and stale-handle errors.
- Sweep-and-prune broad phase, compact body storage and reusable contact buffers.
- CMake install/export, native and binding tests, Linux/macOS/Windows CI configuration.

**Limits:** bodies do not rotate. Boxes stay axis aligned. There is no CCD,
convex mesh solver, volume-preserving jelly, fluid solver, GPU backend, automatic
sleeping, or multithreaded world stepping yet. Fast small bodies can tunnel;
choose a smaller timestep and sensible speed limits. See [Docs/plan.md](Docs/plan.md).
Distance networks are a starting point for deformable objects, not realistic jelly.

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
ctest --test-dir build -C Release --output-on-failure
```

Windows: use Visual Studio 2022 with the Desktop development with C++ workload.
Linux/macOS: a C++17 compiler and CMake 3.18+ are sufficient for the core.
Build results are `PhysikX.dll`, `libPhysikX.so` or `libPhysikX.dylib`, depending
on platform. On multi-config Windows builds the DLL is in `build/Release/`.

Without CMake on Linux/macOS: `python3 tools/build.py` builds and tests with
`c++` and `cc`. No external core dependencies are required.

## Python: `import PhysikX`

```sh
python -m pip install ./Bindings/Python
# Linux example; use the appropriate native filename for your platform.
export PHYSIKX_LIBRARY="$PWD/build/libPhysikX.so"
python Examples/falling_ball.py
```

PowerShell: `$env:PHYSIKX_LIBRARY = (Resolve-Path build/Release/PhysikX.dll).Path`.
The Python package does not download or compile native code automatically.

```python
import PhysikX

with PhysikX.World(dimension=3, iterations=12, fixed_dt=1/120) as world:
    world.create_body(shape=PhysikX.BOX, is_static=1,
                      position=(0, -.5, 0), half_extents=(10, .5, 10))
    ball = world.create_body(position=(0, 5, 0), restitution=.6)
    world.apply_impulse(ball, (2, 0, 0))
    for frame in range(180):
        world.step(1/60)
        print(world.state(ball).position)
```

For a graphical 2D example, run `python Examples/playground_2d.py` on a desktop
with Tkinter. Rendering lives in the host app; the physics library has no graphics dependency.

## Other languages

| Language | Entry point | Ownership |
| --- | --- | --- |
| C++ | `#include <PhysikX/physikx.hpp>`; `PhysikX::World` | RAII |
| C | `#include <PhysikX/physikx.h>`; `px_world_create` | explicit destroy |
| C# | project reference to `Bindings/CSharp/PhysikX.csproj`; `using PhysikX;` | `IDisposable` |
| Rust | path dependency on `Bindings/Rust`; `use physikx::World;` | `Drop` |
| Python | install `Bindings/Python`; `import PhysikX` | context manager |
| Java | compile `Bindings/Java/.../World.java`; `import physikx.World;` | `AutoCloseable` |

C#, Rust and Java currently expose the main body/constraint operations; C and
Python additionally expose raycasts, AABB queries and contact snapshots.
[Docs/bindings.md](Docs/bindings.md) contains exact build and loader instructions.

## Customization

World: dimension, gravity, timestep, iterations, substep budget, linear damping,
maximum speed. Bodies: shape, position, velocity, dimensions, mass, restitution,
friction, collision layer/mask and user data. Constraints: rest length and
compliance. See [Docs/api.md](Docs/api.md) for units and behavior.

## Repository

`Code/`, `Bindings/`, `Examples/`, `Tests/`, `Benchmarks/`, `Docs/`, `.github/`,
`README.md`, `CHANGELOG.md` and `LICENSE`. This follows the Code-oriented layout
of 3D Jelly Physics while separating reusable bindings from applications.

## Quality and performance

Run tests before changes to the solver. CI describes checks for all six languages.
Performance is workload dependent: sweep-and-prune reduces sparse pair searches,
but dense overlap still has quadratic worst-case cost. No cross-machine bitwise
determinism is promised. [Docs/validation.md](Docs/validation.md) records the
actual checks performed for this alpha; a workflow file alone is not a passing CI run.

## License

**PhysikX Proprietary Evaluation License 1.0.** Copyright (c) 2026 Mistrul.
All rights reserved except for the limited permissions in [LICENSE](LICENSE).

Recipients may inspect the source and privately compile/run it for evaluation.
Using it in other projects, public/production deployment, redistribution,
relicensing or sale requires Mistrul's prior written permission. This is a
proprietary library with source available for evaluation, not an open-source
project. The Licensor's own use is not restricted by these recipient terms.

Public hosting platform rights and mandatory legal exceptions remain applicable.
Earlier MIT copies retain their existing permissions; this change applies to
new distributions. See [Docs/licensing.md](Docs/licensing.md).
