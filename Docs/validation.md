# Validation for 0.1.0-alpha.2

The solver checks below were performed for alpha.1. Alpha.2 changes licensing
and version metadata only; it does not imply that the entire suite was rerun.

Local checks were performed in a Linux x86-64 container with GCC 13.3,
Python 3.12 and OpenJDK 17. This is not a substitute for real platform CI.

## Completed checks

- Strict native build: C++17, `-Wall -Wextra -Wpedantic -Werror`.
- 20 native regression cases: input validation, analytical semi-implicit free fall,
  2D projection, inverse-mass impulse scaling, accumulated forces across partial
  steps, stale handles, swap-removal, constraint cleanup, rigid distance projection,
  coincident endpoints, elastic collision momentum, inside-box sphere correction,
  a 2,400-step resting floor, box contacts, collision masks, shape rays, overlap
  queries, parallel ray misses, bounded time/speed, static invariance and repeatability.
- Pure C compilation/linking against the C++ shared library and a falling-body check.
- 10 Python cases: loading/lifetime, fall, invalid inputs, ABI struct sizes, raycasts,
  overlap queries, constraint cleanup, contact readback, forces/impulses and a
  10,000-step resting-body run.
- Java source compilation and JNI shared-library build: 2D projection, gravity,
  impulse, stale body rejection and try-with-resources destruction.
- Release CMake build and CTest, independent of the compiler-only build script.
- CMake installation/export and a separate downstream `find_package` consumer build.
- Rust 1.99: both owning-wrapper tests passed (fall/lifetime and invalid dimension).
- C#/.NET 8: wrapper and example compiled with Roslyn; the native P/Invoke gravity
  and stale-handle checks passed. The container blocks the normal dotnet CLI process
  metadata query, so direct compiler/runtime invocation was used locally.
- AddressSanitizer and UndefinedBehaviorSanitizer: native and C ABI tests passed
  with `ASAN_OPTIONS=detect_leaks=0`; no memory/undefined-behavior finding.

## Performance sample

One warmed-up Release run of `physikx_benchmark` measured **1.50825 ms/substep**
for **10,000 separated spheres**, 120 timed substeps, default settings, zero
collision candidates. This exercises storage/integration/broad-phase sorting,
not contacts or deformable objects. It is a container-specific sample, not an
FPS promise for a game or a claim about other machines.

## Verification boundaries

The Tkinter desktop playground has not been visually run in this headless container.
No Windows/macOS build, hosted GitHub Actions run or production scene is claimed.
LeakSanitizer cannot inspect the process tree in this container; address/undefined
checks are run separately with leak scanning disabled. Leak checks remain enabled
in the hosted sanitizer workflow. Future work includes dense stacks, fuzzing,
CCD stress, realistic material validation and performance regressions.

## Alpha.2 licensing revision checks

- Identical root, Python and Rust license files verified.
- Python/Rust package metadata refers to the new license file.
- Updated native library compiled with GCC 13.3 and strict warning flags.
- Native/Python version strings and a 2D binding smoke check passed.
- Archive hashes verified; physics regression behavior is unchanged by this revision.


## 0.2.0-beta.1 extension checks — 2026-10-11

Completed locally against a freshly compiled C++17 Release shared library:

- 28 native regression cases (the 20 original cases plus volume/density/weight,
  inverse-mass mutation, normalized thrust, runtime contact friction, analytical
  XPBD stiffness/link breaking, all 2D/3D sphere/box fragment invariants,
  automatic load/impulse/contact fracture, event lifetime and invalid requests).
- 14 Python regression cases, including new ctypes output sizes, material queries,
  elasticity, copied event IDs and manual/automatic fragmentation.
- Strict C11 smoke build and execution against the shared library.
- Complete new C/C++/Python examples extracted from README, compiled/run successfully.
- AddressSanitizer + UndefinedBehaviorSanitizer: all 28 native cases passed locally
  with leak scanning disabled for this container. Hosted leak checking remains enabled.
- Original 64/128-byte descriptor sizes retained; new output sizes checked by tests.

Hosted verification completed successfully for commit
`c1f2f4b98cd94b62f64112a71f23031737ea42cc` in
[Build and test #27](https://github.com/klygam-hue/PhysikX/actions/runs/38158873673):

- Native/C and 14 Python tests passed on Windows, Linux and macOS.
- All four Rust owning/FFI tests passed against version 0.2.0-beta.1.
- C# P/Invoke smoke checks passed, including new output layouts and break events.
- Java source/JNI compilation and material/thrust/fracture smoke checks passed.
- Hosted ASan/UBSan native and C ABI tests passed with default leak checking.

All five jobs concluded success. Rust/.NET/JDK development tools are unavailable
in the local container, so their validation came from the hosted bindings job.
Earlier version results above are historical; this run is not a new performance
measurement or validation of an arbitrary production scene.
