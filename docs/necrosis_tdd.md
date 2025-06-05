# Technical Design Document: Necrosis Engine

**Document Version:** 1.0  
**Date:** December 2024  
**Author:** [Kevin]  
**Based on:** Necrosis Engine PRD v1.0 + Noita Technical Analysis  

---

## 1. Architecture Overview

### 1.1. High-Level System Architecture

The Necrosis Engine follows a modular, multi-threaded architecture designed for high-performance 2D sand simulation with rich visual effects.

```
┌─────────────────────────────────────────────────────────────┐
│                    Necrosis Engine API                      │
├─────────────────────────────────────────────────────────────┤
│  Simulation Core  │  Rendering System  │  Physics Engine    │
│                   │                    │                    │
│ ┌───────────────┐ │ ┌────────────────┐ │ ┌───────────────┐  │
│ │ Static Grid   │ │ │ BGFX Renderer  │ │ │ Rigid Bodies  │  │
│ │ (Cellular     │ │ │                │ │ │ (Box2D)       │  │
│ │ Automata)     │ │ │ Material       │ │ │               │  │
│ │               │ │ │ Visualization  │ │ │ Collision     │  │
│ │ 64x64 Chunks  │ │ │                │ │ │ Detection     │  │
│ └───────────────┘ │ └────────────────┘ │ └───────────────┘  │
│                   │                    │                    │
│ ┌───────────────┐ │ ┌────────────────┐ │ ┌───────────────┐  │
│ │ Particle      │ │ │ Visual Effects │ │ │ Grid-Physics  │  │
│ │ System        │ │ │ - Gradients    │ │ │ Integration   │  │
│ │               │ │ │ - Borders      │ │ │               │  │
│ │ Velocity-     │ │ │ - Trails       │ │ │ Marching      │  │
│ │ Based Physics │ │ │ - Glow         │ │ │ Squares       │  │
│ └───────────────┘ │ └────────────────┘ │ └───────────────┘  │
└─────────────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│                    Platform Layer                           │
│              SDL (Input/Audio) + BGFX (Graphics)            │
└─────────────────────────────────────────────────────────────┘
```

### 1.2. Core Design Principles

- **Performance First:** 60 FPS at 1920x1080 with ~50% active pixels
- **Visual Richness:** Material color variations, gradients, and interface effects
- **Modular Architecture:** Clear separation between simulation, rendering, and physics
- **Thread Safety:** Checkerboard threading pattern for race-condition-free updates
- **Memory Efficiency:** Object pooling and spatial partitioning for optimal cache usage

---

## 2. Simulation Core Design

### 2.1. Static Grid System (Cellular Automata)

**Data Structure:**
```cpp
struct Pixel {
    uint8_t material_type;     // Material ID (sand, water, wood, etc.)
    uint16_t properties;       // Bit flags for state information (expanded for more states)
    uint16_t temperature;      // Temperature value (0-65535 range for precision)
    uint8_t visual_variant;    // Color/texture variation index
    uint8_t interface_state;   // Border effects with adjacent materials
};

struct Chunk {
    static constexpr int SIZE = 64;
    Pixel pixels[SIZE][SIZE];
    bool is_dirty;
    Rectangle dirty_rect;
    ChunkColor color_type;     // For checkerboard threading
};
```

**Checkerboard Threading Implementation:**
- Chunks are assigned "black" or "white" color based on position: `(chunk_x + chunk_y) % 2`
- Frame N: Update all "black" chunks in parallel
- Frame N+1: Update all "white" chunks in parallel
- Ensures no race conditions as adjacent chunks are always different colors

**Update Algorithm:**
```cpp
void UpdateChunk(Chunk& chunk, const World& world) {
    if (!chunk.is_dirty) return;
    
    for (int y = chunk.dirty_rect.top; y <= chunk.dirty_rect.bottom; ++y) {
        for (int x = chunk.dirty_rect.left; x <= chunk.dirty_rect.right; ++x) {
            Pixel& pixel = chunk.pixels[y][x];
            ApplyMaterialRules(pixel, GetNeighbors(x, y, world));
            UpdateVisualProperties(pixel, GetNeighbors(x, y, world));
        }
    }
    
    chunk.is_dirty = false;
    chunk.dirty_rect = Rectangle::Empty();
}
```

### 2.2. Dynamic Particle System

