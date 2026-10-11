# PhysikX development plan

## Delivered: 0.1.0-alpha.1

- [x] C++17 shared library and stable-layout C ABI v1.
- [x] Renderer-independent 2D/3D translational bodies.
- [x] Circle/sphere and axis-aligned box narrow phase.
- [x] Fixed-step integration, force/impulse operations, collision masks.
- [x] Impulse response, friction and penetration correction.
- [x] XPBD distance constraints with compliance.
- [x] Broad phase, overlap queries, raycasts, contact/statistics readback.
- [x] Owning C++, Python, C#, Rust and Java interfaces; direct C API.
- [x] Regression tests, examples, benchmark and cross-platform CI configuration.
- [x] English documentation and CMake package export.

## Delivered: 0.1.0-alpha.2

- [x] Proprietary evaluation license for new distributions, with prior-rights and platform exceptions.
- [x] Consistent license notices and Python/Rust/.NET packaging metadata.
- [x] English licensing and publication guidance.

## Delivered: command documentation — 2026-10-11

- [x] Appended an English command guide to README for C, C++, C#, Rust, Python and Java.
- [x] Explained every public operation, defaults, parameters, outputs, ownership and errors.
- [x] Added complete per-language command examples and platform-specific setup instructions.
- [x] Documented current wrapper gaps without claiming unimplemented query/settings parity.

## Next: rotation and robust rigid bodies

- [ ] Quaternion orientation, angular velocity, torque, inertia tensors.
- [ ] Oriented boxes and convex support mapping with reliable contact manifolds.
- [ ] Warm-started accumulated impulses and improved stacked-body solver.
- [ ] Continuous collision detection with swept broad phase.
- [ ] Sleeping/islands, cached broad phase and batched state readback.
- [ ] Full parity for queries and settings across all language wrappers.

## Deformable objects

- [ ] Dedicated particle and tetrahedral mesh API.
- [ ] XPBD volume and bending constraints; preserve volume at high softness.
- [ ] Self-collision, robust collision coupling, physically meaningful material parameters.
- [ ] Bounded mouse attachments and input interpolation for fast dragging.
- [ ] Analytical/material validation, high-softness and low-FPS stress scenes.

## Production acceptance

- [ ] Run CI on real Windows, Linux and macOS runners.
- [ ] Fuzz ABI descriptors and lifetime sequences; allocation-failure injection.
- [ ] Performance suite with sparse/dense/stacks/jelly scenes and hardware records.
- [ ] Package signed/reproducible native releases and each ecosystem's packages.
- [ ] Graphics adapters for representative 2D and 3D host engines.
- [ ] Version ABI changes explicitly and establish long-term compatibility policy.

Do not claim mature rigid-body, jelly, GPU or CCD support until implemented and
validated. Update this file and structure.md whenever the project changes.
