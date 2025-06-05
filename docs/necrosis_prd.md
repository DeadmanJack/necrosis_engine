
# Product Requirements Document: Necrosis Engine

**Document Version:** 1.0<br>
**Date:** October 26, 2023<br>
**Author:** [Kevin]<br>
**Product Owner:** [Kevin]<br>

---

## 1. Project Overview

### 1.1. Vision Statement
The Necrosis Engine is a high-performance 2D sand simulation engine, providing a modular and extensible foundation for creating interactive cellular automata-based games and applications.

### 1.2. Core Concept
Necrosis is a **modular, multi-threaded, highly optimized 2D sand simulation engine** designed to simulate dynamic interactions between various pixel-based materials. It will support complex physics, rigid body dynamics, and character interactions within a grid-based environment. The engine is developed in C++ and built for performance and extensibility, ensuring it can serve as a reusable core for multiple future projects, not just a single game.

### 1.3. Initial Capabilities
The initial release of Necrosis will simulate the following core pixel types, organized into two fundamental categories:

#### Static Material Pixels (Cellular Automata-Based)
*   **Sand:** Granular physics with realistic color speckles (tan, brown, golden variations), falls and forms natural-looking piles, darkens when wet from water interaction.
*   **Water:** Fluid dynamics with subtle blue variations and transparency effects, flows realistically, shows movement through tiny bubbles flitting to the surface, creates steam when heated.
*   **Steam:** Fluid dynamics with subtle white/gray variations and transparency effects, flows realistically, dissipates over time with wispy edge effects, condenses back to water when cooled.
*   **Wood:** Rigid structure with natural grain patterns and brown color variations, flammable with gradual charring effects, shows realistic burning transitions from brown to black to ash.
*   **Fire:** Dynamic spreading behavior with vibrant color gradients (yellow cores transitioning to red/orange edges), consumes materials with realistic burn effects, extinguished by water with steam generation.
*   **Smoke:** Billowing propagation with varying gray densities and transparency, wispy edge effects, naturally dissipates with realistic fading patterns.

#### Dynamic Particle Pixels (Velocity-Based Physics)
*   **Cinders/Embers:** Glowing particles with orange-to-red color gradients and trailing light effects, ejected from fires with realistic physics trajectories, gradually fade from bright orange to dark red before extinguishing, leave brief glow trails and may ignite materials on contact.
*   **Sparks:** Brilliant white-to-yellow particles with electric blue variations, travel in realistic arcs with crackling visual effects, create brief illumination of surrounding areas, fade rapidly with trailing spark effects.
*   **Debris:** Varied colored fragments matching their source materials (brown wood chips, gray stone fragments, metallic shards), bounce and scatter with realistic physics, show wear and color changes during movement, eventually settle and blend with static materials.
*   **Ash Particles:** Light gray particles with subtle color variations and transparency effects, float with wispy movement patterns, create atmospheric haze effects, gradually settle while maintaining realistic drift behaviors.

### 1.4. Key Differentiators
*   **High Performance:** Achieved through aggressive multi-threading and low-level optimizations.
*   **Visual Richness:** Every pixel type features realistic color variations, gradients, and interface effects rather than flat, uniform colors - creating natural, organic-looking materials.
*   **Hybrid Simulation:** Combines traditional cellular automata for static materials with velocity-based physics for dynamic particle effects.
*   **Modularity:** Designed for easy addition of new pixel types, physics rules, visual effects, and rendering methods without requiring significant core engine changes.
*   **Engine/Game Separation:** A clear API/SDK boundary ensures the engine is a standalone, reusable library.
*   **Advanced Physics:** Beyond basic cellular automata, incorporating rigid body dynamics, particle physics, and character-environment interactions.

---

## 2. Technical Requirements

### 2.1. Base Implementation & Language
*   **Programming Language:** C++ (C++17 or newer, leveraging modern C++ features for performance and safety).
*   **Build System:** CMake (recommended for cross-platform compatibility and project management).
*   **Memory Management:** Prioritize efficient, custom allocators where performance is critical (e.g., for pixel grid data) alongside standard smart pointers, and use of object pooling for all dynamic entities.

### 2.2. Input & Audio Handling
*   **Selected Framework:** **SDL (Simple DirectMedia Layer)** for cross-platform input (keyboard, mouse, gamepad) and audio playback/mixing.
*   **Key Benefits of SDL:**
    *   Mature, stable cross-platform support
    *   Excellent integration with BGFX (proven examples and documentation)
    *   Comprehensive input handling (keyboard, mouse, gamepad)
    *   Built-in audio mixing and playback capabilities
    *   Minimal learning curve and extensive community support
    *   Zlib License (very permissive)
