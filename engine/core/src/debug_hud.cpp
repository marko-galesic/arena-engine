#include "arena/debug_hud.hpp"
#include "../../../src/app/Clock.hpp"
#include "arena/input.hpp"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <cstdio>
#include <sstream>
#include <iomanip>

// Prevent GLFW from pulling in legacy OpenGL headers
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif

namespace arena {

void DebugHud::draw(const Clock& clock, const InputState& input) {
    // Set window position and size
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200, 120), ImGuiCond_FirstUseEver);
    
    // Begin the debug HUD window
    if (ImGui::Begin("Debug HUD", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        // Calculate FPS
        double fps = clock.lastDt > 0.0 ? 1.0 / clock.lastDt : 0.0;
        
        // Display FPS
        ImGui::Text("FPS: %.1f", fps);
        
        // Display ms/frame
        ImGui::Text("ms/frame: %.2f", clock.lastDt * 1000.0);
        
        // Display tick count
        ImGui::Text("Ticks: %llu", static_cast<unsigned long long>(clock.ticks));
        
        // Display WASD pressed flags
        ImGui::Separator();
        ImGui::Text("WASD: %s %s %s %s",
            isKeyPressed(input, GLFW_KEY_W) ? "W" : "-",
            isKeyPressed(input, GLFW_KEY_A) ? "A" : "-",
            isKeyPressed(input, GLFW_KEY_S) ? "S" : "-",
            isKeyPressed(input, GLFW_KEY_D) ? "D" : "-"
        );
        
        // Display mouse movement
        ImGui::Text("Mouse: dx=%.2f dy=%.2f", input.mouseDx, input.mouseDy);
    }
    
    ImGui::End();
}

bool DebugHud::isKeyPressed(const InputState& input, int key) {
    return key >= 0 && key < 512 && input.keys[key];
}

} // namespace arena