**Particle Structure:**
```cpp
struct Particle {
    Vector2 position;
    Vector2 velocity;
    uint8_t material_type;
    float lifetime;
    float max_lifetime;
    uint8_t visual_variant;
    ParticleBehavior* behavior;  // Extensible behavior system
};

class ParticleSystem {
    ObjectPool<Particle> particle_pool;
    SpatialHash spatial_hash;
    std::vector<ParticleBehavior*> behaviors;
};
```

**Pixel-to-Particle Conversion (Noita-Inspired):**
```cpp
void ConvertPixelToParticle(int x, int y, Vector2 impulse) {
    Pixel& pixel = GetPixel(x, y);
    if (pixel.material_type == EMPTY) return;
    
    Particle* particle = particle_pool.Acquire();
    particle->position = Vector2(x, y);
    particle->velocity = impulse + GetMaterialVelocity(pixel.material_type);
    particle->material_type = pixel.material_type;
    particle->visual_variant = pixel.visual_variant;
    particle->lifetime = GetMaterialLifetime(pixel.material_type);
    
    // Remove from grid
    pixel = Pixel::Empty();
    MarkChunkDirty(x / 64, y / 64);
    
    spatial_hash.Insert(particle);
}
```

**Particle-to-Pixel Reintegration:**
```cpp
void ReintegrateParticle(Particle* particle) {
    int x = static_cast<int>(particle->position.x);
    int y = static_cast<int>(particle->position.y);
    
    if (IsValidPosition(x, y) && GetPixel(x, y).material_type == EMPTY) {
        Pixel& pixel = GetPixel(x, y);
        pixel.material_type = particle->material_type;
        pixel.visual_variant = particle->visual_variant;
        pixel.properties = 0; // Reset properties
        
        MarkChunkDirty(x / 64, y / 64);
        spatial_hash.Remove(particle);
        particle_pool.Release(particle);
    }
}
```

---

## 3. Physics Integration

### 3.1. Rigid Body Integration Pipeline

**Marching Squares → Box2D Pipeline:**
```cpp
class PixelToPhysicsConverter {
public:
    b2Body* CreateRigidBodyFromPixels(const PixelRegion& region) {
        // Step 1: Extract outline using Marching Squares
        std::vector<Vector2> outline = MarchingSquares(region);
        
        // Step 2: Simplify using Douglas-Peucker
        std::vector<Vector2> simplified = DouglasPeucker(outline, tolerance);
        
        // Step 3: Triangulate polygon
        std::vector<Triangle> triangles = Triangulate(simplified);
        
        // Step 4: Create Box2D body
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        b2Body* body = physics_world->CreateBody(&bodyDef);
        
        for (const auto& triangle : triangles) {
            b2PolygonShape shape;
            shape.Set(triangle.vertices, 3);
            body->CreateFixture(&shape, density);
        }
        
        return body;
    }
};
```

### 3.2. Grid-Physics Interaction

**Collision Detection:**
```cpp
void HandleRigidBodyCollision(b2Body* body, const b2Contact& contact) {
    // Get collision point in world space
    Vector2 collision_point = contact.GetWorldManifold().points[0];
    
    // Convert to grid coordinates
    int grid_x = static_cast<int>(collision_point.x);
    int grid_y = static_cast<int>(collision_point.y);
    
    // Apply deformation to pixel grid
    ApplyDeformation(grid_x, grid_y, body->GetLinearVelocity());
    
    // Spawn particles from displaced pixels
    SpawnParticlesFromDeformation(grid_x, grid_y, body->GetLinearVelocity());
}
```

---

## 4. Rendering System Design

### 4.1. Material Visualization

**Visual Property System:**
```cpp
struct MaterialVisualProperties {
    ColorPalette base_colors;           // Multiple color variations
    ColorPalette interface_colors;      // Border effect colors
    GradientDefinition gradients;       // Core-to-edge transitions
    float transparency;
    bool has_glow_effect;
    float animation_speed;
};

class MaterialRenderer {
    std::unordered_map<uint8_t, MaterialVisualProperties> material_properties;
    
public:
    Color GetPixelColor(const Pixel& pixel, const NeighborInfo& neighbors) {
        const auto& props = material_properties[pixel.material_type];
        
        // Base color with variation
        Color base = props.base_colors[pixel.visual_variant];
        
        // Apply interface effects
        Color interface = CalculateInterfaceColor(pixel, neighbors, props);
        
        // Apply gradients (for fire, etc.)
        Color gradient = ApplyGradient(base, props.gradients, pixel.properties);
        
        return BlendColors(base, interface, gradient);
    }
};
```

### 4.2. BGFX Integration and Texture Management

