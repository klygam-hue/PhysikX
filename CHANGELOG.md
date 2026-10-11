# Changelog

## 0.2.0-beta.1 — 2026-10-11

- Added mass/volume/density/weight readback, runtime mass/density setters and normalized thrust.
- Added runtime friction, XPBD stiffness and breaking-force thresholds for links and bodies.
- Added automatic/manual primitive fracture (four fragments in 2D, eight in 3D) and copied break events.
- Preserved existing ABI v1 descriptor layouts; added separate output structures and 12 C exports.
- Exposed every new operation in C, C++, C#, Rust, Python and Java, with English README references and complete examples.
- Expanded native, C, Python, Rust, C# and Java regression checks for these operations.
- Fracture conserves mass and translational momentum; sphere fragment placement is approximate and does not cut arbitrary meshes.

## Unreleased — documentation update, 2026-10-11

- Appended a complete English command guide to README for all six supported languages.
- Explained current API names, arguments, return values, descriptor settings, units, errors and ownership.
- Added complete usage programs, Windows/Linux/macOS loader guidance and query/result explanations.
- Explicitly documented wrapper coverage gaps and common integration mistakes.
- Updated the roadmap and repository structure documentation; simulation code and ABI are unchanged.

## 0.1.0-alpha.2 — 2026-10-10

- Replaced MIT terms for new distributions with the PhysikX Proprietary Evaluation License 1.0.
- Reserved project integration, deployment, redistribution and sale for separate written permission.
- Added licensing guidance and consistent source/package notices; preserved earlier licensed rights.
- Updated package/native version strings; simulation behavior is unchanged.

## 0.1.0-alpha.1 — 2026-10-10

- Introduced the original PhysikX C++17 physics library with C ABI v1.
- Added translational 2D/3D sphere and axis-aligned box simulation.
- Added fixed steps, forces, impulses, material settings and collision filtering.
- Added XPBD distance constraints, contact inspection, overlap queries and raycasts.
- Added C++, C, Python, C#, Rust and Java entry points and examples.
- Added regression tests, sparse benchmark, CMake export and CI configuration.
- Documented current limits and the roadmap for rotation, CCD and realistic jelly.
