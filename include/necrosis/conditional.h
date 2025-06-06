#pragma once

#include <type_traits>
#include <utility>

// Necrosis Engine Conditional Compilation System
// 
// This header provides clean template-based conditional compilation that completely
// eliminates disabled code paths at compile time without cluttering source code
// with preprocessor directives.
//
// Usage examples:
//   
//   // Simple conditional call
//   conditional<features::cheat_system_enabled>::call([]() {
//       CheatSystem::ProcessCommand(input);
//   });
//
//   // Conditional with return value
//   auto result = conditional<features::debug_overlay_enabled>::call_with_return([]() {
//       return DebugOverlay::GetMetrics();
//   }, DefaultMetrics{});
//
//   // Conditional class instantiation
//   auto system = conditional<features::cheat_system_enabled>::create<CheatSystem>();

namespace necrosis {

// Primary template for conditional compilation
template<bool Enabled>
struct conditional {
    
    // Execute a function only if the feature is enabled
    // When Enabled is false, this entire call is eliminated by the compiler
    template<typename Func>
    static constexpr void call(Func&& func) {
        if constexpr (Enabled) {
            std::forward<Func>(func)();
        }
    }
    
    // Execute a function and return its result, or return a default value
    template<typename Func, typename DefaultType>
    static constexpr auto call_with_return(Func&& func, DefaultType&& default_value) {
        if constexpr (Enabled) {
            return std::forward<Func>(func)();
        } else {
            return std::forward<DefaultType>(default_value);
        }
    }
    
    // Create an object only if the feature is enabled
    // Returns std::optional<T> or similar wrapper
    template<typename T, typename... Args>
    static constexpr auto create(Args&&... args) {
        if constexpr (Enabled) {
            return T{std::forward<Args>(args)...};
        } else {
            // Return a "null" object that can be safely ignored
            return typename std::conditional_t<Enabled, T, empty_type>{};
        }
    }
    
    // Check if feature is enabled (compile-time constant)
    static constexpr bool enabled() {
        return Enabled;
    }

private:
    // Empty type for disabled features
    struct empty_type {
        // Make it safely callable/assignable but do nothing
        template<typename... Args>
        constexpr empty_type(Args&&...) {}
        
        template<typename... Args>
        constexpr void operator()(Args&&...) const {}
        
        template<typename T>
        constexpr empty_type& operator=(T&&) { return *this; }
    };
};

// Convenience aliases for common patterns
template<bool Enabled>
using when = conditional<Enabled>;

// Macro-like helpers for cleaner syntax (optional)
#define NECROSIS_WHEN(feature) necrosis::conditional<necrosis::features::feature##_enabled>

// Feature-specific conditional types (will be available after features.h is generated)
// These provide even cleaner syntax for specific features

} // namespace necrosis
