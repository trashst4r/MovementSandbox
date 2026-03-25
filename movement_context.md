# Movement Sandbox Context

## Purpose

This project is a movement physics research sandbox.

We are replicating the core movement model used in Source engine (Source 1) in a standalone environment, using the public Source SDK 2013 movement code as the baseline reference for `CGameMovement`, `WalkMove`, `AirMove`, friction, and jump/gravity flow. :contentReference[oaicite:0]{index=0}

This is not a game.

This is a movement simulation and analysis tool.

The purpose is to:
- reproduce Source-style acceleration behavior
- inspect how velocity evolves frame by frame
- study grounded/airborne transitions
- study landing and collision edge cases in a controlled sandbox
- build deterministic instrumentation around movement state

---

## Core Movement Model

Source movement is derived from the Quake-style movement model.

It is not:
- force-based physics
- impulse-based acceleration
- a simple global velocity clamp

It is a projection-limited acceleration system.

The defining behavior is that velocity is projected onto the desired direction, and acceleration is only applied to the remaining speed gap along that direction. This is the core idea behind Source-style movement and the main thing this sandbox must preserve. :contentReference[oaicite:1]{index=1}

---

## Canonical Accelerate() Behavior

The canonical Source-style pattern is:

```cpp
currentspeed = DotProduct(velocity, wishdir);
addspeed = wishspeed - currentspeed;

if (addspeed <= 0)
    return;

accelspeed = accel * frametime * wishspeed * surfaceFriction;

if (accelspeed > addspeed)
    accelspeed = addspeed;

velocity += accelspeed * wishdir;

This naming and flow match the public CGameMovement::Accelerate pattern discussed from Source movement code:

currentspeed
wishdir
wishspeed
addspeed
accelspeed

This is the non-negotiable core of the sandbox.

Do not replace this with naive acceleration such as:

velocity += wishdir * accel * dt;

That is not Source-style movement.

Key Concepts
1. Projection-Based Acceleration

Velocity is projected onto the wish direction:

currentspeed = DotProduct(velocity, wishdir);

Then the engine computes how much speed can still be added in that direction:

addspeed = wishspeed - currentspeed;

Only that remaining amount may be added.

This is what creates directional speed limiting instead of a single global cap.

2. Directional Speed Limit

There is no single global clamp that defines the movement feel.

Speed is limited along the current wish direction only.

This allows:

speed gain through directional change
non-linear velocity growth
emergent strafing behavior

This directional cap is why changing yaw and input together can keep creating new acceleration opportunities.

3. Wish Direction and Wish Speed
wishdir = normalized desired input direction, built from WASD relative to yaw
wishspeed = desired speed along that direction

wishspeed is not just a cosmetic value. In the public Source-style acceleration formula, it directly scales accelspeed, which is part of why the system feels responsive and why larger intended movement magnitudes accelerate toward their target in a roughly consistent time scale.

4. Ground vs Air Movement

Movement must be split into separate ground and air paths.

Ground
friction applied
higher acceleration
strong directional control
Air
no ground friction
lower acceleration
directional influence only

This split is exposed publicly in Source-family cvars such as sv_accelerate, sv_airaccelerate, and sv_friction, including the Counter-Strike: Source command listings.

5. Friction

Ground friction is state-dependent and speed-dependent.

It should:

apply only when grounded
reduce horizontal speed proportionally
run before ground acceleration

Representative structure:

speed = length(horizontal_velocity);

if (speed > 0)
{
    drop = speed * friction * dt;
    newspeed = max(0, speed - drop);
    horizontal_velocity *= newspeed / speed;
}

Do not use constant decay on all movement states. Ground friction belongs to the ground path. Public Source-family documentation and cvar listings expose sv_friction as part of this movement model.

6. Jump Behavior

Jumping should:

set vertical velocity
preserve horizontal velocity
transition from ground rules to air rules

Horizontal momentum should carry through the jump. That preserved horizontal velocity is part of the characteristic Source movement feel. Valve’s public movement and bunnyhop documentation describe this behavior at a high level.

Emergent Behavior

When the above rules are implemented correctly, the system naturally produces:

Strafing

Changing yaw changes wishdir, so each frame may create a new valid acceleration opportunity.

Speed Gain

Because acceleration is capped directionally rather than globally, total speed magnitude can increase through correct alignment.

Momentum Preservation

Horizontal velocity carries across jumps and through air movement, subject to air-accel limits rather than ground-friction rules.

These are the behaviors we want to study in the sandbox.

What We Are Replicating
Movement Core
projection-based Accelerate()
separate ground and air movement paths
speed-dependent ground friction
gravity
jump
horizontal momentum preservation
Collision Model (Simplified)
AABB player
AABB world geometry
axis-separated resolution (X → Y → Z)
stable landing detection
basic wall sliding

This is a simplification of Source tracing and collision, but it is acceptable for this sandbox as long as the movement-state transitions remain readable and deterministic.

Camera Model
yaw-relative movement direction
pitch affects view only
no roll
What We Are Not Replicating

Do not attempt to implement:

the full Source physics engine
networking
prediction/interpolation
animation systems
weapons or gameplay systems
map formats
branch-specific engine quirks unless they are directly relevant to core movement behavior

The focus is movement math and movement-relevant collision behavior.

Simulation Layers
Layer 1 — Movement Math
wishdir
wishspeed
currentspeed
addspeed
accelspeed
friction
gravity
Layer 2 — Movement State
grounded
prevGrounded
jump events
landing events
Layer 3 — Geometry
floor
walls
platforms
narrow ledges
test geometry for precision contact scenarios
Instrumentation Requirements

The sandbox must expose the following values in debug output:

velocity (x, y, z)
horizontal speed
vertical speed
grounded
prevGrounded
wishdir
currentspeed
addspeed
accelspeed
position delta per frame

These values must remain visible and easy to inspect.

Instrumentation is a requirement, not an optional polish feature.

Design Constraints
deterministic simulation
readable movement code
minimal abstraction
no feature creep
maintain build stability after every change
keep movement logic explicit
prefer correctness over convenience
Instructions for Codex

When modifying this project:

preserve the projection-based Accelerate() model
do not replace projection logic with naive acceleration
do not introduce a global velocity clamp as a substitute for directional limiting
keep ground and air movement paths separate
keep friction grounded-only
preserve horizontal momentum through jumps
keep debug instrumentation intact or improve it
keep the code compact and readable
do not convert the project into a general FPS or game framework

If uncertain:

match Source SDK terminology and flow
prefer correctness over convenience
prefer clarity over abstraction
do not simplify the core movement math
Immediate Priority

Validate that the current sandbox correctly demonstrates:

projection-based acceleration
addspeed approaching zero as velocity aligns with wishdir
weaker air acceleration than ground acceleration
friction applied only while grounded
horizontal velocity preserved across jumps

Only after this is validated should we expand:

geometry complexity
instrumentation depth
precision landing scenarios
Reference Notes

Primary public references for this project:

Source SDK 2013 movement architecture (CGameMovement, WalkMove, AirMove, movement code structure). Use this as the public baseline for terminology and control flow.
Public discussion and excerpt of Source-style Accelerate() showing currentspeed, addspeed, and accelspeed naming and flow.
Valve Developer Wiki command listings for Counter-Strike: Source / Source-family movement cvars such as sv_accelerate, sv_airaccelerate, and sv_friction.