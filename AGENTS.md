# PhysikX contributor instructions

- Use English for code, identifiers, docs and commit messages.
- Preserve C ABI v1 layouts; introduce a versioned ABI if a layout must change.
- Keep the core dependency-free, C++17 and renderer-independent.
- Validate finite descriptors before mutation; never let C++ exceptions escape C exports.
- Keep world access single-threaded and retain deterministic ownership in each wrapper.
- Update Docs/plan.md, Docs/structure.md and CHANGELOG.md after meaningful changes.
- Run native regression tests and relevant binding tests after solver/ABI changes.
- Use Release benchmarks for performance claims and record the scene and hardware context.
- Do not describe unimplemented rotation, CCD, sleeping or realistic jelly as supported.
- Limit filesystem and remote changes to this project and its explicitly requested outputs.

- Preserve the proprietary evaluation license and package notices; do not silently switch to an open-source license.
- Do not imply that new terms revoke valid permissions on older MIT copies.
