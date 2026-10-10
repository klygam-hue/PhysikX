# API contract (ABI v1)

Use meters, kilograms, seconds and Newtons. Y points upward; gravity defaults
to `(0, -9.81, 0)`. 2D worlds operate in XY and project position/velocity/forces,
gravity and ray directions to Z=0. AABB queries in 2D ignore Z after validating
finite ordered input. `half_extents` means half width/height/depth, not full size.

## Core operations

| Function | Purpose |
| --- | --- |
| `px_world_defaults`, `px_body_defaults` | obtain valid descriptors |
| `px_world_create`, `px_world_destroy` | own the simulation |
| `px_world_step` | advance the fixed-step accumulator |
| `px_world_set_gravity`, `px_world_get_stats` | configure gravity and inspect workload |
| `px_body_create`, `px_body_destroy` | manage monotonically allocated world-local IDs |
| `px_body_get_state` | copy position, velocity, user data |
| `px_body_set_position`, `px_body_set_velocity` | teleport or set motion |
| `px_body_apply_force`, `px_body_apply_impulse` | accumulated force or immediate momentum change |
| `px_distance_create`, `px_distance_destroy` | XPBD distance constraint |
| `px_world_contacts` | copy final-substep pre-correction contacts |
| `px_world_query_aabb` | return overlapping body IDs |
| `px_world_raycast` | nearest shape hit with layer filter |
| `px_version`, `px_last_error` | version and thread-local diagnostic |

## Descriptors and bounds

Create descriptors using defaults and then override fields. Do not zero-initialize
a world or body descriptor and assume it is valid. `reserved` must remain zero.

- `dimension`: 2 or 3. `iterations`: 1..128. `max_substeps`: 1..4096.
- `fixed_dt`: 1e-6..1 second; default 1/120. Typical interactive values: 1/60..1/240.
- `linear_damping`: 0..1e6 per second; velocity factor is `exp(-damping * dt)`.
- `max_speed`: >0..1e9 m/s; default 1000. This is a guardrail, not CCD.
- Finite vector components must have absolute value <=1e9.
- Body mass: 1e-9..1e12 kg, also for static descriptors (ignored by the solver).
- Radius and all half extents must be positive and <=1e9, including Z in 2D.
- Restitution: 0..1. Friction: 0..10. Static flag: 0 or 1.
- Layer/mask are 32-bit bitfields; a pair must pass both directions.
- Constraint rest length: 0..1e9. Compliance: 0..1e9; 0 means rigid distance.
- Elapsed time: 0..3600, finite. All of it may not be simulated under overload.

## Time and forces

`step(elapsed)` accumulates wall-clock seconds. It executes zero or more steps
of exactly `fixed_dt`, up to `max_substeps`. Excess accumulated time is discarded
and counted in `dropped_time`; increase the budget for offline simulations or
step directly with `fixed_dt`. Physics slows relative to wall time when overloaded.

Forces accumulate until a call actually executes a substep. A force is applied
across every substep in that call, then cleared. Reapply a persistent force before
each frame call. Impulses change velocity immediately using inverse mass.
Static bodies reject forces, impulses and velocity changes, but may be teleported.

## Constraints and contacts

Distance constraints attach to body centers. XPBD compliance has units m/N.
Deleting a body removes attached constraints. Connected bodies still collide:
use collision masks to disable self-collision where needed. The distance solver
runs before contact corrections; simultaneous constraints and contacts may
compete and are not a fully coupled solver yet.

Restitution combines with the minimum coefficient of the pair; friction combines
with the geometric mean. Restitution is disabled for normal approach speeds below
0.5 m/s to reduce resting jitter. Contacts describe the final executed substep
before penetration corrections. A call with no substep retains the last snapshot.
Teleport/deletion clears the snapshot. Constraints do not produce contact events.

For contacts and AABB queries, call with `out=NULL`/capacity zero to get count,
then allocate and call again. Total count is returned even if capacity is too
small. Never mutate the world between those two calls. Invalid count-query
arguments return zero and set the diagnostic. This is indistinguishable from
an empty query by count alone; validate query arguments before invoking it.

Rays need nonzero directions, which are normalized internally. Distance is in
meters; starting inside a shape returns distance 0 and zero normal. Misses return
`PX_NOT_FOUND`; wrappers may translate that to `None`. Masks select body layers.

## Safety and threading

All native pointers and output arrays must be valid. Never use a destroyed world,
pass an arbitrary pointer, or destroy twice in C. IDs belong to their original
world and must not be mixed between worlds (different worlds may reuse the same
numbers). Native validation catches unknown IDs, not dangling world pointers.

One world must be accessed on one thread at a time, including reads. Independent
worlds can be used on separate threads. No global solver state exists.

C++ exceptions from allocating operations are caught at the C boundary. C callers
check `px_status`; other language wrappers throw/return an error. Allocation failure
during stepping can leave a partially advanced state; discard/recreate that world
if the application requires transactional recovery. Error text remains valid until
the next error on the same thread; success does not clear it.