**Single Large Texture Approach (Recommended):**
Based on research into falling sand simulation rendering and Noita's approach, we use a single large texture for the entire world with region-based updates for optimal performance:

```cpp
class PixelRenderer {
    bgfx::TextureHandle world_texture;        // Single large texture for entire world
    bgfx::UniformHandle material_uniforms;
    uint32_t* texture_buffer;                 // CPU-side texture data
    std::vector<Rectangle> dirty_regions;     // Regions needing GPU upload

public:
    void Initialize(int world_width, int world_height) {
        // Create single large texture
        world_texture = bgfx::createTexture2D(
            world_width, world_height, false, 1,
            bgfx::TextureFormat::RGBA8,
            BGFX_TEXTURE_NONE | BGFX_SAMPLER_POINT
        );

        texture_buffer = new uint32_t[world_width * world_height];
    }

    void UpdateChunk(const Chunk& chunk, int chunk_x, int chunk_y) {
        if (!chunk.is_dirty) return;

        // Update CPU texture buffer for this chunk
        UpdateTextureRegion(chunk, chunk_x, chunk_y);

        // Mark region for GPU upload
        Rectangle dirty_rect = {
            chunk_x * Chunk::SIZE,
            chunk_y * Chunk::SIZE,
            Chunk::SIZE,
            Chunk::SIZE
        };
        dirty_regions.push_back(dirty_rect);
    }

    void FlushToGPU() {
        // Batch upload all dirty regions to GPU
        for (const auto& region : dirty_regions) {
            bgfx::updateTexture2D(
                world_texture, 0, 0,
                region.x, region.y, region.width, region.height,
                bgfx::makeRef(&texture_buffer[region.y * world_width + region.x],
                             region.width * region.height * sizeof(uint32_t))
            );
        }
        dirty_regions.clear();
    }

    void Render() {
        // Single draw call for entire world
        bgfx::setTexture(0, material_uniforms, world_texture);
        bgfx::submit(view_id, shader_program);
    }

private:
    void UpdateTextureRegion(const Chunk& chunk, int chunk_x, int chunk_y) {
        int base_x = chunk_x * Chunk::SIZE;
        int base_y = chunk_y * Chunk::SIZE;

        for (int y = 0; y < Chunk::SIZE; ++y) {
            for (int x = 0; x < Chunk::SIZE; ++x) {
                const Pixel& pixel = chunk.pixels[y][x];
                uint32_t color = MaterialRenderer::GetPixelColor(pixel);

                int texture_x = base_x + x;
                int texture_y = base_y + y;
                texture_buffer[texture_y * world_width + texture_x] = color;
            }
        }
    }
};
```

**Alternative: Per-Chunk Texture Approach (For Comparison):**
While less efficient for most cases, per-chunk textures might be useful for very large worlds with sparse activity:

```cpp
class ChunkedPixelRenderer {
    std::unordered_map<ChunkCoord, bgfx::TextureHandle> chunk_textures;

    // Less efficient due to multiple draw calls and texture switches
    // Only consider for worlds larger than GPU texture limits
};
```

**Rendering Performance Considerations:**
- **Single texture benefits:** One draw call, minimal state changes, better batching
- **Memory efficiency:** Contiguous memory layout improves cache performance
- **Update efficiency:** Region-based updates minimize GPU bandwidth usage
- **Scalability:** Works well up to GPU texture size limits (typically 16K x 16K)

---

## 5. Performance Optimization

### 5.1. Memory Layout

**Cache-Friendly Chunk Organization:**
```cpp
class World {
    static constexpr int CHUNK_SIZE = 64;
    static constexpr int WORLD_WIDTH_CHUNKS = 30;  // 1920 / 64
    static constexpr int WORLD_HEIGHT_CHUNKS = 17; // 1080 / 64
    
    // Contiguous memory layout for cache efficiency
    std::array<Chunk, WORLD_WIDTH_CHUNKS * WORLD_HEIGHT_CHUNKS> chunks;
    
    Chunk& GetChunk(int chunk_x, int chunk_y) {
        return chunks[chunk_y * WORLD_WIDTH_CHUNKS + chunk_x];
    }
};
```

### 5.2. Threading Architecture

