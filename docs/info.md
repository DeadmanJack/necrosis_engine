Please help me create a comprehensive Product Requirements Document (PRD) for "Necrosis," a 2D sand simulation engine I'm developing in C++. The PRD should include:

1. **Project Overview**
   - Core concept: A modular, multi-threaded, highly optimized 2D sand simulation engine
   - Initial pixel types: sand, water, wood, and fire, with each having unique physical properties
   - Support for rigid bodies and character entities
   - Extensible architecture allowing easy addition of new pixel/item types

2. **Technical Requirements**
   - Base implementation using C++ with SDL for input/audio handling
        - Alternative suggestions for input/audio handling beyond SDL, if applicable
   - Graphics rendering via a framework like The-Forge or BGFX
        - Alternative suggestions for graphics rendering frameworks beyond The-Forge and BGFX, if applicable
   - Initial support for OpenGL/DirectX backends with planned expansion to Vulkan and Metal
   - This will eventually be turned into a game so I want this engine to be separate from the game codebase. I want to be able to use this engine for other projects in the future.
   

3. **Core Features**
   - Pixel-level simulation mechanics
   - Physics interactions between different material types
   - Rigid body dynamics
   - Character movement and interaction
   - Modular design for extensibility

4. **Development Approach**
   - Continuous Deployment pipeline to maintain an "always working" product
   - Frequent iteration cycles
   - Complementary Technical Design Document (TDD) to follow

5. **Performance Requirements**
   - Multi-threading approach
   - Optimization targets
   - Minimum hardware specifications

Please structure the PRD in a clear, organized format that can guide development while allowing for future expansion of the engine's capabilities.