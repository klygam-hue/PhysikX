# PhysikX

**A renderer-independent 2D/3D physics library by Mistrul.**

Version: **0.1.0-beta**. This is a working foundation with regression tests,
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

## Command guide: all supported languages

In this guide, a **command** means a library function or method that your program
calls. It is not a terminal command. Build and loader instructions are in
[Build](#build) and [Docs/bindings.md](Docs/bindings.md). Build the native library
before running any example; installing a language wrapper alone is not enough.

Read in this order:

1. [Shared concepts and settings](#shared-concepts-and-settings).
2. Your language: [C](#c-command-reference), [C++](#cpp-command-reference),
   [C#](#csharp-command-reference), [Rust](#rust-command-reference),
   [Python](#python-command-reference), or [Java](#java-command-reference).
3. [Queries and returned data](#queries-and-returned-data).
4. [Common mistakes](#common-mistakes).

The names below match the current source. Languages have different wrapper
coverage; an unavailable method is explicitly marked rather than given a made-up name.

#### Before running the examples

Open a terminal in the repository root (the folder containing `CMakeLists.txt`).
Build in Release mode using the commands in [Build](#build). Code examples below
are complete programs; save the code to the indicated file rather than typing
C++/Python/etc. into your operating system's shell. Set the native library path
in the same terminal where you run the program.

| Platform | Native file and loader setup |
| --- | --- |
| Windows, PowerShell | Core: `build/Release/PhysikX.dll`. Set `$env:PATH = "$((Resolve-Path build/Release).Path);$env:PATH"` for C/C++/C#/Rust and native dependencies. For Python, also set `$env:PHYSIKX_LIBRARY = (Resolve-Path build/Release/PhysikX.dll).Path`. Rust additionally needs `$env:PHYSIKX_LIB_DIR = (Resolve-Path build/Release).Path`. |
| Linux, shell | Core: `build/libPhysikX.so`. Set `export LD_LIBRARY_PATH="$PWD/build:${LD_LIBRARY_PATH:-}"`. Python: `export PHYSIKX_LIBRARY="$PWD/build/libPhysikX.so"`. Rust: `export PHYSIKX_LIB_DIR="$PWD/build"`. |
| macOS, shell | Core: `build/libPhysikX.dylib`. Set `export DYLD_LIBRARY_PATH="$PWD/build:${DYLD_LIBRARY_PATH:-}"`. Python: `export PHYSIKX_LIBRARY="$PWD/build/libPhysikX.dylib"`. Rust: `export PHYSIKX_LIB_DIR="$PWD/build"`. |

Paths assume the build layouts described above; use your actual output folder
if your generator differs. Java additionally needs the JNI target and
`-Djava.library.path` as described in its section. These are per-terminal settings,
not permanent machine changes. x64 applications need x64 native binaries;
ARM64 applications need ARM64 binaries.

### Shared concepts and settings

**World:** an independent simulation containing bodies and distance constraints.
Create a world, add bodies, apply inputs, call `step`, read positions for your
renderer, then release the world. PhysikX does not draw graphics.

**Body ID:** the number returned when a body is created. Keep this number to
move, inspect or delete that body. IDs belong to one world; do not use an ID from
another world. A constraint has its own ID; do not substitute it for a body ID.
IDs are opaque: do not infer body count or array positions from them.

**Units:** position and size in meters, mass in kilograms, time in seconds,
velocity in meters/second, force in Newtons and impulse in Newton-seconds.
Y points up. Default gravity is `(0, -9.81, 0)`.
Dimension `2` uses the XY plane and projects motion to Z=0; dimension `3` uses XYZ.
Always supply positive Z half extents even for a 2D box.

**Shapes:** sphere/circle = `0`; axis-aligned box/rectangle = `1`.
C/C++ spell these `PX_SPHERE` and `PX_BOX`; Python uses `PhysikX.SPHERE` and
`PhysikX.BOX`. C# and Rust currently use numeric values. Java provides separate
`sphere` and `box` methods. A box with half extents `(2, 1, 3)` has full dimensions
`4 x 2 x 6` meters. Shapes do not rotate.

**Defaults first:** C, C++, C# and Rust should obtain the supplied default
descriptors, then change selected fields. An all-zero descriptor is invalid.
Python obtains defaults internally and lets you override them with keyword
arguments. Java uses native defaults with a smaller set of exposed parameters.

#### World configuration

These settings are chosen when the world is constructed. Only gravity has a
runtime setter in the current API; to change other world settings, create a new world.

| Setting in C/C++/Python/Rust | C# field | Default | Meaning and accepted values |
| --- | --- | --- | --- |
| `dimension` | `Dimension` | `3` when omitted by a wrapper | `2` for XY or `3` for XYZ. Pass explicitly to C/Rust/Java. |
| `iterations` | `Iterations` | `8` | Solver passes per fixed step, `1..128`. More passes cost more CPU and may improve constraint/contact convergence. |
| `max_substeps` | `MaxSubsteps` | `16` | Maximum fixed steps in one `step` call, `1..4096`. A limit prevents an unlimited catch-up loop. |
| `fixed_dt` | `FixedDt` | `1/120` | Physics step length in seconds, `0.000001..1`. This is not a frame rate or a millisecond value. |
| `linear_damping` | `LinearDamping` | `0.05` | Damping per second, `0..1e6`; velocity is multiplied by `exp(-damping * dt)`. `0` disables it. |
| `max_speed` | `MaxSpeed` | `1000` | Positive velocity limit in m/s, at most `1e9`. It does not prevent tunneling. |
| `gravity` | `Gravity` | `(0, -9.81, 0)` | Acceleration vector in m/s squared; `(0, 0, 0)` disables gravity. |
| `reserved` | `Reserved` | `0` | ABI field. Leave it at zero. Python does not accept this as a world keyword. |

Java exposes `dimension` at construction and gravity through `setGravity`.
It currently has no public controls for the other world descriptor fields.

#### Body configuration

All body descriptor fields are set at creation. Afterwards, the current API can
change position and velocity, apply force/impulse, or delete the body. There are
no runtime mass, shape, size, material, layer or mask setters yet.

| Setting in C/C++/Python/Rust | C# field | Default | Meaning and accepted values |
| --- | --- | --- | --- |
| `shape` | `Shape` | `0` | `0` sphere/circle; `1` axis-aligned box/rectangle. |
| `is_static` | `IsStatic` | `0` | `0` dynamic; `1` static. Static bodies are not moved by physics. |
| `layer` | `Layer` | `1` | 32-bit bitfield describing this body's collision categories. |
| `mask` | `Mask` | `0xFFFFFFFF` | 32-bit bitfield describing categories this body may collide with. |
| `position` | `Position` | `(0, 0, 0)` | Body center in meters. |
| `velocity` | `Velocity` | `(0, 0, 0)` | Initial velocity in m/s. Static-body creation replaces the supplied velocity with zero. |
| `half_extents` | `HalfExtents` | `(0.5, 0.5, 0.5)` | Half width/height/depth of a box. Each component must be positive and at most `1e9`. |
| `radius` | `Radius` | `0.5` | Sphere/circle radius, positive and at most `1e9`. |
| `mass` | `Mass` | `1` | `1e-9..1e12` kg. A valid positive mass is still required for a static descriptor, although the solver ignores it. |
| `restitution` | `Restitution` | `0.2` | Bounce coefficient, `0..1`: `0` suppresses bounce; `1` requests full normal-speed restitution. |
| `friction` | `Friction` | `0.5` | Friction coefficient, `0..10`. `0` disables friction. |
| `user_data` | `UserData` | `0` | Unsigned 64-bit application tag copied into body state. It is not a managed object or pointer ownership mechanism. |

Keep radius and all half extents valid even when the selected shape does not use
them. Every vector component must be finite and have absolute value at most
`1e9`; do not pass NaN or infinity. Use integer values within each field's type
range; Python's ctypes integer conversion can wrap out-of-range values.

Two bodies collide only when **both** conditions hold:
`(a.layer & b.mask) != 0` and `(b.layer & a.mask) != 0`.
For example, assigning `mask=0` disables collisions for that body. Distance-linked
bodies still collide unless you configure their masks to exclude each other.

Java's body constructors expose shape, position, size, mass and the static flag.
Velocity can be changed after creation. Material settings, collision bitfields
and user data are not currently exposed in Java.

#### What the main operations do

| Operation | Explanation |
| --- | --- |
| Create body | Copy a validated descriptor into the world and return a body ID. Changing the descriptor afterwards does not change the body. |
| Read state | Copy the current position, velocity and, where exposed, user data. Editing the returned copy does not move the actual body. |
| Set position | Teleport the body center immediately. It also works for static bodies, does not reset velocity, and clears the world's contact snapshot. |
| Set velocity | Replace a dynamic body's velocity immediately. A static body rejects this call, including a zero-velocity setter call. |
| Apply force | Add to accumulated force. With mass `2` kg, `(4,0,0)` N contributes `2` m/s squared on X while applied. Gravity, damping and collisions also affect motion. |
| Apply impulse | Change velocity immediately by `impulse / mass`. With mass `2` kg, `(4,0,0)` N-s adds `2` m/s on X. It is a one-time input. |
| Step | Add elapsed seconds to the fixed-step accumulator and execute zero or more fixed steps. It returns no position; read state afterwards. |
| Create distance | Connect the centers of two distinct existing bodies at a target distance. `rest_length` is in meters, `0..1e9`; `compliance` is in m/N, `0..1e9`. `0` compliance requests a rigid distance; larger values allow more stretch. |
| Delete distance | Remove only that link, leaving both bodies alive. |
| Delete body | Remove the body, its attached distance constraints and the contact snapshot. Its old ID cannot be used again. |
| Release world | Free every remaining body and constraint. All its IDs become unusable. |

`step(elapsed)` accepts finite seconds in `0..3600`. At a default `fixed_dt` of
`1/120`, `step(1/60)` normally performs two fixed steps. A call with insufficient
accumulated time performs no step. Excess time beyond `fixed_dt * max_substeps`
is discarded and counted as `dropped_time`; `step(10)` does not necessarily
simulate ten seconds. For offline simulation, call `step(fixed_dt)` repeatedly.

Forces survive calls that execute no fixed step. When a call executes steps,
its accumulated force applies across all those substeps and is then cleared.
For a continuous thrust, reapply force before each frame's `step` call. Force,
impulse and velocity setters reject static bodies; teleportation is allowed.

Restitution uses the smaller coefficient of a pair; friction uses the geometric
mean. Bounce is suppressed below a normal approach speed of `0.5` m/s to reduce
resting jitter. Distance constraints and collision corrections may compete;
they are not yet a fully coupled solver.

#### Available operations by language

| Operation | C | C++ | C# | Rust | Python | Java |
| --- | --- | --- | --- | --- | --- | --- |
| World/body settings at creation | Full | Full | Full | Full | Full | Selected fields |
| Step, gravity, bodies, forces, impulses, distance constraints | Yes | Yes | Yes | Yes | Yes | Yes |
| Statistics | Yes | Yes | Yes | Yes | Yes | Not exposed |
| Contacts, AABB query, raycast | Yes | Through C API and `native_handle()` | Not exposed | Not exposed | Yes | Not exposed |
| Native version/error strings | Yes | Through C API | Errors become exceptions | Errors become `Err(String)` | Errors become exceptions; package has `__version__` | Errors become exceptions |

<a id="c-command-reference"></a>
### C command reference

Include `#include <PhysikX/physikx.h>` and link the native library.
In the table, `w` is a live `px_world *`, `body` and `link` are `px_id`,
`v` is `px_vec3`, and `&out` means the address of caller-owned output storage.
Functions returning `px_status` return `PX_OK` on success; always check it.

| Command and arguments | Result | How to use it |
| --- | --- | --- |
| `px_version()` | `const char *` | Native version text. Print or copy it; never free it. |
| `px_last_error()` | `const char *` | Diagnostic for the latest error on this thread. Read immediately after failure; success does not clear it. Never free it. |
| `px_world_defaults(dimension)` | `px_world_desc` | Obtain valid default fields for dimension `2` or `3`. A dimension is mandatory. |
| `px_body_defaults()` | `px_body_desc` | Obtain a dynamic sphere descriptor; then override selected fields. |
| `px_world_create(&desc)` | `px_world *` or `NULL` | Construct the world. On `NULL`, read `px_last_error()`. |
| `px_world_destroy(w)` | `void` | Release once when finished. `NULL` is accepted; a dangling pointer or double destruction is unsafe. |
| `px_world_step(w, elapsed)` | `px_status` | Advance elapsed seconds; for example `1.0/60.0`. |
| `px_world_set_gravity(w, v)` | `px_status` | Replace gravity, for example `(px_vec3){0,-9.81,0}`. |
| `px_world_get_stats(w, &out)` | `px_status`; fills `px_stats` | Read counts and dropped time. |
| `px_body_create(w, &desc, &out)` | `px_status`; fills a `px_id` | Create a body and keep its ID. |
| `px_body_destroy(w, body)` | `px_status` | Delete the body and its attached links. |
| `px_body_get_state(w, body, &out)` | `px_status`; fills `px_body_state` | Copy state for rendering or inspection. |
| `px_body_set_position(w, body, v)` | `px_status` | Teleport the center in meters. |
| `px_body_set_velocity(w, body, v)` | `px_status` | Replace a dynamic body's velocity in m/s. |
| `px_body_apply_force(w, body, v)` | `px_status` | Accumulate force in Newtons. |
| `px_body_apply_impulse(w, body, v)` | `px_status` | Apply a one-time impulse in Newton-seconds. |
| `px_distance_create(w, &desc, &out)` | `px_status`; fills a `px_id` | `px_distance_desc` contains `a`, `b`, `rest_length`, `compliance`. Keep the returned constraint ID. |
| `px_distance_destroy(w, link)` | `px_status` | Delete a distance link. |
| `px_world_contacts(w, out, capacity)` | `size_t` total count | Pass `NULL, 0` to count, allocate, then pass the array and capacity to copy. |
| `px_world_query_aabb(w, minimum, maximum, out, capacity)` | `size_t` total count | Return IDs whose body bounds overlap the query box. Count and allocate as above. |
| `px_world_raycast(w, origin, direction, max_distance, mask, &out)` | `px_status`; fills `px_ray_hit` on hit | Find the nearest shape along a ray. `PX_NOT_FOUND` is a normal miss; do not read hit output on a miss. |

Status values: `PX_OK=0`, `PX_INVALID_ARGUMENT=1`, `PX_NOT_FOUND=2`,
`PX_OUT_OF_MEMORY=3`, `PX_INTERNAL_ERROR=4`. `PX_NOT_FOUND` also means an unknown
body/constraint for operations other than raycasting.
Count-returning query functions do not return a status: invalid arguments return
zero and set the diagnostic, so count alone cannot distinguish invalid from empty.

**Complete example covering every exported C command.** Save as `commands.c`.
The helper `require_ok` exits on failure and releases the world.

```c
#include <PhysikX/physikx.h>
#include <stdio.h>
#include <stdlib.h>

static void require_ok(px_status status, px_world *w) {
    if (status != PX_OK) {
        fprintf(stderr, "%s\n", px_last_error());
        px_world_destroy(w);
        exit(EXIT_FAILURE);
    }
}

int main(void) {
    printf("Native version: %s\n", px_version());
    px_world_desc settings = px_world_defaults(3); // Use 2 for XY physics.
    settings.iterations = 12;
    px_world *w = px_world_create(&settings);
    if (!w) {
        fprintf(stderr, "%s\n", px_last_error());
        return EXIT_FAILURE;
    }
    require_ok(px_world_set_gravity(w, (px_vec3){0, -9.81, 0}), w);

    px_body_desc desc = px_body_defaults();
    desc.shape = PX_BOX;
    desc.is_static = 1;
    desc.position = (px_vec3){0, -0.5, 0};
    desc.half_extents = (px_vec3){10, 0.5, 10};
    px_id floor;
    require_ok(px_body_create(w, &desc, &floor), w);

    desc = px_body_defaults();
    desc.position = (px_vec3){0, 3, 0};
    desc.mass = 2;
    desc.user_data = 42;
    px_id ball;
    require_ok(px_body_create(w, &desc, &ball), w);
    desc.position = (px_vec3){2, 3, 0};
    px_id other;
    require_ok(px_body_create(w, &desc, &other), w);

    require_ok(px_body_set_position(w, ball, (px_vec3){0, 3, 0}), w);
    require_ok(px_body_set_velocity(w, ball, (px_vec3){1, 0, 0}), w);
    require_ok(px_body_apply_force(w, ball, (px_vec3){4, 0, 0}), w);
    require_ok(px_body_apply_impulse(w, ball, (px_vec3){0, 2, 0}), w);
    px_distance_desc rope = {ball, other, 2.0, 0.001};
    px_id link;
    require_ok(px_distance_create(w, &rope, &link), w);
    require_ok(px_world_step(w, 1.0 / 60.0), w);

    px_body_state state;
    require_ok(px_body_get_state(w, ball, &state), w);
    printf("Ball Y: %.3f, velocity Y: %.3f\n",
           state.position.y, state.velocity.y);
    px_stats stats;
    require_ok(px_world_get_stats(w, &stats), w);
    printf("Fixed steps: %llu\n", (unsigned long long)stats.steps);

    size_t count = px_world_contacts(w, NULL, 0);
    if (count) {
        px_contact *contacts = malloc(count * sizeof(*contacts));
        if (!contacts) { px_world_destroy(w); return EXIT_FAILURE; }
        px_world_contacts(w, contacts, count);
        printf("First penetration: %g\n", contacts[0].penetration);
        free(contacts);
    }
    px_vec3 lo = {-5, -1, -5}, hi = {5, 5, 5};
    count = px_world_query_aabb(w, lo, hi, NULL, 0);
    if (count) {
        px_id *ids = malloc(count * sizeof(*ids));
        if (!ids) { px_world_destroy(w); return EXIT_FAILURE; }
        px_world_query_aabb(w, lo, hi, ids, count);
        printf("Bodies in query: %zu\n", count);
        free(ids);
    }
    px_ray_hit hit;
    px_status status = px_world_raycast(w, (px_vec3){0, 10, 0},
        (px_vec3){0, -1, 0}, 20, UINT32_MAX, &hit);
    if (status == PX_OK) printf("Ray distance: %g\n", hit.distance);
    else if (status != PX_NOT_FOUND) require_ok(status, w);

    require_ok(px_distance_destroy(w, link), w);
    require_ok(px_body_destroy(w, other), w);
    require_ok(px_body_destroy(w, ball), w);
    require_ok(px_body_destroy(w, floor), w);
    px_world_destroy(w); // Do not access w afterwards.
    return EXIT_SUCCESS;
}
```

Linux, from the repository root after the native build:

```sh
cc -std=c11 commands.c -ICode/include -Lbuild -lPhysikX -o commands_c
LD_LIBRARY_PATH="$PWD/build:${LD_LIBRARY_PATH:-}" ./commands_c
```

On Windows/macOS, use the platform loader setup in [Docs/bindings.md](Docs/bindings.md).

Windows, from a Visual Studio x64 Developer Command Prompt at the repository root:

```bat
cl /nologo /TC /std:c11 /ICode/include commands.c /Fe:commands_c.exe /link /LIBPATH:build/Release PhysikX.lib
set "PATH=%CD%\build\Release;%PATH%"
commands_c.exe
```

<a id="cpp-command-reference"></a>
### C++ command reference

Include `#include <PhysikX/physikx.hpp>`. C++ uses the same `px_*` descriptors,
vectors, shapes and IDs as C. `PhysikX::World` owns the native world; its destructor
releases it automatically. Failed wrapper operations throw `std::runtime_error`.
In the table, `world` is a live `PhysikX::World` and `body`/`link` are `px_id`.

| Command | Result | Explanation |
| --- | --- | --- |
| `PhysikX::World world;` | Owning world | Create a 3D world with default settings. |
| `PhysikX::World world(desc);` | Owning world | Use a customized `px_world_desc` from `px_world_defaults(2)` or `(3)`. |
| `PhysikX::World moved(std::move(world));` / move assignment | Transfers ownership | The original is moved-from; do not call simulation methods on it. Copy construction/assignment are disabled. |
| `world.create_body(desc)` | `px_id` | Copy a body descriptor into the world. |
| `world.state(body)` | `px_body_state` | Copy position, velocity and user data. |
| `world.step(elapsed)` | `void` | Advance elapsed seconds. |
| `world.gravity(v)` | `void` | Replace gravity. The C++ name is `gravity`, not `set_gravity`. |
| `world.position(body, v)` | `void` | Teleport the body center. |
| `world.velocity(body, v)` | `void` | Replace dynamic-body velocity. |
| `world.force(body, v)` | `void` | Add force for the next executed substeps. |
| `world.impulse(body, v)` | `void` | Apply an immediate impulse. |
| `world.destroy_body(body)` | `void` | Delete a body and its attached links. |
| `world.distance(desc)` | `px_id` | Create a link from a `px_distance_desc`. |
| `world.destroy_distance(link)` | `void` | Delete a link. |
| `world.stats()` | `px_stats` | Copy simulation statistics. |
| `world.native_handle()` | Borrowed `px_world *` | Use C query/version APIs where the wrapper has no method. Do not destroy this pointer; the C++ object owns it. |
| `PhysikX::check(status)` | `void` or exception | Convert a C status into a C++ exception; handle normal ray misses separately. |
| `world` leaves scope / `~World()` | Releases ownership | Automatic cleanup. Do not manually call the destructor or `px_world_destroy(world.native_handle())`. |

`px_version()`, `px_last_error()`, `px_world_defaults()` and `px_body_defaults()`
are directly callable in C++. See the C table for their contracts.

**Complete example including C-backed queries.** Save as `commands.cpp`.

```cpp
#include <PhysikX/physikx.hpp>
#include <iostream>
#include <utility>
#include <vector>

int main() {
    try {
        std::cout << "Native version: " << px_version() << '\n';
        auto settings = px_world_defaults(3);
        settings.iterations = 12;
        PhysikX::World world(settings);
        world.gravity({0, -9.81, 0});
        auto desc = px_body_defaults();
        desc.shape = PX_BOX;
        desc.is_static = 1;
        desc.position = {0, -0.5, 0};
        desc.half_extents = {10, 0.5, 10};
        auto floor = world.create_body(desc);
        desc = px_body_defaults();
        desc.position = {0, 3, 0};
        desc.mass = 2;
        auto ball = world.create_body(desc);
        desc.position = {2, 3, 0};
        auto other = world.create_body(desc);
        world.position(ball, {0, 3, 0});
        world.velocity(ball, {1, 0, 0});
        world.force(ball, {4, 0, 0});
        world.impulse(ball, {0, 2, 0});
        auto link = world.distance({ball, other, 2, 0.001});
        world.step(1.0 / 60.0);
        std::cout << "Ball Y: " << world.state(ball).position.y << '\n';
        std::cout << "Fixed steps: " << world.stats().steps << '\n';

        auto *native = world.native_handle(); // Borrowed, not owned here.
        std::vector<px_contact> contacts(px_world_contacts(native, nullptr, 0));
        px_world_contacts(native, contacts.data(), contacts.size());
        px_vec3 lo{-5, -1, -5}, hi{5, 5, 5};
        std::vector<px_id> ids(px_world_query_aabb(native, lo, hi, nullptr, 0));
        px_world_query_aabb(native, lo, hi, ids.data(), ids.size());
        px_ray_hit hit{};
        auto status = px_world_raycast(native, {0, 10, 0}, {0, -1, 0},
                                      20, UINT32_MAX, &hit);
        if (status == PX_OK) std::cout << "Ray distance: " << hit.distance << '\n';
        else if (status != PX_NOT_FOUND) PhysikX::check(status);

        world.destroy_distance(link);
        world.destroy_body(other);
        world.destroy_body(ball);
        world.destroy_body(floor);
        PhysikX::World moved(std::move(world)); // Owns the same, now empty world.
        PhysikX::World target;
        target = std::move(moved); // Also releases target's previous world.
        // Destructors clean up at scope exit, including moved-from objects.
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
```

Linux build/run from the repository root:

```sh
c++ -std=c++17 commands.cpp -ICode/include -Lbuild -lPhysikX -o commands_cpp
LD_LIBRARY_PATH="$PWD/build:${LD_LIBRARY_PATH:-}" ./commands_cpp
```

Windows, from a Visual Studio x64 Developer Command Prompt:

```bat
cl /nologo /EHsc /std:c++17 /ICode/include commands.cpp /Fe:commands_cpp.exe /link /LIBPATH:build/Release PhysikX.lib
set "PATH=%CD%\build\Release;%PATH%"
commands_cpp.exe
```

On macOS, use the same `c++` compilation command and run with
`DYLD_LIBRARY_PATH="$PWD/build:${DYLD_LIBRARY_PATH:-}" ./commands_cpp`.
For a CMake consumer, use the `PhysikX::PhysikX` target shown in
[Docs/bindings.md](Docs/bindings.md).

<a id="csharp-command-reference"></a>
### C# command reference

Reference `Bindings/CSharp/PhysikX.csproj` and add `using PhysikX;`.
Use `new Vec3(x, y, z)`; Z is optional and defaults to zero. Body/constraint IDs
are `ulong`. Methods return values directly or throw `InvalidOperationException`
on native failure. Calls after disposal throw `ObjectDisposedException`.

| Command | Result | Explanation |
| --- | --- | --- |
| `World.Defaults(dimension = 3)` | `WorldDesc` | Obtain default world fields. Use `2` for XY. |
| `World.BodyDefaults()` | `BodyDesc` | Obtain a valid body descriptor before customizing it. |
| `new World(dimension = 3)` | `World` | Construct with default settings. |
| `new World(desc)` | `World` | Construct with a customized `WorldDesc`. |
| `world.CreateBody(desc)` | `ulong` | Create a body; fields use PascalCase, for example `Position`, `Mass`, `IsStatic`. |
| `world.State(body)` | `BodyState` | Copy `Position`, `Velocity`, `UserData`. |
| `world.Step(elapsed)` | `void` | Advance elapsed seconds as a `double`. |
| `world.SetGravity(value)` | `void` | Replace gravity using a `Vec3`. |
| `world.SetPosition(body, value)` | `void` | Teleport the body. |
| `world.SetVelocity(body, value)` | `void` | Replace dynamic-body velocity. |
| `world.ApplyForce(body, value)` | `void` | Accumulate force. |
| `world.ApplyImpulse(body, value)` | `void` | Apply an immediate impulse. |
| `world.DestroyBody(body)` | `void` | Delete a body and attached constraints. |
| `world.Distance(a, b, restLength, compliance = 0)` | `ulong` | Create a center-to-center link and return its constraint ID. |
| `world.DestroyDistance(link)` | `void` | Delete a link. |
| `world.Statistics()` | `Stats` | Copy `Bodies`, `Constraints`, `Candidates`, `Contacts`, `Steps`, `DroppedTime`. |
| `world.Dispose()` / end of `using` | `void` | Free the native world. Repeated `Dispose()` is safe. Prefer `using` for automatic cleanup, including exceptions. |

`DistanceDesc` is a public layout type with `A`, `B`, `RestLength`, `Compliance`,
but the public `Distance` method accepts these as separate arguments.
Contacts, AABB queries, raycasting and native version access are not exposed by
this C# wrapper. Its internal `Native` class is not a public API.

**Complete example covering all public World operations.** Use it as `Program.cs`
in a .NET 8 console project that references the binding project.

```csharp
using System;
using PhysikX;

try {
    var settings = World.Defaults(3);
    settings.Iterations = 12;
    using var world = new World(settings);
    world.SetGravity(new Vec3(0, -9.81, 0));
    var desc = World.BodyDefaults();
    desc.Shape = 1; // Box. Use 0 for a sphere.
    desc.IsStatic = 1;
    desc.Position = new Vec3(0, -0.5, 0);
    desc.HalfExtents = new Vec3(10, 0.5, 10);
    ulong floor = world.CreateBody(desc);
    desc = World.BodyDefaults();
    desc.Position = new Vec3(0, 3, 0);
    desc.Mass = 2;
    desc.UserData = 42;
    ulong ball = world.CreateBody(desc);
    desc.Position = new Vec3(2, 3, 0);
    ulong other = world.CreateBody(desc);

    world.SetPosition(ball, new Vec3(0, 3, 0));
    world.SetVelocity(ball, new Vec3(1, 0, 0));
    world.ApplyForce(ball, new Vec3(4, 0, 0));
    world.ApplyImpulse(ball, new Vec3(0, 2, 0));
    ulong link = world.Distance(ball, other, 2, 0.001);
    world.Step(1.0 / 60.0); // 1.0 avoids integer division.
    BodyState state = world.State(ball);
    Stats stats = world.Statistics();
    Console.WriteLine($"Ball Y: {state.Position.Y}, steps: {stats.Steps}");
    world.DestroyDistance(link);
    world.DestroyBody(other);
    world.DestroyBody(ball);
    world.DestroyBody(floor);
    // 'using' calls Dispose automatically at the end of this scope.
} catch (InvalidOperationException error) {
    Console.Error.WriteLine(error.Message);
}
```

To create a fresh application from the repository root:

```sh
dotnet new console -n CommandsDemo -f net8.0
dotnet add CommandsDemo/CommandsDemo.csproj reference Bindings/CSharp/PhysikX.csproj
```

Replace `CommandsDemo/Program.cs` with the C# example above, set the loader path
from the platform table, then run:

```sh
dotnet run --project CommandsDemo/CommandsDemo.csproj -c Release
```

The existing `Examples/CSharp/Demo.csproj` also shows the project reference.

<a id="rust-command-reference"></a>
### Rust command reference

Add the path dependency described in [Docs/bindings.md](Docs/bindings.md).
Import `use physikx::{World, WorldDesc, BodyDesc, DistanceDesc, Vec3};`.
A vector is `Vec3 { x: 1.0, y: 2.0, z: 0.0 }`; `Vec3::default()` is zero.
Body/constraint IDs are `u64`. Keep a world `mut` for commands that change it.
Operations returning `Result` use `Err(String)` on failure; use `?` in a
function returning `Result`, or explicitly `match` the result.

| Command | Result | Explanation |
| --- | --- | --- |
| `WorldDesc::defaults(dimension)` | `WorldDesc` | Valid default settings for `2` or `3`; dimension is mandatory. |
| `BodyDesc::default()` | `BodyDesc` | Native defaults, including positive mass/size. This is not an all-zero body descriptor. |
| `World::new(desc)` | `Result<World, String>` | Construct an owning world. |
| `world.create_body(desc)` | `Result<u64, String>` | Pass a body descriptor by value; retain the returned ID. |
| `world.state(body)` | `Result<BodyState, String>` | Copy `position`, `velocity`, `user_data`. Read-only borrow. |
| `world.step(elapsed)` | `Result<(), String>` | Advance elapsed seconds. |
| `world.set_gravity(value)` | `Result<(), String>` | Replace gravity using `Vec3`. |
| `world.set_position(body, value)` | `Result<(), String>` | Teleport the body. |
| `world.set_velocity(body, value)` | `Result<(), String>` | Replace dynamic-body velocity. |
| `world.apply_force(body, value)` | `Result<(), String>` | Accumulate force. |
| `world.apply_impulse(body, value)` | `Result<(), String>` | Apply an immediate impulse. |
| `world.destroy_body(body)` | `Result<(), String>` | Delete a body and attached constraints. |
| `world.distance(desc)` | `Result<u64, String>` | Use `DistanceDesc { a, b, rest_length, compliance }`. Both length and compliance must be supplied. |
| `world.destroy_distance(link)` | `Result<(), String>` | Delete a link. |
| `world.stats()` | `Result<Stats, String>` | Copy counts and `dropped_time`. Read-only borrow. |
| `drop(world)` / scope exit | `()` | `Drop` releases the native world automatically. The moved value cannot be used afterwards. |

The wrapper intentionally does not implement `Send`/`Sync`. It exposes no public
native handle, contacts, AABB queries, raycasting or version function.
`BodyState::default()` and `Stats::default()` create zero-valued output containers;
they do not read a world. Formatting output with `Debug` does not change physics.

**Complete example covering all public World operations.** Place in your
application's `src/main.rs` with the binding dependency configured.

```rust
use physikx::{BodyDesc, DistanceDesc, Vec3, World, WorldDesc};

fn main() -> Result<(), String> {
    let mut settings = WorldDesc::defaults(3);
    settings.iterations = 12;
    let mut world = World::new(settings)?;
    world.set_gravity(Vec3 { x: 0.0, y: -9.81, z: 0.0 })?;
    let floor = world.create_body(BodyDesc {
        shape: 1, is_static: 1,
        position: Vec3 { x: 0.0, y: -0.5, z: 0.0 },
        half_extents: Vec3 { x: 10.0, y: 0.5, z: 10.0 },
        ..BodyDesc::default()
    })?;
    let mut desc = BodyDesc::default();
    desc.position = Vec3 { x: 0.0, y: 3.0, z: 0.0 };
    desc.mass = 2.0;
    desc.user_data = 42;
    let ball = world.create_body(desc)?;
    desc.position.x = 2.0;
    let other = world.create_body(desc)?;
    world.set_position(ball, Vec3 { x: 0.0, y: 3.0, z: 0.0 })?;
    world.set_velocity(ball, Vec3 { x: 1.0, y: 0.0, z: 0.0 })?;
    world.apply_force(ball, Vec3 { x: 4.0, y: 0.0, z: 0.0 })?;
    world.apply_impulse(ball, Vec3 { x: 0.0, y: 2.0, z: 0.0 })?;
    let link = world.distance(DistanceDesc {
        a: ball, b: other, rest_length: 2.0, compliance: 0.001,
    })?;
    world.step(1.0 / 60.0)?;
    println!("Ball Y: {}, steps: {}", world.state(ball)?.position.y,
             world.stats()?.steps);
    world.destroy_distance(link)?;
    world.destroy_body(other)?;
    world.destroy_body(ball)?;
    world.destroy_body(floor)?;
    drop(world); // Optional: scope exit also releases it, including on '?'.
    Ok(())
}
```

To create a fresh application from the repository root:

```sh
cargo new commands_rust --edition 2021
```

In `commands_rust/Cargo.toml`, add this line under its existing `[dependencies]`
heading (do not add a second heading):

```toml
physikx = { path = "../Bindings/Rust" }
```

Replace `commands_rust/src/main.rs` with the example above. Set `PHYSIKX_LIB_DIR`
and the loader path from the platform table, then run from the repository root:

```sh
cargo run --manifest-path commands_rust/Cargo.toml
```

<a id="python-command-reference"></a>
### Python command reference

Use `import PhysikX` (capital P and X). Vectors accept `PhysikX.Vec3(x, y, z)`
or a tuple/list of components; this guide always supplies all three components.
Body and constraint IDs are Python integers. Native failures raise `RuntimeError`.
Unknown world/body keywords raise `TypeError`; invalid query bounds raise
`ValueError`; a missing native library raises `OSError` when constructing a world.

| Command | Result | Explanation |
| --- | --- | --- |
| `PhysikX.__version__` | `str` | Python package version, not a function or native-library version query. |
| `PhysikX.Vec3(x, y, z)` | `Vec3` | Vector with readable `x`, `y`, `z`; `tuple(v)` copies components and `repr(v)` displays them. |
| `PhysikX.World(dimension=3, **settings)` | `World` | Create a world. Allowed extra keywords: `iterations`, `max_substeps`, `fixed_dt`, `linear_damping`, `max_speed`, `gravity`. |
| `with PhysikX.World(...) as world:` | Managed scope | Enters the world context; exiting calls `close()`, including on exceptions. |
| `world.create_body(**settings)` | `int` body ID | Every field in the body settings table is accepted as a keyword, using snake_case. Unspecified fields use defaults. |
| `world.state(body)` | `BodyState` | Copy `position`, `velocity`, `user_data`; read `state.position.y`, for example. |
| `world.step(elapsed)` | `None` | Advance elapsed seconds. |
| `world.set_gravity(value)` | `None` | Replace gravity. |
| `world.set_position(body, value)` | `None` | Teleport the body. |
| `world.set_velocity(body, value)` | `None` | Replace dynamic-body velocity. |
| `world.apply_force(body, value)` | `None` | Accumulate force. |
| `world.apply_impulse(body, value)` | `None` | Apply an immediate impulse. |
| `world.destroy_body(body)` | `None` | Delete the body and attached constraints. |
| `world.distance(a, b, rest_length, compliance=0)` | `int` constraint ID | Create a distance link. Length is required; compliance defaults to zero. |
| `world.destroy_distance(link)` | `None` | Delete a link. |
| `world.stats()` | `Stats` | Copy counts and `dropped_time`. |
| `world.contacts()` | `list[Contact]` | Copy the last contact snapshot; an empty list is normal. |
| `world.query_aabb(minimum, maximum)` | `list[int]` | IDs whose bounds overlap the query. Both corners are vectors; no mask argument is exposed. |
| `world.raycast(origin, direction, max_distance=1000, mask=0xFFFFFFFF)` | `RayHit` or `None` | Nearest shape hit; `None` means a miss, not an error. |
| `world.close()` | `None` | Release the world; repeated calls are safe. Other world operations after closing raise `RuntimeError`. |

Public data containers: `WorldDesc`, `BodyDesc`, `BodyState`, `DistanceDesc`,
`Contact`, `RayHit`, `Stats`, plus `Vec3`. Their snake_case fields match the C
layouts described above/below. They are ctypes containers, not independent
simulations. Pass keyword settings to `World`/`create_body`, not a descriptor
object as an undocumented positional argument. `_load`, `_check`, `_vec`,
`_ptr` and other names starting with `_` are implementation details.

**Complete example covering all World commands.** Save as `commands.py`.

```python
import PhysikX

print("Package version:", PhysikX.__version__)
print("Vector:", tuple(PhysikX.Vec3(1, 2, 3)))

try:
    with PhysikX.World(dimension=3, iterations=12, fixed_dt=1/120) as world:
        world.set_gravity((0, -9.81, 0))
        floor = world.create_body(shape=PhysikX.BOX, is_static=1,
            position=(0, -0.5, 0), half_extents=(10, 0.5, 10))
        ball = world.create_body(shape=PhysikX.SPHERE,
            position=(0, 3, 0), mass=2, user_data=42)
        other = world.create_body(position=(2, 3, 0), mass=2)
        world.set_position(ball, (0, 3, 0))
        world.set_velocity(ball, (1, 0, 0))
        world.apply_force(ball, (4, 0, 0))
        world.apply_impulse(ball, (0, 2, 0))
        link = world.distance(ball, other, rest_length=2, compliance=0.001)
        world.step(1/60)
        state = world.state(ball)
        print("Ball Y:", state.position.y, "tag:", state.user_data)
        print("Fixed steps:", world.stats().steps)
        for contact in world.contacts():
            print("Contact:", contact.a, contact.b, contact.penetration)
        print("Query IDs:", world.query_aabb((-5, -1, -5), (5, 5, 5)))
        hit = world.raycast((0, 10, 0), (0, -1, 0), max_distance=20)
        if hit is not None:
            print("Ray:", hit.body, tuple(hit.point), hit.distance)
        else:
            print("Ray missed")
        world.destroy_distance(link)
        world.destroy_body(other)
        world.destroy_body(ball)
        world.destroy_body(floor)
    # The context already called close(). No manual cleanup is needed here.
except (RuntimeError, OSError, TypeError, ValueError) as error:
    print("PhysikX error:", error)

# Explicit cleanup alternative when a context is inconvenient:
world = PhysikX.World(dimension=2)
try:
    world.step(1/120)
finally:
    world.close()
```

From the repository root:

```sh
python -m pip install ./Bindings/Python
```

Set `PHYSIKX_LIBRARY` using the platform table, then run `python commands.py`.
Use `python3` instead of `python` if that is your Python executable's name.
No third-party Python physics package is required.

<a id="java-command-reference"></a>
### Java command reference

Import `physikx.World`. Build the JNI bridge and load both the JNI library and
the core library using the instructions in [Docs/bindings.md](Docs/bindings.md).
IDs are `long`; treat them as opaque native identifiers.
Native failures and use after close throw `IllegalStateException`. A missing JNI
library causes `UnsatisfiedLinkError` during class loading.

| Command | Result | Explanation |
| --- | --- | --- |
| `new World(dimension)` | `World` | Create a world with `2` or `3`. Dimension is mandatory. |
| `world.sphere(x, y, z, radius, mass, isStatic)` | `long` body ID | Create a sphere in 3D or circle in 2D. Position/size/mass are `double`; `isStatic` is `boolean`. |
| `world.box(x, y, z, hx, hy, hz, mass, isStatic)` | `long` body ID | Create an axis-aligned box/rectangle; `hx`, `hy`, `hz` are half extents, not full dimensions. |
| `world.state(body)` | `double[6]` | Copy `[x, y, z, vx, vy, vz]`. Index `1` is Y position; index `4` is Y velocity. User data is not exposed. |
| `world.step(elapsed)` | `void` | Advance elapsed seconds. Use `1.0/60.0`, not integer division `1/60`. |
| `world.setGravity(x, y, z)` | `void` | Replace gravity using three `double` components. |
| `world.setPosition(body, x, y, z)` | `void` | Teleport the body center. |
| `world.setVelocity(body, x, y, z)` | `void` | Replace dynamic-body velocity. |
| `world.applyForce(body, x, y, z)` | `void` | Accumulate force. |
| `world.applyImpulse(body, x, y, z)` | `void` | Apply an immediate impulse. |
| `world.destroyBody(body)` | `void` | Delete the body and attached constraints. |
| `world.distance(a, b, length, compliance)` | `long` constraint ID | Create a distance link. Both length and compliance are required; pass `0.0` for rigid distance. |
| `world.destroyDistance(link)` | `void` | Delete a link. |
| `world.close()` / end of try-with-resources | `void` | Release the world. Repeated close is safe; use try-with-resources for cleanup on exceptions. |

Java does not yet expose descriptors, restitution/friction/masks, statistics,
contacts, AABB queries, raycasts or native version access. Private native methods
such as `stepNative` and `vector` are bridge internals, not commands to call.

**Complete example covering every public command.** Save as `Commands.java`.

```java
import physikx.World;

public class Commands {
    public static void main(String[] args) {
        try (World world = new World(3)) {
            world.setGravity(0, -9.81, 0);
            long floor = world.box(0, -0.5, 0, 10, 0.5, 10, 1, true);
            long ball = world.sphere(0, 3, 0, 0.5, 2, false);
            long other = world.sphere(2, 3, 0, 0.5, 2, false);
            world.setPosition(ball, 0, 3, 0);
            world.setVelocity(ball, 1, 0, 0);
            world.applyForce(ball, 4, 0, 0);
            world.applyImpulse(ball, 0, 2, 0);
            long link = world.distance(ball, other, 2, 0.001);
            world.step(1.0 / 60.0);
            double[] state = world.state(ball);
            System.out.println("Ball Y: " + state[1] + ", velocity Y: " + state[4]);
            world.destroyDistance(link);
            world.destroyBody(other);
            world.destroyBody(ball);
            world.destroyBody(floor);
            // try-with-resources calls close() automatically.
        } catch (IllegalStateException error) {
            System.err.println(error.getMessage());
        }
    }
}
```

Build the JNI target first, using a JDK (17+), not only a JRE:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DPHYSIKX_BUILD_JAVA=ON
cmake --build build --config Release -j
```

Set `JAVA_HOME` to your JDK if CMake cannot find JNI. On Linux/macOS, compile/run
from the repository root:

```sh
mkdir -p build/java
javac -d build/java Bindings/Java/src/main/java/physikx/World.java Commands.java
java -Djava.library.path=build -cp build/java Commands
```

Windows PowerShell, with the PATH setting from the platform table:

```powershell
New-Item -ItemType Directory -Force build/java | Out-Null
javac -d build/java Bindings/Java/src/main/java/physikx/World.java Commands.java
java "-Djava.library.path=build/Release" -cp build/java Commands
```

Keep both `PhysikXJNI` and `PhysikX` native binaries available to the loader.

### Queries and returned data

#### Body state and statistics

| Data | C/C++/Rust/Python field | C# field | Meaning |
| --- | --- | --- | --- |
| Body state | `position`, `velocity`, `user_data` | `Position`, `Velocity`, `UserData` | Snapshot of a body's center, motion and application tag. Java returns only position/velocity as six doubles. |
| Statistics | `bodies` | `Bodies` | Current number of live bodies. |
| Statistics | `constraints` | `Constraints` | Current number of distance links. |
| Statistics | `candidates` | `Candidates` | Broad-phase candidate pairs from the latest executed fixed step. |
| Statistics | `contacts` | `Contacts` | Contact count from the latest executed fixed step; reset to zero when teleportation/deletion clears the snapshot. |
| Statistics | `steps` | `Steps` | Total number of fixed steps since world creation, not frame calls. |
| Statistics | `dropped_time` | `DroppedTime` | Cumulative seconds discarded by the substep budget. |

#### Contacts

Available through C, C++ using its borrowed handle, and Python.
Each contact contains `a` and `b` (body IDs), `normal` (direction from A toward B),
and `penetration` (overlap depth in meters). This is a snapshot of the final
executed substep **before** penetration correction, not a callback or a persistent
collision-start/end event. A step call that executes no substep retains the
previous snapshot. Teleportation/deletion clears it; distance links themselves
do not generate contact events. Returned data is a copy.

For C/C++, count with a null output pointer, allocate enough elements, then copy.
The function returns the total count even if your capacity is too small. Do not
change the world or access it concurrently between counting and copying.
Python performs this allocation internally and returns a list.

#### AABB overlap query

An AABB is an axis-aligned bounding box defined by minimum and maximum corners.
Example: `(-5,-1,-5)` to `(5,5,5)` asks for body bounds overlapping that region.
It checks bounding boxes, so a sphere's bounds may overlap even when the curved
surface does not. The current query returns static and dynamic bodies, applies
no layer mask and does not perform collision response.
Components must be finite, within `+/-1e9`, and minimum must be <= maximum on
every axis. In 2D, Z is ignored after those checks. C/C++ use the count/allocate/
copy pattern; Python returns a list of IDs.

#### Raycast

Inputs: origin, a nonzero direction, maximum distance in meters (`0..1e9`), and
a 32-bit mask selecting body **layers**. Direction is normalized internally;
`(0,-1,0)` and `(0,-10,0)` point in the same direction. A 2D ray must have a
nonzero XY component. The query returns the nearest sphere/box shape hit,
including static bodies; it does not use the body's collision `mask` field.

Result fields: `body` (ID), `point` (world-space hit position), `normal` (outward
surface direction), `distance` (meters from origin). Starting inside a body
returns distance zero and a zero normal. Python returns `None` on a miss;
C/C++ return `PX_NOT_FOUND`, which should be handled separately from errors.
Do not read uninitialized C output after a miss/failure.

### Common mistakes

| Symptom | What to check |
| --- | --- |
| Import succeeds but world creation cannot load the library | Build the native core, match x64/ARM64 architecture, then set the loader path. Python imports lazily; it loads native code at `World()`. |
| Body does not move after `step` | Is it static? Did elapsed time reach `fixed_dt`? In C/C++/C#/Java, use floating-point division (`1.0/60.0`). |
| Simulation seems slow after a long frame | Inspect `dropped_time` where available. Raise `max_substeps` at construction or use fixed steps; one large call is capped. |
| Box is twice the intended size | `half_extents`/`hx,hy,hz` are half dimensions. |
| 2D object loses its Z coordinate | 2D operates in XY; motion is projected to Z=0. Use dimension `3` for depth. |
| Invalid descriptor error | Start from defaults; check positive size/mass, finite values, valid dimension, ranges and zero `reserved`. |
| Setter/force/impulse fails for a floor | Static bodies reject velocity/force/impulse commands. Set position to move a static body. |
| Reading a deleted body's state fails | Keep IDs only while their bodies are alive. Deleting a body also removes attached constraints; their old IDs are no longer valid. |
| Editing a returned state has no effect | State is a copy. Call the position/velocity setter to change the world. |
| Calling a method from another language fails | Names differ (`gravity` in C++, `SetGravity` in C#, `setGravity` in Java). Use your language's table and check wrapper coverage. |
| Continuous force stops accelerating | Reapply it before each frame step. An impulse is a one-time momentum change, not continuous force. |
| Fast bodies pass through thin objects | CCD is not implemented. Reduce fixed timestep/speed or use thicker colliders; `max_speed` alone is not CCD. |
| Crashes around world cleanup or concurrent access | Use scoped ownership, never double-destroy a C world or its C++ borrowed pointer, and access each world on one thread at a time. |

For full bounds, lifetime guarantees and solver limitations, see
[Docs/api.md](Docs/api.md). The command examples demonstrate API use, not a
renderer, a realistic jelly simulation or a promise of identical motion across
hardware.