*   **Alternative Frameworks Considered:**
    *   **SFML (Simple and Fast Multimedia Library):** Object-oriented C++ API but less optimal BGFX integration.
    *   **GLFW (OpenGL FrameWork):** Excellent for window/input management but requires separate audio library.
    *   **PortAudio / RtAudio:** Dedicated audio APIs that would require additional complexity for input handling.

### 2.3. Graphics Rendering
*   **Selected Framework:** **BGFX** - A cross-platform, graphics API agnostic rendering library chosen for its excellent balance of performance, control, and ease of integration with SDL.
*   **Key Benefits of BGFX:**
    *   Excellent for 2D pixel manipulation and cellular automata workloads
    *   Proven SDL integration with extensive examples and documentation
    *   Moderate learning curve suitable for rapid development
    *   Efficient command submission and state management
    *   Strong cross-platform support with mature backend implementations
    *   BSD 2-Clause License (commercial-friendly)
*   **Initial Backend Support:** OpenGL (for broader compatibility and easier debugging) and DirectX 11/12 (for Windows native performance).
*   **Planned Backend Expansion:**
    *   **Vulkan:** High-performance, low-overhead graphics API, crucial for next-generation performance.
    *   **Metal:** Apple's low-overhead graphics API, essential for macOS and iOS compatibility.
*   **Alternative Frameworks Considered:**
    *   **The-Forge:** Modern API-focused framework, but deemed overkill for 2D-focused engine requirements.
    *   **Diligent Engine:** Another modern, cross-platform rendering framework offering higher-level abstraction than BGFX.
    *   **Raylib:** Simpler library excellent for prototyping but potentially lacking advanced optimization features.
    *   **SDL_gpu (SDL 3.0):** New graphics API in development, but not yet production-ready.
    *   **Custom Low-Level Implementation:** Direct API usage offering maximum control but requiring significantly more development time.

### 2.4. Architectural Decision Summary
*   **Final Technology Stack:** SDL (input/audio) + BGFX (graphics rendering)
*   **Decision Rationale:**
    *   **SDL + BGFX Integration:** Proven combination with extensive documentation and examples
    *   **2D Optimization:** BGFX excels at 2D pixel manipulation and cellular automata workloads
    *   **Learning Curve:** Moderate complexity suitable for rapid development and iteration
    *   **Cross-Platform:** Both libraries provide excellent cross-platform support matching project requirements
    *   **Performance:** Efficient for target 60 FPS at 1920x1080 with ~50% active pixels
    *   **Licensing:** Both use permissive licenses (Zlib for SDL, BSD 2-Clause for BGFX)
    *   **Community Support:** Strong communities with active maintenance and extensive resources

### 2.5. Performance Optimization Strategy (Based on Noita Analysis)
*   **Spatial Partitioning:** 64x64 pixel chunks for efficient spatial organization and update management
*   **Dirty Rectangle Tracking:** Only update regions where pixels have changed, significantly reducing computational overhead in static areas
*   **Multi-threading Pattern:** Checkerboard threading approach - alternating updates between "black" and "white" chunks across frames to eliminate race conditions in single-buffered simulation
*   **Single-Buffered Simulation:** Memory-efficient approach using spatial partitioning instead of double-buffering
*   **Rigid Body Integration Pipeline:** Marching Squares → Douglas-Peucker simplification → Box2D collision shape generation for pixel-to-physics conversion

### 2.6. Engine/Game Codebase Separation
*   The Necrosis engine must be developed as a **standalone library** (e.g., a static or dynamic library).
*   It will expose a clear, well-defined **C++ API/SDK** for integration into other applications (e.g., games, simulations, tools).
*   All game-specific logic, assets, UI, and higher-level gameplay systems will reside in a **separate codebase** that consumes the Necrosis engine API.
*   This separation ensures reusability and maintainability.

---

## 3. Core Features

### 3.1. Pixel-Level Simulation Mechanics

