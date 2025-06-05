### State Machine Libraries for Necrosis Engine
For the Necrosis Engine project (a high-performance 2D sand simulation engine built in C++ with SDL for input/audio and bgfx for graphics), I need to evaluate open source state machine libraries that would be suitable for managing:

1. **Material state transitions** (e.g., wood → burning wood → charcoal → ash, water → steam, etc.)
2. **Particle lifecycle states** (e.g., cinder creation → glowing → fading → extinguished)
3. **Game/engine states** (e.g., simulation running, paused, loading, etc.)
4. **Character interaction states** if applicable

Please provide recommendations for open source C++ state machine libraries that meet these criteria:
- **Performance**: Suitable for real-time simulation at 60 FPS with potentially thousands of state transitions per frame
- **Memory efficiency**: Low overhead per state machine instance
- **Integration**: Compatible with modern C++ (C++17+) and works well with SDL/bgfx architecture
- **Licensing**: Permissive licenses compatible with commercial use
- **Ease of use**: Reasonable learning curve for implementation
- **Scalability**: Can handle both simple material transitions and complex hierarchical states

For each recommendation, please include:
- Library name and brief description
- Performance characteristics relevant to real-time simulation
- Integration complexity with the existing tech stack
- Licensing information
- Any specific advantages for cellular automata or particle system state management

### <Placeholder System Heading>