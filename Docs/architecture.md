# Architecture

The native library owns simulation state. Host renderers read body positions and
apply them to their own objects; the core never creates windows or draws geometry.

The world stores bodies contiguously and maps monotonically allocated IDs to their
indices. Swap-removal keeps iteration compact without exposing movable addresses.
Constraints hold body IDs and are removed with either endpoint. The broad phase
sorts X-axis bounds each substep and stops pair searches beyond interval overlap.
Y/Z checks and bilateral collision masks run before narrow-phase geometry.

Each fixed substep performs semi-implicit Euler integration and exponential damping,
XPBD distance projection, velocity reconstruction for constrained worlds, collision
detection, iterative normal/friction impulses, penetration correction and speed
limiting. XPBD multipliers reset every substep. This alpha does not cache contact
manifolds or warm-start across frames; dense stacks and coupled constraints need
further solver work.

The C ABI uses opaque world pointers, fixed-width integer IDs and natural-layout
POD descriptors containing doubles. No STL objects, C++ exceptions or managed
objects cross it. Python uses ctypes; .NET uses P/Invoke; Rust uses repr(C) extern
calls; Java uses a small JNI bridge. C++ ownership uses RAII. Each wrapper provides
an explicit deterministic release mechanism.

Optimization is measured rather than assumed: reusable vectors reduce recurring
allocation after warmup; dense storage improves iteration locality; sparse broad
phase avoids enumerating every pair. Current limitations include per-substep
sorting, unordered-map lookups within constraint/contact loops, linear queries,
quadratic dense-overlap behavior and individual FFI state calls. No SIMD, GPU,
multithreading or hard real-time guarantee is claimed.


Material readback derives volume and density from existing body geometry/mass;
no redundant density cache is stored. Runtime mass updates inverse mass without
changing static flags. Thrust normalizes a direction and uses the existing force
accumulator. Distance links track XPBD reaction estimates and optional breaking
thresholds without changing the original public descriptor layout.

Body load estimation runs only when body strength is enabled. A reusable queue
collects eligible parent IDs after solving. Fragment creation reserves storage
and installs child index entries before removing a copied parent; this avoids
references into reallocating vectors. Children are unbreakable by default.
Break events use a separate POD snapshot shared by all bindings. Events are
cleared once per executed frame call and retained across its substeps.
These features add per-body/per-link state; sparse-benchmark timing from alpha.1
must not be presented as a measurement of this version.
