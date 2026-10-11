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


## Materials and fracture extension (0.2.0-beta.1)

Existing ABI v1 descriptors remain unchanged. New standalone output structures
are `px_mass_properties` (56 bytes), `px_material` (24),
`px_distance_properties` (48) and `px_break_event` (88) on supported 64-bit ABIs.
The [final README section](../README.md#mass-thrust-friction-elasticity-and-fracture)
lists all fields and complete usage programs in every language.

| Function | Contract |
| --- | --- |
| `px_body_get_mass_properties` | Copy geometry volume, density, mass, gravitational weight magnitude and vector. |
| `px_body_set_mass` | Finite 1e-9..1e12 kg; preserve velocity and static flag; update inverse mass. |
| `px_body_set_density` | Finite positive kg/m³; derived mass must satisfy mass bounds. |
| `px_body_apply_thrust` | Normalize a bounded, nonzero direction in the world plane and accumulate 0..1e9 N. |
| `px_body_get_material` | Copy friction, restitution and body breaking force. |
| `px_body_set_friction` | Finite 0..10; immediately affects subsequent collision response. |
| `px_body_set_strength` | Finite 0..1e12 N; zero disables fracture. Positive values require a dynamic body. |
| `px_distance_get_properties` | Copy endpoints, rest length, compliance, breaking force and last force estimate. |
| `px_distance_set_elasticity` | Finite 1e-9..1e12 N/m; compliance = 1/stiffness. Resets force estimate. |
| `px_distance_set_strength` | Finite 0..1e12 N; zero disables link breaking. |
| `px_body_fracture` | Replace a dynamic primitive with 4 (2D) or 8 (3D) children; copy the event. |
| `px_world_break_events` | Return total event count; copy min(count, capacity) events if output is provided. |

Volume is sphere/box volume in 3D. In 2D it is circle/rectangle area times an
assumed depth of **1 meter**, regardless of stored Z half-extent. Weight is
mass times current world gravity, including for static bodies; it is not a
scale's support reaction. Extremely small geometry that cannot yield finite
volume/density fails mass-property readback. Mass changes preserve velocity,
so they do not preserve momentum during that explicit edit.

A thrust is an accumulated force, with the same lifetime as `apply_force`.
A 2D direction containing only Z is rejected. Set body strength before applying
loads: automatic fracture compares its threshold to the largest applied net
force, impulse/fixed_dt, intact-link reaction, or normal contact impulse/fixed_dt.
Gravity and tangential friction impulses are excluded. This force proxy is not
stress, tensile strength in Pa or a calibrated real-material failure law.
Links break when trial abs(XPBD lambda)/fixed_dt² exceeds their threshold;
the exceeding correction is skipped. Already-applied corrections in earlier
iterations are retained. Body fracture is evaluated after the substep.

Fragments inherit shape, velocity, friction, restitution, masks and user data.
Mass and pending force are divided equally. Child strength starts at zero to
bound growth until the caller explicitly enables further fracture. Boxes tile
the original box exactly; smaller spheres/circles conserve total volume but
approximate the original shape and can expand its bounds. No rotational motion,
mesh slicing, crack propagation or explosion impulse is implemented.
Each child must have at least 1e-9 kg and valid bounded coordinates. Invalid
manual fracture leaves the parent intact; automatic fracture skips unsplittable
bodies. Manual fragmentation completes allocations before removing the parent.
Allocation failure during an automatic step follows the general partial-step
recovery contract above.

Parent IDs and attached-link IDs become stale. Removing attached links this way
does not generate additional link-break events. Update host renderer objects from
the returned child IDs. Events contain kind (`PX_DISTANCE_BROKEN` = 0,
`PX_BODY_FRACTURED` = 1), source ID, count, up to 8 child IDs and estimated force.
The world clears its event list at the start of a call that actually executes a
substep, accumulates events across all of that call's substeps and retains the
list when no substep runs. Manual fracture appends until the next executed step.
Querying events never consumes them. As with contacts, do not mutate the world
between count and copy calls. Event copies remain readable after parent deletion.