**Thread Pool Implementation:**
```cpp
class SimulationThreadPool {
    std::vector<std::thread> workers;
    ThreadSafeQueue<ChunkUpdateTask> task_queue;
    
public:
    void UpdateFrame(World& world, bool update_black_chunks) {
        // Queue chunk update tasks
        for (int y = 0; y < WORLD_HEIGHT_CHUNKS; ++y) {
            for (int x = 0; x < WORLD_WIDTH_CHUNKS; ++x) {
                bool is_black_chunk = (x + y) % 2 == 0;
                if (is_black_chunk == update_black_chunks) {
                    task_queue.Push({&world.GetChunk(x, y), &world});
                }
            }
        }
        
        // Wait for completion
        WaitForAllTasks();
    }
};
```

---

## 6. State Management Framework

### 6.1. Lightweight Material State System (Noita-Inspired)

Based on research into Noita's implementation, the engine uses a lightweight approach to material state management rather than complex per-pixel state machines. Our system follows this proven methodology:

**Material Type + Property Flags Approach:**
```cpp
struct Pixel {
    uint8_t material_type;     // Base material ID (wood, water, sand, etc.)
    uint16_t properties;       // Bit flags for state information
    uint16_t temperature;      // Temperature value (0-65535 range for precision)
    uint8_t visual_variant;    // Color/texture variation index
};

// Property bit flags for efficient state tracking
enum MaterialProperties : uint16_t {
    PROP_WET        = 1 << 0,   // Material is wet
    PROP_BURNING    = 1 << 1,   // Material is on fire
    PROP_HOT        = 1 << 2,   // Material is heated
    PROP_FROZEN     = 1 << 3,   // Material is frozen
    PROP_DIRTY      = 1 << 4,   // Needs visual update
    PROP_MOVING     = 1 << 5,   // Currently in motion
    PROP_FLAMMABLE  = 1 << 6,   // Can catch fire
    PROP_LIQUID     = 1 << 7,   // Flows like liquid
    PROP_GAS        = 1 << 8,   // Behaves like gas
    PROP_SOLID      = 1 << 9,   // Solid material
    // Additional flags as needed...
};
```

**Material State Transitions via Rules:**
```cpp
class MaterialRules {
public:
    static void UpdatePixel(Pixel& pixel, const NeighborContext& neighbors, float delta_time) {
        switch (pixel.material_type) {
            case MATERIAL_WOOD:
                UpdateWoodState(pixel, neighbors, delta_time);
                break;
            case MATERIAL_WATER:
                UpdateWaterState(pixel, neighbors, delta_time);
                break;
            // ... other materials
        }
    }

private:
    static void UpdateWoodState(Pixel& pixel, const NeighborContext& neighbors, float delta_time) {
        // Check for ignition from neighboring fire
        if (HasBurningNeighbor(neighbors) && !(pixel.properties & PROP_WET)) {
            pixel.properties |= PROP_BURNING;
            pixel.temperature = 200; // Ignition temperature
        }

        // Burning wood transitions
        if (pixel.properties & PROP_BURNING) {
            pixel.temperature += 10 * delta_time;

            // Transition to charcoal after burning time
            if (pixel.temperature > 250) {
                pixel.material_type = MATERIAL_CHARCOAL;
                pixel.properties &= ~PROP_BURNING;
                pixel.temperature = 150;
            }
        }

        // Water extinguishes fire
        if ((pixel.properties & PROP_BURNING) && HasWaterNeighbor(neighbors)) {
            pixel.properties &= ~PROP_BURNING;
            pixel.properties |= PROP_WET;
            pixel.temperature = 20;
        }
    }
};
```

### 6.2. Pooled State Machines for Complex Behaviors

For materials requiring complex state management, use pooled state machines for active regions only:

```cpp
class RegionalStateManager {
    ObjectPool<MaterialStateMachine> state_machine_pool;
    std::unordered_map<ChunkCoord, MaterialStateMachine*> active_regions;

public:
    void RegisterActiveRegion(ChunkCoord coord) {
        if (active_regions.find(coord) == active_regions.end()) {
            auto* state_machine = state_machine_pool.Acquire();
            state_machine->Initialize(coord);
            active_regions[coord] = state_machine;
        }
    }

    void UpdateActiveRegions(float delta_time) {
        for (auto& [coord, state_machine] : active_regions) {
            state_machine->Update(delta_time);

            // Return to pool if region becomes inactive
            if (!state_machine->HasActiveStates()) {
                state_machine_pool.Release(state_machine);
                active_regions.erase(coord);
            }
        }
    }
};
```

### 6.3. Engine State Management

**Simple Engine States:**
```cpp
enum class EngineState {
    INITIALIZING,
    RUNNING,
    PAUSED,
    LOADING,
    ERROR,
    SHUTTING_DOWN
};

class EngineStateManager {
    EngineState current_state = EngineState::INITIALIZING;

public:
    void TransitionTo(EngineState new_state) {
        OnStateExit(current_state);
        current_state = new_state;
        OnStateEnter(new_state);
    }

private:
    void OnStateEnter(EngineState state);
    void OnStateExit(EngineState state);
};
```

