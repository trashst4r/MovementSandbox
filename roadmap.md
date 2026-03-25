# MovementSandbox Roadmap

## Project Goal

Rebuild this project into a **3D Source-style movement physics research sandbox** focused on studying and visualizing movement math in isolation.

This is **not** a live-game integration project and must remain fully self-contained.  
No hooks, no memory access, no game attachment, no external automation.

The purpose is to:

- reproduce Source-inspired first-person movement on a flat plane and simple blockout maps
- inspect acceleration, friction, jumping, air movement, and collision behavior
- study precision landings and edge-contact scenarios in a controlled environment
- test tiny ledges, thin surfaces, and narrow collision tolerances inside our own sandbox
- build deterministic instrumentation for movement-state transitions frame by frame

## Primary Outcome

Produce a local executable that acts as a **movement math calculator + visual sandbox** with:

- first-person 3D camera
- flat test map with simple blocks/platforms
- Source-style horizontal acceleration model
- ground friction
- air acceleration
- gravity + jump
- collision resolution
- debug overlay
- deterministic frame-by-frame movement diagnostics
- configurable tiny platform/ledge test geometry

---

## Development Principles

- Keep code simple, explicit, and debuggable
- Prefer deterministic math over visual polish
- Build movement systems first, visuals second
- Separate simulation logic from rendering logic
- Avoid feature creep
- No networking
- No weapons
- No menus unless needed for testing
- No "game" features beyond movement research

---

## Final Tool Shape

The final sandbox should support:

1. **Movement simulation**
   - ground acceleration
   - air acceleration
   - friction
   - jump impulse
   - gravity
   - horizontal speed preservation behavior

2. **Collision research**
   - floor, wall, and block collisions
   - axis-separated collision resolution
   - thin ledges
   - tiny platforms
   - narrow landing surfaces
   - configurable collision geometry

3. **Instrumentation**
   - current velocity
   - horizontal speed
   - vertical speed
   - grounded state
   - previous grounded state
   - landing frame detection
   - jump frame detection
   - wish direction
   - wish speed
   - dot product / projected speed
   - addSpeed / accelSpeed
   - position deltas
   - contact normals where available

4. **Scenario testing**
   - flat ground strafing
   - wall glide / wall contact
   - landing on narrow ledges
   - repeated jump timing tests
   - tiny-edge landing tests
   - minimal ledge-width sweeps for tolerance measurement

---

## Scope Boundaries

### In Scope
- Source-style movement math recreation
- collision and landing analysis
- tiny ledge and thin platform testing
- debug overlays and deterministic state logging
- local sandbox map geometry
- test presets and reproducible scenarios

### Out of Scope
- any live-game integration
- reading or writing external process memory
- hooking, injection, overlays on top of another game
- scripting against real games
- input automation for real games
- bypasses, cheats, or exploit tooling

---

## Architecture Plan

## Phase 1 - Project Cleanup and Simulation Structure

### Objective
Refactor the current prototype into a clean foundation.

### Tasks
- separate movement math from rendering
- introduce a `Player` struct with:
  - position
  - velocity
  - yaw
  - pitch
  - grounded flag
  - previous grounded flag
- introduce simulation constants in one place
- keep map geometry in a simple structure
- ensure `src/main.cpp` remains readable
- confirm project still builds cleanly with current CMake setup

### Deliverable
A cleaned base project with stable first-person movement shell and readable structure.

---

## Phase 2 - Source-Style Ground and Air Movement Core

### Objective
Implement Source-inspired movement math in a self-contained way.

### Tasks
- implement forward/right vectors from yaw
- implement wish direction from WASD
- implement `Accelerate()` using projection-based acceleration
- implement `ApplyFriction()` for grounded movement
- separate ground and air movement handling
- preserve horizontal momentum correctly through jump
- keep tuning constants grouped and editable

### Deliverable
Movement should feel recognizably closer to Source than generic arcade movement.

---

## Phase 3 - Collision and World Interaction

### Objective
Build reliable collision suitable for movement research.

### Tasks
- retain simple block/AABB world geometry
- implement axis-separated movement resolution:
  - X move + resolve
  - Y move + resolve
  - Z move + resolve
- support standing on top of blocks
- support sliding into walls without jitter
- support landing on raised platforms
- maintain stable grounded detection