#### Static Material Simulation (Cellular Automata)
*   **Grid-Based World:** The simulation operates on a 2D grid of pixels (e.g., `uint8_t` for material type, `uint8_t` for property flags).
*   **Cellular Automata Core:** Each pixel updates based on its own state and the state of its neighbors.
*   **Gravity Simulation:** Pixels like sand and water respond to simulated gravity, falling downwards and accumulating.
*   **Flow Dynamics:** Water and other fluid-like materials will exhibit realistic flow behavior, seeking the lowest point and spreading.
*   **Diffusion/Propagation:** Fire will spread to adjacent flammable pixels; heat could diffuse.
*   **State Transitions:** Pixels change state based on interactions (e.g., wood to ash/fire, water to steam/ice - though not all for initial release).

#### Dynamic Particle Simulation (Velocity-Based Physics)
*   **Hybrid Particle System:** Combines grid-based cellular automata with individual particle physics using pixel-to-particle conversion approach (inspired by Noita's proven methodology)
*   **Pixel-to-Particle Conversion:** When pixels are displaced by forces (player actions, explosions, impacts), temporarily remove from grid and spawn as individual particles with velocity and physics properties
*   **Particle-to-Pixel Reintegration:** Convert particles back to grid pixels upon collision with environment or when velocity drops below threshold
*   **Velocity-Based Movement:** Particle pixels maintain individual velocity vectors (x, y components) and move according to physics calculations rather than cellular automata rules
*   **Physics-Based Trajectories:** Particles follow realistic motion paths including parabolic arcs, bouncing, and collision responses
*   **Limited Lifespans:** Each particle has a time-to-live (TTL) value, automatically despawning after expiration to prevent memory bloat
*   **Gravity and Air Resistance:** Particles respond to gravity and optional air resistance for realistic falling and floating behaviors
*   **Collision Detection:** Particles detect collisions with static materials, rigid bodies, and other particles, triggering appropriate responses
*   **Visual Effects:** Support for particle trails, fading opacity, color transitions, and glowing effects during their lifetime
*   **Interaction Capabilities:** Particles can trigger effects on contact (e.g., cinders igniting wood, sparks causing brief illumination)
*   **Behavior System:** Extensible particle behavior framework for defining complex particle interactions beyond basic physics (splash patterns, clustering, chain reactions)

### 3.2. Physics Interactions Between Different Material Types
*   **Collision Resolution:** Basic collision detection and response between pixel types (e.g., sand piles on wood, water flows over sand).
*   **Flammability:** Wood ignites when touching fire, spreads fire.
*   **Extinguishing:** Water extinguishes fire.
*   **Density & Viscosity:** Materials will have configurable properties influencing their movement and interaction.
*   **Reactivity:** Materials react to their environment and other materials (e.g., sand becoming wet, water becoming polluted).

### 3.3. Rigid Body Dynamics
*   **Definition:** Support for defining arbitrary 2D rigid bodies (e.g., squares, circles, custom polygons).
*   **Physics Properties:** Mass, inertia, friction, restitution.
*   **Collision Detection:** Between rigid bodies themselves and between rigid bodies and the pixel simulation grid.
*   **Collision Response:** Realistic impulse-based physics for collisions, allowing rigid bodies to push, rotate, and interact with the pixel environment (e.g., a falling rock displacing sand).
*   **Joints/Constraints (Future Scope):** Potential for hinges, sliders, etc., to connect rigid bodies.

### 3.4. Character Movement and Interaction
*   **Abstract Character Entity:** The engine will provide mechanisms to define and update character entities.
*   **Collision with Environment:** Characters will collide with the pixel grid (e.g., standing on sand, walking through water) and rigid bodies.
*   **Basic Interaction:** Characters can displace pixels (e.g., digging sand), apply forces to rigid bodies, and potentially manipulate materials.
*   **Input Mapping:** The engine will expose input states (via SDL) for character control.

### 3.5. Visual Rendering Features
*   **Material Color Variations:** All pixel types feature realistic color diversity rather than flat, uniform colors:
    *   **Sand:** Multiple speckles (tan, brown, golden, gray particles) creating natural granular appearance
    *   **Water:** Blue, transparency effects, bubbles flitting to surface
    *   **Slime:** Bright green, transparency effects, bubbles flitting to surface
    *   **Wood:** Grain patterns, knots, and natural brown variations from light to dark
    *   **Fire:** Dynamic color gradients from yellow cores to red/orange edges, with occasional blue-white hot spots
    *   **Smoke:** Varying gray densities from light wisps to dark billowing clouds
*   **Material Gradients and Transitions:**
    *   **Fire cores:** Bright yellow/white centers transitioning to red/orange edges    
    *   **Wood burning:** Gradual color transition from brown to charcoal to ash
    *   **Sand moisture:** Darker colors when wet, lighter when dry
*   **Border and Interface Effects:** Materials display different visual characteristics when adjacent to other types:
    *   **Water-sand interface:** Darker, wet sand appearance at boundaries
    *   **Water-stone interface:** Darker stone color at boundaries
    *   **Slime-stone interface:** Florescent slime color at boundaries
    *   **Slime-wood interface:** Florescent slime color at boundaries
    *   **Fire-wood contact:** Glowing ember effects and charring gradients
    *   **Smoke-air boundary:** Wispy, fading edges with transparency gradients
    *   **Material mixing zones:** Blended colors where different materials interact
*   **Dynamic Visual Properties:**    
    *   **Fire spread:** Fire intensity affects color and spread rate
    *   **Smoke density:** Thicker smoke appears darker and more opaque
    *   **Temperature visualization:** Heat-based color shifts (glowing embers)
    *   **Density variations:** Visual thickness changes based on material concentration
    *   **Lighting interactions:** Materials respond to light sources with appropriate shading and highlights

### 3.6. Particle System Architecture
*   **Hybrid Simulation Model:** The engine maintains two distinct but interconnected simulation systems:
    *   **Static Grid System:** Traditional cellular automata for materials like sand, water, wood, fire, and smoke.
    *   **Dynamic Particle System:** Velocity-based physics simulation for short-lived particle effects.
*   **Particle Pool Management:** Efficient memory management using object pools to handle frequent particle creation/destruction without memory fragmentation.
*   **Spatial Partitioning:** Particles use spatial data structures (e.g., spatial hash, quadtree) for efficient collision detection and neighbor queries.
*   **Grid-Particle Interaction:** Seamless interaction between particle pixels and static grid materials:
    *   Particles can collide with and affect static materials
    *   Static materials can spawn particles (e.g., fire creating cinders)
    *   Particles can transition to static materials (e.g., debris settling as sand)
*   **Batch Processing:** Particle updates are batched and potentially GPU-accelerated for optimal performance.
*   **Level-of-Detail (LOD):** Distant or numerous particles may use simplified physics or visual representation to maintain performance.

### 3.6. Modular Design for Extensibility
*   **Data-Driven Material Definitions:** New pixel types and their properties (e.g., color, density, flammability, interaction rules) can be defined via configuration files (e.g., JSON, XML) without recompiling core engine code.
*   **Abstract Physics Interfaces:** Clear interfaces for different physics solvers (e.g., pixel sim, rigid bodies) allowing for alternative implementations.
*   **Render Layer Abstraction:** The rendering component should be separable from the simulation logic, allowing different rendering backends or visualization tools to be plugged in.
*   **Component-Based Entities (Optional but Recommended):** For rigid bodies and characters, a component-based system (e.g., `PhysicsComponent`, `RenderComponent`) would greatly enhance modularity.

---

## 4. Development Approach

### 4.1. Methodology
*   **Agile-Inspired Iteration:** Embrace frequent iteration cycles (e.g., 1-2 week sprints) with clear, achievable goals.
*   **Prioritization:** Focus on core engine stability and performance before expanding features.
*   **MVP (Minimum Viable Product):** The initial target is a stable engine capable of simulating the core pixel types, basic rigid bodies, and a simple character interaction within a renderable window.

### 4.2. Continuous Deployment (CD) Pipeline
*   **"Always Working" Product:** Maintain a high level of code quality and stability in the main development branch. All committed code must be functional and pass automated tests.
*   **Automated Builds:** Implement automated build processes (e.g., GitHub Actions) for cross-platform compilation.
*   **Automated Testing:** Integrate unit tests, integration tests, and performance benchmarks into the CI/CD pipeline.
*   **Regular Releases:** Frequent internal builds for testing and potentially public alpha/beta releases.

### 4.3. Documentation
*   **Complementary Technical Design Document (TDD):** A separate, detailed TDD will follow this PRD, outlining:
    *   Detailed module designs (e.g., `PixelGrid`, `PhysicsSystem`, `RenderSystem`).
    *   Data structures and algorithms (e.g., cellular automata update rules, spatial partitioning).
    *   API specifications for engine integration.
    *   Performance optimization strategies.
*   **Code Comments:** Extensive, clear, and up-to-date comments within the codebase.
*   **API Documentation:** Generate public API documentation (e.g., using Doxygen) for easy integration by external projects.

### 4.4. Version Control
*   **Git:** Use Git for source code management.
*   **Branching Strategy:** Employ a clear branching strategy (e.g., GitHub flow) for feature development, bug fixes, and releases.

---

## 5. Performance Requirements

### 5.1. Multi-threading Approach
*   **Checkerboard Threading Pattern:** Implement proven checkerboard approach where chunks are divided into two sets (like a checkerboard) and updated in alternating frames to eliminate race conditions in single-buffered simulation
*   **64x64 Chunk Processing:** Process 64x64 pixel chunks in parallel across multiple CPU cores, with each chunk maintaining its own dirty rectangle tracking
*   **Spatial Partitioning:** Use chunk-based spatial organization for efficient collision detection, neighbor queries, and update management
*   **Asynchronous Updates:** Rendering preparation, input polling, and physics calculations (rigid bodies) can be performed asynchronously where possible, minimizing stalls
*   **Thread Pool:** Implement or utilize a thread pool to manage worker threads efficiently for chunk processing
*   **Data Locality:** Design data structures to maximize cache utilization and minimize false sharing, with chunk-aligned memory layouts

### 5.2. Optimization Targets
*   **Static Pixel Simulation:**
    *   Target: `60 FPS` for a `1920x1080` (or similar resolution) simulation grid with `~50%` active pixels on minimum specifications.
    *   Memory: Efficient memory footprint per pixel, accounting for additional visual data (color variations, gradients, interface states).
    *   Visual Complexity: Maintain performance while rendering realistic color variations, material gradients, and border effects for all pixel types.
*   **Dynamic Particle Simulation:**
    *   Target: `60 FPS` with `~10,000` to `50,000?` active particles simultaneously on minimum specifications.
    *   Memory: Efficient particle pool management to handle frequent allocation/deallocation without fragmentation.
    *   Culling: Automatic particle culling based on distance, density, and performance thresholds.
*   **Rigid Body Simulation:**
    *   Target: `60 FPS` for `~100` active rigid bodies interacting with both pixel grid and particle systems.
*   **Hybrid System Performance:**
    *   Combined simulation (static pixels + particles + rigid bodies) should maintain `60 FPS` under typical gameplay conditions.
    *   Graceful performance degradation with automatic LOD adjustments when limits are exceeded.
*   **Overall Engine Latency:**
    *   Input to Render: Minimize latency between user input and visual feedback (< 30ms).
*   **CPU Utilization:** Efficient use of all available CPU cores, avoiding bottlenecks on a single thread.

### 5.3. Minimum Hardware Specifications
The engine should be reasonably performant on common mid-range hardware from the last 5-7 years.

*   **CPU:** Quad-core processor (e.g., Intel i5-7th gen / AMD Ryzen 3 1st gen equivalent or newer)
*   **RAM:** 8 GB
*   **GPU:** Integrated graphics (e.g., Intel HD Graphics 620, AMD Radeon Vega 8) or entry-level discrete GPU with modern OpenGL/DirectX support.
*   **Operating System:** Windows 10/11, macOS (latest two versions), modern Linux distributions.

---

## 6. Future Considerations (Beyond Initial Scope)

*   **Additional Static Pixel Types:** Lava, oil, acid, grass, rock, explosives, etc.
*   **Advanced Particle Types:**
    *   **Liquid droplets** (water spray, blood splatter)
    *   **Gas particles** (steam puffs, toxic clouds)
    *   **Energy particles** (magic effects, plasma)
    *   **Composite particles** (burning debris, electrified fragments)
*   **Advanced Physics:** Heat propagation, pressure dynamics, chemical reactions, electrical conductivity, fluid dynamics for particle interactions.
*   **Particle System Enhancements:**
    *   **Particle-to-particle interactions** (clustering, chain reactions)
    *   **Advanced visual effects** (particle lighting, shadows, refractions)
    *   **GPU compute shaders** for massive particle simulation
    *   **Particle emitters and attractors** for complex behaviors
*   **Scripting Language Integration:** Lua or Python for game logic, custom material behaviors, and particle effect definitions.
*   **Networking Support:** For multiplayer simulation or shared environments, including particle synchronization.
*   **Editor Tools:** Integration with or development of tools for designing simulation maps, particle effects, and material properties.
*   **3D Extensibility:** While primarily 2D, consider architecture that wouldn't entirely preclude future 2.5D or volumetric simulation with 3D particles.