---

## 7. Extensibility Framework

### 6.1. Material Definition System

**Data-Driven Material Properties:**
```json
{
    "materials": {
        "sand": {
            "id": 1,
            "density": 1.5,
            "friction": 0.8,
            "color_variants": ["#D2B48C", "#F4A460", "#DEB887", "#BC9A6A"],
            "interface_effects": {
                "water": {"darken": 0.3, "saturation": 1.2}
            },
            "cellular_automata_rules": "gravity_fall",
            "particle_conversion": {
                "impulse_threshold": 5.0,
                "lifetime": 2.0
            }
        }
    }
}
```

### 6.2. Particle Behavior System

**Extensible Behavior Framework:**
```cpp
class ParticleBehavior {
public:
    virtual void Update(Particle& particle, float delta_time) = 0;
    virtual void OnCollision(Particle& particle, const CollisionInfo& info) = 0;
    virtual bool ShouldReintegrate(const Particle& particle) = 0;
};

class SplashBehavior : public ParticleBehavior {
public:
    void Update(Particle& particle, float delta_time) override {
        // Apply splash-specific physics
        particle.velocity.y += gravity * delta_time;
        particle.position += particle.velocity * delta_time;
        
        // Fade over time
        particle.lifetime -= delta_time;
    }
    
    bool ShouldReintegrate(const Particle& particle) override {
        return particle.velocity.Length() < reintegration_threshold;
    }
};
```

### 6.3. State Machine Integration

**Material State Definition:**
```json
{
    "material_states": {
        "wood": {
            "states": ["solid", "burning", "charcoal", "ash"],
            "transitions": {
                "solid": {"ignite": "burning"},
                "burning": {"burn_complete": "charcoal", "extinguish": "solid"},
                "charcoal": {"ignite": "burning", "decay": "ash"}
            },
            "timers": {
                "burning": {"duration": 5.0, "auto_transition": "charcoal"}
            }
        },
        "water": {
            "states": ["liquid", "steam", "ice"],
            "transitions": {
                "liquid": {"heat_up": "steam", "cool_down": "ice"},
                "steam": {"cool_down": "liquid"},
                "ice": {"heat_up": "liquid"}
            },
            "temperature_thresholds": {
                "steam_point": 100.0,
                "freeze_point": 0.0
            }
        }
    }
}
```

**Particle State Integration:**
```cpp
enum class ParticleState {
    SPAWNING,
    ACTIVE,
    FADING,
    EXTINGUISHED
};

struct ParticleStateMachine {
    StateMachine<ParticleState, ParticleEvent> state_machine;
    float state_duration;
    float fade_rate;

    void Initialize() {
        // Define particle lifecycle transitions
        state_machine.RegisterTransition(ParticleState::SPAWNING, ParticleEvent::SPAWN_COMPLETE, ParticleState::ACTIVE);
        state_machine.RegisterTransition(ParticleState::ACTIVE, ParticleEvent::BEGIN_FADE, ParticleState::FADING);
        state_machine.RegisterTransition(ParticleState::FADING, ParticleEvent::FADE_COMPLETE, ParticleState::EXTINGUISHED);

        // State callbacks for visual effects
        state_machine.RegisterStateCallback(ParticleState::FADING,
            [this]() { fade_rate = 0.5f; },  // on_enter
            [this]() { fade_rate = 0.0f; }   // on_exit
        );
    }
};
```

---

## 7. Debug and Development Tools

### 7.1. Performance Monitoring Panel
```cpp
class PerformanceMonitor {
    struct FrameStats {
        float fps = 0.0f;
        uint32_t active_pixel_count = 0;
        uint32_t moving_pixel_count = 0;
        uint32_t particle_count = 0;
        uint32_t active_chunk_count = 0;
        size_t memory_usage_mb = 0;
    } stats;

public:
    void RenderUI() {
        if (ImGui::Begin("Performance Monitor")) {
            ImGui::Text("FPS: %.1f", stats.fps);
            ImGui::Text("Active Pixels: %u", stats.active_pixel_count);
            ImGui::Text("Moving Pixels: %u", stats.moving_pixel_count);
            ImGui::Text("Particles: %u", stats.particle_count);
            ImGui::Text("Active Chunks: %u", stats.active_chunk_count);
            ImGui::Text("Memory Usage: %zu MB", stats.memory_usage_mb);
        }
        ImGui::End();
    }
};
```

