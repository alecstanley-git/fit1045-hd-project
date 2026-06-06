# FIT1045 High Distinction Project -- N-Body Collision Simulator

A from-scratch C++ N-body gravitational simulator with a custom physics integration engine, 
a custom rendering library (window, figures, camera), and other custom tools like a JSON loader and various 
data types (dynamic arrays, vectors). No OpenGL.

---

## Main discussion points

### 1. Overall architecture & design
- **Clean separation of concerns**: 'Simulator' (physics) is fully decoupled
  from 'Window'/'Figure'/'Camera' (rendering) and from 'parsejson' (I/O). The
  solver never knows it is being drawn.
- **State-machine UI loop** in [main.cpp](src/main.cpp): the 'Menu' /
  'MenuCommand' enums drive a render-then-dispatch loop. This separates "what the user is looking at" ('Menu') from "what they asked for"
  ('MenuCommand') -- it decouples button handling from navigation.
- **Header-heavy design**: 'Simulator' is implemented inline in
  [simulator.hpp](include/simulator.hpp). You can see my learning process from writing mostly inline header files to becoming much more comfortable working across header files and separate implementation files.

### 2. The physics engine
- **Leapfrog (kick-drift-kick) integrator** in
  [simulator.hpp:254](include/simulator.hpp#L254). It is a **symplectic** (bounded energy error over long runs) and **time-reversible** integrator, unlike naive Euler which spirals out. It is simple and accurate, especially coupled with the softening factor
- **Softening factor** ('SOFTENING = 0.1') in 'calculate_acceleration' -- prevents
  the '1/r^2' force blowing up to infinity during close encounters / division by
  zero. This tends to be unphysical for very close encounters but remains symplectic and time-reversible. Not an issue in most configurations I demonstrated.
- **Ring initialisation following Toomre & Toomre (1972)** in
  ['build_rings'](include/simulator.hpp#L176) -- massless tracer particles placed
  on circular orbits ('nphi = 12 + 6(i-1)' per ring) given a circular orbital
  velocity 'vphi'. This is taken straight from known literature in astrophysical simulations.
- **Massless tracer optimisation**: ring particles have 'mass = 0' and are skipped
  as force *sources* (the 'j' loop), so the expensive interaction is only between
  massive bodies -- tracers feel gravity but do not exert it. This is a standard
  restricted N-body trick; it is O(N*M) not O(N^2).

### 3. Units & normalisation
- [unitsystem.hpp](include/unitsystem.hpp): the solver runs in **normalised units
  where G = 1**, and 'UnitSystem' converts to/from physical CGS units. The time
  unit is *derived* from the chosen length and mass units via
  't = sqrt(L^3 / (G*M))'. This improves numerical conditioning because the solver purely works in non-dimensional units and unit conversions are only performed when displayed to the user. This allows us to even change the unit system while the simulation is running.

### 4. Data structures (from FIT1045 tasks)
- **Custom 'dynamic_array<T>'** in [dynamic-array.hpp](include/dynamic-array.hpp):
  comes from the tasks done in the unit:
  - **O(1) append** via capacity doubling, and **shrink-to-half at 1/4 full**
    to avoid thrashing.
  - Manual memory management with 'malloc' + **placement new** + explicit
    destructor calls. Using this over 'new' allows us to have unallocated extra capacity.
  - Rule-of-three: copy constructor, copy assignment (with self-assignment guard),
    destructor.
  - 'std::move' in reallocation/append for efficiency.
- **'Vec3' / 'Vec4' / 'Mat4'** in
  [data-structures.hpp](include/data-structures.hpp) with operator overloading -
  shows my understanding of operators.

### 5. The graphics pipeline
- **Full 3D projection from scratch** -- no OpenGL. In
  [camera.cpp](src/camera.cpp) I build a **view matrix** (look-at) and a
  **perspective projection matrix**, then in [figure.cpp](src/figure.cpp) do the
  'viewProj * point' transform, the **perspective divide** ('/w'), and the
  **NDC -> screen mapping** (including the y-flip).
- **Liang-Barsky line clipping** ([figure.cpp:101](src/figure.cpp#L101)) to clip
  axes/points to the figure box. Cited from Wikipedia (yes, I know, but it works).
- **Orbit-camera controls**: drag-to-rotate (elevation on a fixed-radius
  sphere with pole-clamping) and scroll-to-zoom in [camera.cpp](src/camera.cpp).
- **'Figure' as a mini matplotlib**: 'set_xlim', 'set_title', 'plot', 'plot3d',
  'show' -- a reusable plotting abstraction built to mimic my experience with matplotlib and MATLAB formats.

### 6. JSON config loader
- **Hand-written JSON parser**
  ([parsejson.cpp](src/parsejson.cpp)) -- iterator-based, recursive for nested
  objects, with int/double type detection. Lets configurations be modular and
  repeatable. Pairs with 'FreeJson' for manual tree cleanup (recursive delete).
- The supplied configs allow for ready-made, visually interesting demos. **Note: some JSON configs were
  AI-generated in my own format -- disclosed in [main.cpp](src/main.cpp).**

### 7. Cross-platform abstraction
- A single 'Window' interface ([window.hpp](include/window.hpp)) with two native
  backends: Cocoa/Objective-C++ ([window.mm](src/platform/mac/window.mm)) and
  Win32 ([window.cpp](src/platform/win/window.cpp)). The 'void* _window' hides the
  OS handle. Demonstration of my interface-vs-implementation thinking.

---

## Weaknesses of the simulator

- **O(N^2) force calculation**: fine for demos with a small number of massive particles (like this one), but real codes use Barnes-Hut (O(N log N)) tree codes -- I discussed using other simulators at length in my comments for the implementation plan in [main.cpp](src/main.cpp). I tested the simulator on my older Windows PC and it's much slower than an M5 Mac.
- **No energy/momentum conservation tracking**: I deliberately skipped this
  ([main.cpp](src/main.cpp) notes). How do I know my integration scheme is physical without testing for every conservation? The simple answer is that the leapfrog algorithm is, by definition, a symplectic solver (energy is bounded). I mentioned this as a possible extension to the project if more advanced hybrid integration schemes were used (to verify physicality).

---

## Build & run

```bash
make            # builds bin/simulator
./bin/simulator
```