### Deliverable
A stable sandbox where movement interacts correctly with blocks and narrow platforms.

---

## Phase 4 - Debug Overlay and State Instrumentation

### Objective
Turn the sandbox into a real movement analysis tool.

### Tasks
Add on-screen debug output for:
- position
- full velocity
- horizontal speed
- vertical speed
- grounded / previous grounded
- jump trigger state
- landing frame
- wish direction
- projected speed in wish direction
- addSpeed
- accelSpeed
- yaw / pitch

Also add optional toggles for:
- freeze camera
- slow simulation
- frame stepping
- debug text on/off

### Deliverable
A movement lab where frame-by-frame behavior is observable.

---

## Phase 5 - Precision Landing Test Geometry

### Objective
Create dedicated research geometry for tiny-edge and narrow-platform study inside the sandbox.

### Tasks
- add configurable ledges/platforms with variable width
- support very small top surfaces
- allow spawning the player at repeatable test positions
- add a few dedicated test rooms:
  - flat acceleration room
  - jump timing room
  - narrow ledge room
  - stacked platform room
- allow editing tiny platform sizes directly in code
- expose ledge width in world units for repeatable testing

### Deliverable
A map layout built specifically for precision landing and contact-state analysis.

---

## Phase 6 - Frame-Accurate Landing and Contact Analysis

### Objective
Measure exact state changes around precision contacts.

### Tasks
Implement tracking for:
- previous frame position
- previous frame velocity
- previous frame grounded
- exact landing frame number
- exact takeoff frame number
- vertical velocity immediately before landing
- horizontal speed immediately before landing
- whether collision occurred on floor / wall / ceiling
- current support surface identifier if applicable

Add optional console/log output when:
- landing occurs
- leaving ground occurs
- colliding with a ledge top occurs
- colliding with a wall edge occurs

### Deliverable
A deterministic analysis layer for precise movement-state transitions.

---

## Phase 7 - Test Presets and Reproducibility

### Objective
Make experiments repeatable.

### Tasks
- add named test presets
- reset player state with one key
- reset to exact spawn position and velocity
- switch between simple test maps
- allow quick tuning of:
  - gravity
  - jump impulse
  - ground accel
  - air accel
  - friction
  - max speed
- optionally add a fixed timestep simulation mode

### Deliverable
A sandbox suitable for repeated measurement instead of one-off manual play.

---

## Phase 8 - Calculator / Inspector Features

### Objective
Turn the sandbox into a movement math calculator, not just a playable test scene.

### Tasks
Add tools that show:
- expected horizontal speed gain per frame
- effect of accel constants on velocity
- distance traveled before landing
- height/time calculations for jump arcs
- landing-speed readouts
- tolerance sweeps for ledge width and landing success
- batch test hooks inside the sandbox codebase only

### Deliverable
A movement research tool that explains outcomes numerically.

---

## Milestone Order

### Milestone 1
Clean structure + correct first-person shell

### Milestone 2
Source-style ground/air movement core working

### Milestone 3
Stable block collision and platform landing

### Milestone 4
Meaningful debug overlay with state instrumentation

### Milestone 5
Precision ledge test room with tiny platforms

### Milestone 6
Frame-accurate landing/contact analysis

### Milestone 7
Resettable presets and calculator-style diagnostics

---

## Coding Rules for Codex

When editing this project:

- do not add unnecessary abstractions
- do not add unrelated engine systems
- do not introduce networking, UI frameworks, or asset pipelines
- keep everything local and self-contained
- prefer straightforward structs and functions
- keep simulation math explicit and readable
- preserve the existing CMake + raylib workflow unless a change is required
- when making large edits, keep the project building at each step
- do not remove debug visibility unless replaced with a better version

---

## Immediate Next Task for Codex

Refactor the current `src/main.cpp` into a cleaner 3D movement sandbox foundation while preserving the current build setup.

Specifically:
- keep the existing CMake structure
- keep raylib
- preserve first-person camera
- preserve block rendering and collision
- preserve Source-style acceleration approach
- organize movement code into clearer helper functions
- add previous-grounded tracking
- add a more useful debug overlay
- keep everything in one file for now unless a split is clearly beneficial

Do not add polish.  
Do not add menus.  
Do not add unrelated gameplay systems.  
Focus only on movement math, collision, and instrumentation.