### 7.2. Chunk Visualization System
```cpp
class ChunkVisualizer {
    enum class VisualizationMode {
        BOUNDARIES,           // Show chunk boundaries
        ACTIVITY_STATUS,      // Red=active, Blue=static, Green=empty
        DIRTY_REGIONS,        // Yellow=dirty chunks
        THREAD_ASSIGNMENT     // Checkerboard pattern for threading
    } mode = VisualizationMode::BOUNDARIES;

public:
    void RenderChunkOverlay(const World& world) {
        for (int y = 0; y < world.GetChunkCountY(); ++y) {
            for (int x = 0; x < world.GetChunkCountX(); ++x) {
                const Chunk& chunk = world.GetChunk(x, y);
                Color overlay_color = GetChunkOverlayColor(chunk, mode);
                RenderChunkOutline(x * 64, y * 64, 64, 64, overlay_color);
            }
        }
    }
};
```

### 7.3. Pixel Debug Overlay
```cpp
class PixelDebugOverlay {
    enum class PixelDebugMode {
        SIMULATION_STATE,     // Blue=static, Red=active/simulated
        TEMPERATURE,          // Heat visualization
        MATERIAL_PROPERTIES   // Color-coded property flags
    } mode = PixelDebugMode::SIMULATION_STATE;

public:
    void RenderPixelOverlay(const World& world) {
        // Color-code pixels based on debug mode
        for (int y = 0; y < world.GetHeight(); ++y) {
            for (int x = 0; x < world.GetWidth(); ++x) {
                const Pixel& pixel = world.GetPixel(x, y);
                Color debug_color = GetPixelDebugColor(pixel, mode);
                if (debug_color.a > 0) {
                    RenderPixel(x, y, debug_color);
                }
            }
        }
    }
};
```

### 7.4. Frame Stepping and Simulation Control
```cpp
class SimulationController {
    bool is_paused = false;
    bool step_single_frame = false;
    float simulation_speed = 1.0f;

public:
    void RenderUI() {
        if (ImGui::Begin("Simulation Control")) {
            if (ImGui::Button(is_paused ? "Resume" : "Pause")) {
                is_paused = !is_paused;
            }
            ImGui::SameLine();
            if (ImGui::Button("Step Frame")) {
                step_single_frame = true;
            }
            ImGui::SliderFloat("Speed", &simulation_speed, 0.1f, 5.0f);
        }
        ImGui::End();
    }

    bool ShouldUpdate() const { return !is_paused || step_single_frame; }
    float GetDeltaTime(float real_delta_time) {
        if (step_single_frame) {
            step_single_frame = false;
            return 1.0f / 60.0f; // Fixed timestep for stepping
        }
        return real_delta_time * simulation_speed;
    }
};
```

### 7.5. Individual Pixel Inspector
```cpp
class PixelInspector {
    Vector2 mouse_position;
    bool show_inspector = false;

public:
    void RenderUI(const World& world) {
        if (!show_inspector) return;

        int pixel_x = static_cast<int>(mouse_position.x);
        int pixel_y = static_cast<int>(mouse_position.y);

        if (world.IsValidPosition(pixel_x, pixel_y)) {
            const Pixel& pixel = world.GetPixel(pixel_x, pixel_y);

            ImGui::Begin("Pixel Inspector");
            ImGui::Text("Position: (%d, %d)", pixel_x, pixel_y);
            ImGui::Text("Material: %s", GetMaterialName(pixel.material_type));
            ImGui::Text("Temperature: %d", pixel.temperature);
            ImGui::Text("Properties: 0x%04X", pixel.properties);

            // Property flags breakdown
            if (pixel.properties & PROP_WET) ImGui::Text("  - Wet");
            if (pixel.properties & PROP_BURNING) ImGui::Text("  - Burning");
            if (pixel.properties & PROP_MOVING) ImGui::Text("  - Moving");
            ImGui::End();
        }
    }
};
```

### 7.6. Material Brush and Testing Tools
```cpp
class DevelopmentTools {
    MaterialType selected_material = MATERIAL_SAND;
    int brush_size = 5;

public:
    void RenderUI() {
        if (ImGui::Begin("Development Tools")) {
            // Material brush
            const char* materials[] = {"Sand", "Water", "Wood", "Fire"};
            int current = static_cast<int>(selected_material);
            ImGui::Combo("Material", &current, materials, 4);
            selected_material = static_cast<MaterialType>(current);

            ImGui::SliderInt("Brush Size", &brush_size, 1, 20);

            // Testing scenarios
            if (ImGui::Button("Load Sand Test")) LoadTestScenario("sand_pile");
            if (ImGui::Button("Load Water Test")) LoadTestScenario("water_flow");
            if (ImGui::Button("Load Fire Test")) LoadTestScenario("fire_spread");
        }
        ImGui::End();
    }
};
```

---

## 8. Threading Architecture and Incremental Development

### 8.1. Threading Implementation Strategy

**Incremental Threading Approach:**
Following Noita's incremental development philosophy, threading can be implemented progressively while maintaining the full architecture design:

```cpp
class ThreadingManager {
    enum class ThreadingMode {
        SINGLE_THREADED,    // Initial implementation
        CHECKERBOARD,       // Noita-inspired threading
        ADVANCED_PARALLEL   // Future optimizations
    } current_mode = ThreadingMode::SINGLE_THREADED;

    ThreadPool thread_pool;

public:
    void UpdateWorld(World& world, float delta_time) {
        switch (current_mode) {
            case ThreadingMode::SINGLE_THREADED:
                UpdateWorldSingleThreaded(world, delta_time);
                break;
            case ThreadingMode::CHECKERBOARD:
                UpdateWorldCheckerboard(world, delta_time);
                break;
        }
    }

private:
    void UpdateWorldSingleThreaded(World& world, float delta_time) {
        // Simple sequential chunk updates for initial development
        for (auto& chunk : world.GetAllChunks()) {
            if (chunk.is_dirty) {
                UpdateChunk(chunk, world);
            }
        }
    }

    void UpdateWorldCheckerboard(World& world, float delta_time) {
        // Noita-inspired checkerboard pattern
        bool update_black_chunks = (world.GetFrameCount() % 2) == 0;

        std::vector<std::future<void>> futures;
        for (auto& chunk : world.GetAllChunks()) {
            bool is_black_chunk = (chunk.coord.x + chunk.coord.y) % 2 == 0;
            if (is_black_chunk == update_black_chunks && chunk.is_dirty) {
                futures.push_back(thread_pool.enqueue([&chunk, &world]() {
                    UpdateChunk(chunk, world);
                }));
            }
        }

        // Wait for all chunks to complete
        for (auto& future : futures) {
            future.wait();
        }
    }
};
```

**Benefits of Incremental Threading:**
- Start with simple, debuggable single-threaded implementation
- Add threading when core simulation is stable
- Avoid premature optimization while maintaining scalable architecture
- Easier debugging and profiling of core algorithms

### 8.2. Rendering Approach Decision

**Direct BGFX Integration (Recommended):**
Based on research into falling sand simulation rendering and implementation complexity trade-offs, proceed directly with BGFX integration rather than simple pixel manipulation:

```cpp
// BGFX integration is straightforward for pixel simulation
class SimplePixelRenderer {
    bgfx::TextureHandle world_texture;
    uint32_t* pixel_buffer;

public:
    void Initialize(int width, int height) {
        world_texture = bgfx::createTexture2D(width, height, false, 1,
                                            bgfx::TextureFormat::RGBA8);
        pixel_buffer = new uint32_t[width * height];
    }

    void UpdatePixel(int x, int y, uint32_t color) {
        pixel_buffer[y * width + x] = color;
    }

    void FlushToGPU() {
        bgfx::updateTexture2D(world_texture, 0, 0, 0, 0, width, height,
                             bgfx::makeRef(pixel_buffer, width * height * 4));
    }
};
```

---

## 9. Implementation Phases

### 9.1. Phase 1: Core Foundation (Weeks 1-4)
**Deliverables:**
- Basic chunk-based world representation with 64x64 chunks
- Simple cellular automata for sand and water
- Single-threaded implementation with dirty rectangle tracking
- Basic BGFX integration for pixel rendering
- Material color variation system

**Success Criteria:**
- Sand falls and accumulates realistically
- Water flows and seeks lowest points
- Basic color variations visible for both materials
- Stable simulation at target resolution

### 9.2. Phase 2: Multi-threading & Performance (Weeks 5-8)
**Deliverables:**
- Checkerboard threading implementation
- Thread pool for chunk processing
- Performance profiling and optimization
- Memory layout optimization for cache efficiency
- Dirty rectangle optimization refinement
- Basic material state transitions (wood burning)

**Success Criteria:**
- Efficient multi-core CPU utilization
- No race conditions or threading artifacts
- Memory usage within target limits
- Material state transitions working smoothly

### 9.3. Phase 3: Particle System (Weeks 9-12)
**Deliverables:**
- Pixel-to-particle conversion system
- Particle-to-pixel reintegration
- Basic particle behaviors (splash, debris)
- Spatial partitioning for particles
- Object pooling for memory management

**Success Criteria:**
- Smooth pixel-particle transitions
- Realistic particle physics and collisions
- No memory leaks or fragmentation

### 9.4. Phase 4: Rigid Body Integration (Weeks 13-16)
**Deliverables:**
- Marching Squares implementation
- Douglas-Peucker polygon simplification
- Box2D integration pipeline
- Grid-physics collision handling
- Deformation and particle spawning

**Success Criteria:**
- Rigid bodies interact realistically with pixel grid
- Deformation creates appropriate particle effects
- Stable physics simulation

---

## 10. Testing Strategy

### 10.1. Unit Testing
**Core Systems:**
- Chunk update algorithms
- Pixel-particle conversion logic
- Material interaction rules
- Threading synchronization
- Memory pool management
- State machine transitions and timing
- Material state lifecycle validation
- Particle state progression

**Testing Framework:** Google Test (gtest) for C++ unit testing

### 10.2. Performance Testing
**Benchmarks:**
- Frame rate under various pixel densities
- Memory usage patterns
- Thread utilization efficiency
- Cache miss rates
- Particle system scalability

**Tools:**
- Custom profiling framework
- Platform-specific profilers (Intel VTune, Visual Studio Profiler)
- Memory leak detection (Valgrind, Application Verifier)

### 10.3. Integration Testing
**Scenarios:**
- Large-scale simulations with mixed materials
- Stress testing with maximum particle counts
- Long-running stability tests
- Cross-platform compatibility verification

---

## 11. API Design

### 11.1. Core Engine Interface
```cpp
namespace Necrosis {
    class Engine {
    public:
        // Initialization
        bool Initialize(const EngineConfig& config);
        void Shutdown();

        // Simulation control
        void Update(float delta_time);
        void Render();
        void Pause();
        void Resume();

        // World manipulation
        void SetPixel(int x, int y, MaterialType material);
        MaterialType GetPixel(int x, int y) const;
        void ApplyImpulse(int x, int y, Vector2 force);

        // Rigid body management
        RigidBodyHandle CreateRigidBody(const RigidBodyDef& def);
        void DestroyRigidBody(RigidBodyHandle handle);

        // Material system
        void RegisterMaterial(const MaterialDefinition& material);
        void LoadMaterialsFromFile(const std::string& filepath);

        // State management
        void TriggerMaterialEvent(int x, int y, MaterialEvent event);
        MaterialState GetMaterialState(int x, int y) const;
        void LoadStateDefinitionsFromFile(const std::string& filepath);

        // Events
        void SetCollisionCallback(CollisionCallback callback);
        void SetParticleCallback(ParticleCallback callback);
        void SetStateChangeCallback(StateChangeCallback callback);
    };
}
```

### 11.2. Configuration System
```cpp
struct EngineConfig {
    // World settings
    int world_width = 1920;
    int world_height = 1080;
    int chunk_size = 64;

    // Performance settings
    int max_particles = 50000;
    int thread_count = 0; // 0 = auto-detect
    bool enable_dirty_rects = true;

    // Rendering settings
    bool enable_visual_effects = true;
    float particle_lod_distance = 500.0f;

    // Physics settings
    float gravity = 9.81f;
    int physics_iterations = 8;
    bool enable_rigid_bodies = true;
};
```

---

## 12. Future Enhancements

### 12.1. Advanced Material Systems
- **Heat propagation:** Temperature-based material state changes
- **Chemical reactions:** Complex material interactions and transformations
- **Electrical conductivity:** Current flow through conductive materials
- **Pressure dynamics:** Compression and expansion effects

### 12.2. GPU Acceleration
- **Compute shaders:** Massive particle simulation on GPU
- **Parallel cellular automata:** GPU-based grid updates
- **Advanced visual effects:** Shader-based material rendering

### 12.3. Networking Support
- **Deterministic simulation:** Synchronized multi-player physics
- **State compression:** Efficient network transmission of world state
- **Rollback networking:** Client-side prediction with server reconciliation

---

This TDD provides the foundational technical architecture for the Necrosis Engine, incorporating proven techniques from Noita while maintaining our visual fidelity and modularity goals. The document will continue to evolve as we implement and refine each system.
