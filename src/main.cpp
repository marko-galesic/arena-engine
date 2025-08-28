#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <vector>

// OpenGL API header - must be included first
#include "arena/gl_api.hpp"

#include "app/Clock.hpp"
#include "app/Config.hpp"
#include "arena/input.hpp"
#include "arena/ecs/registry.hpp"
#include "arena/ecs/camera_system.hpp"
#include "arena/text.hpp"
#include "arena/gfx/gl_context.hpp"
#include "arena/gfx/shader.hpp"
#include "arena/gfx/mesh.hpp"
#include "arena/sun_lighting.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// High-resolution timer wrapper using standard C++ chrono
static double NowSeconds() {
    static auto start = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start);
    return duration.count() / 1e9;
}

// Simple logging macro with timestamps
#define LOG(msg) std::cout << "[" << NowSeconds() << "] " << msg << std::endl

// Global input state
static arena::InputState g_inputState;

// Global ECS registry and camera system
static arena::ecs::Registry g_registry;
static arena::ecs::CameraSystem g_cameraSystem;
static arena::ecs::Entity g_cameraEntityId = 0;

// Global sun lighting system
static arena::SunLighting g_sunLighting;

// Input state accessor
arena::InputState& getInputState() { return g_inputState; }

// GLFW callback functions (these will be set up by the GL context)
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    arena::handleKey(g_inputState, key, action);
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    static double lastX = 0.0, lastY = 0.0;
    static bool firstMove = true;
    
    if (firstMove) {
        lastX = xpos;
        lastY = ypos;
        firstMove = false;
        std::cout << "First mouse position: " << xpos << ", " << ypos << std::endl;
        return;
    }
    
    double dx = xpos - lastX;
    double dy = ypos - lastY;
    
    // Debug: Always log mouse movement to see if callback is working
    std::cout << "Mouse callback: pos(" << xpos << ", " << ypos << ") delta(" << dx << ", " << dy << ")" << std::endl;
    
    arena::handleMouseMove(g_inputState, dx, dy);
    
    lastX = xpos;
    lastY = ypos;
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    arena::handleMouseButton(g_inputState, button, action);
}



// Simple command line argument parser
struct Args {
    bool server = false;
    std::string configPath = "config/engine.ini";
    int runForMs = -1; // -1 means run indefinitely
    
    void parse(int argc, char* argv[]) {
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            
            if (arg == "--server") {
                server = true;
            } else if (arg.substr(0, 9) == "--config=") {
                configPath = arg.substr(9);
            } else if (arg.substr(0, 11) == "--runForMs=") {
                try {
                    runForMs = std::stoi(arg.substr(11));
                } catch (...) {
                    std::cout << "Warning: Invalid --runForMs value, ignoring" << std::endl;
                }
            } else if (arg == "--help" || arg == "-h") {
                std::cout << "Arena Engine\n";
                std::cout << "Usage: arena [options]\n";
                std::cout << "Options:\n";
                std::cout << "  --server              Run in headless mode (no window)\n";
                std::cout << "  --config=<path>       Load configuration from file\n";
                std::cout << "  --runForMs=<ms>       Run for specified milliseconds then exit\n";
                std::cout << "  --help, -h            Show this help message\n";
                exit(0);
            }
        }
    }
};

int main(int argc, char* argv[]) {
    // Initialize timing first
    double initTime = NowSeconds();
    
    Args args;
    args.parse(argc, argv);
    
    LOG("Starting Arena Engine");
    
    // Load configuration first
    Config config;
    if (config.loadFromFile(args.configPath)) {
        LOG("Loaded config from: " << args.configPath);
        LOG("Tick rate: " << config.tick_hz << " Hz");
        LOG("Window size: " << config.window_w << "x" << config.window_h);
    } else {
        LOG("Warning: Could not load config from " << args.configPath << ", using defaults");
    }
    
        arena::gfx::GLContext glContext;
    arena::gfx::Shader basicShader;
    arena::gfx::Mesh gridMesh;
    arena::gfx::Mesh coordinateAxesMesh;
    arena::gfx::Mesh centerCubeMesh;
    
    if (args.server) {
        LOG("Running in HEADLESS mode (--server)");
    } else {
        LOG("Running in WINDOWED mode");
        
        // Initialize GL context with OpenGL 4.5 core profile and sRGB
        if (!glContext.initialize(config.window_w, config.window_h, "Arena Engine")) {
            LOG("ERROR: Failed to initialize GL context");
            return -1;
        }
        
        // Set input callbacks
        glContext.setKeyCallback(keyCallback);
        glContext.setCursorPosCallback(cursorPosCallback);
        glContext.setMouseButtonCallback(mouseButtonCallback);
        
        // Load basic shader (with lighting support)
        if (!basicShader.load("assets/shaders/basic.vert", "assets/shaders/basic.frag")) {
            LOG("ERROR: Failed to load basic shader");
            return -1;
        }
        LOG("Basic shader loaded successfully");
        
        // Create grid mesh
        gridMesh = arena::gfx::Mesh::makeGrid(16, 1.0f); // 16 units radius, 1.0f cell size (but we override spacing)
        LOG("Grid mesh created successfully");
        
        // Create coordinate axes mesh with thicker X and Z axes
        coordinateAxesMesh = arena::gfx::Mesh::makeCoordinateAxes(5.0f, 0.15f); // 5 units long, thicker axes
        LOG("Coordinate axes mesh created successfully");
        
        // Create center cube mesh (3x3x3 units)
        centerCubeMesh = arena::gfx::Mesh::makeColoredCube(1.5f, 0.5f, 0.5f, 0.5f); // Grey cube // 1.5 units radius = 3x3x3 cube
        LOG("Center cube mesh created successfully");
        
        // Initialize text HUD system (temporarily disabled)
        // LOG("About to initialize text HUD system");
        arena::hud::TextHud_Init();
        
        // LOG("Text HUD system initialized successfully");
        
        // Create camera entity with Transform and CameraController components
        g_cameraEntityId = g_registry.create();
        g_registry.add<arena::ecs::Transform>(g_cameraEntityId,
            {{0, 1.6f, 5},   // position: a little above the ground
             {0, -0.35f, 0}, // yaw, pitch, roll: slight downward pitch
             {1, 1, 1}});
        g_registry.add<arena::ecs::CameraController>(g_cameraEntityId, {5.0f, 0.01f});
        LOG("Created camera entity with ID: " << g_cameraEntityId);
    }
    
    // Initialize clock with config
    Clock clock;
    clock.setTickRate(config.tick_hz);
    
    double last = NowSeconds();
    double lastLogTime = last;
    double startTime = last;
    
    LOG("Engine loop starting...");
    if (args.runForMs > 0) {
        LOG("Will run for " << args.runForMs << "ms then exit");
    }
    
    // Main engine loop
    while (true) {
        double now = NowSeconds(); 
        double frame = now - last; 
        last = now;
        
        // FPS tracking for HUD
        static double acc = 0.0;
        static int frames = 0;
        static arena::hud::HudStats stats;
        frames++;
        acc += frame;
        if (acc >= 1.0) {
            stats.fps = frames / acc;
            stats.ms = stats.fps > 0 ? (1000.0 / stats.fps) : 0.0;
            stats.ticks = clock.ticks;
            frames = 0;
            acc = 0.0;
        }
        
        // Begin frame for input system
        if (!args.server) {
            arena::beginFrame(g_inputState);
            glContext.pollEvents();
            
            // Update camera system with current input state
            g_cameraSystem.update(frame, g_inputState, g_registry);
            
            // Debug: Show input state changes
            static bool lastW = false, lastA = false, lastS = false, lastD = false;
            static bool lastSpace = false, lastC = false;
            static bool lastLeftClick = false, lastRightClick = false;
            
            // Check for key state changes
            if (g_inputState.keys[GLFW_KEY_W] != lastW) {
                lastW = g_inputState.keys[GLFW_KEY_W];
                if (lastW) LOG("W key PRESSED");
                else LOG("W key RELEASED");
            }
            
            if (g_inputState.keys[GLFW_KEY_A] != lastA) {
                lastA = g_inputState.keys[GLFW_KEY_A];
                if (lastA) LOG("A key PRESSED");
                else LOG("A key RELEASED");
            }
            
            if (g_inputState.keys[GLFW_KEY_S] != lastS) {
                lastS = g_inputState.keys[GLFW_KEY_S];
                if (lastS) LOG("S key PRESSED");
                else LOG("S key RELEASED");
            }
            
            if (g_inputState.keys[GLFW_KEY_D] != lastD) {
                lastD = g_inputState.keys[GLFW_KEY_D];
                if (lastD) LOG("D key PRESSED");
                else LOG("D key RELEASED");
            }
            
            if (g_inputState.keys[GLFW_KEY_SPACE] != lastSpace) {
                lastSpace = g_inputState.keys[GLFW_KEY_SPACE];
                if (lastSpace) LOG("SPACE key PRESSED");
                else LOG("SPACE key RELEASED");
            }
            
            if (g_inputState.keys[GLFW_KEY_C] != lastC) {
                lastC = g_inputState.keys[GLFW_KEY_C];
                if (lastC) LOG("C key PRESSED");
                else LOG("C key RELEASED");
            }
            
            // Check for mouse button changes
            if (g_inputState.mouseButtons[GLFW_MOUSE_BUTTON_LEFT] != lastLeftClick) {
                lastLeftClick = g_inputState.mouseButtons[GLFW_MOUSE_BUTTON_LEFT];
                if (lastLeftClick) LOG("Left mouse button PRESSED");
                else LOG("Left mouse button RELEASED");
            }
            
            if (g_inputState.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT] != lastRightClick) {
                lastRightClick = g_inputState.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT];
                if (lastRightClick) LOG("Right mouse button PRESSED");
                else LOG("Right mouse button RELEASED");
            }
            
            // Show mouse movement (only when there is movement)
            if (g_inputState.mouseDx != 0.0 || g_inputState.mouseDy != 0.0) {
                LOG("Mouse moved: dx=" << g_inputState.mouseDx << " dy=" << g_inputState.mouseDy);
            }
            
            // Handle sun time controls
            static bool lastLeftBracket = false, lastRightBracket = false;
            
            if (g_inputState.keys[GLFW_KEY_LEFT_BRACKET] != lastLeftBracket) {
                lastLeftBracket = g_inputState.keys[GLFW_KEY_LEFT_BRACKET];
                if (lastLeftBracket) {
                    g_sunLighting.adjustTime(-1.0f); // Move time backward by 1 hour
                    LOG("Sun time adjusted: " << g_sunLighting.getTimeOfDay() << ":00");
                }
            }
            
            if (g_inputState.keys[GLFW_KEY_RIGHT_BRACKET] != lastRightBracket) {
                lastRightBracket = g_inputState.keys[GLFW_KEY_RIGHT_BRACKET];
                if (lastRightBracket) {
                    g_sunLighting.adjustTime(1.0f); // Move time forward by 1 hour
                    LOG("Sun time adjusted: " << g_sunLighting.getTimeOfDay() << ":00");
                }
            }
            
            // Show current key states every 60 frames for debugging
            static int keyDebugCounter = 0;
            if (++keyDebugCounter % 60 == 0) {
                LOG("Key states - W:" << g_inputState.keys[GLFW_KEY_W] 
                    << " A:" << g_inputState.keys[GLFW_KEY_A] 
                    << " S:" << g_inputState.keys[GLFW_KEY_S] 
                    << " D:" << g_inputState.keys[GLFW_KEY_D]
                    << " SPACE:" << g_inputState.keys[GLFW_KEY_SPACE]
                    << " C:" << g_inputState.keys[GLFW_KEY_C]);
            }
            
            // Show camera position and rotation
            static int logCounter = 0;
            if (++logCounter % 60 == 0) { // Log every 60 frames (about once per second at 60Hz)
                auto* cameraTransform = g_registry.get<arena::ecs::Transform>(g_cameraEntityId);
                if (cameraTransform) {
                    LOG("Camera pos: (" << cameraTransform->pos[0] << ", " << cameraTransform->pos[1] << ", " << cameraTransform->pos[2] << ")");
                    LOG("Camera rot: (" << cameraTransform->rotYawPitchRoll[0] << ", " << cameraTransform->rotYawPitchRoll[1] << ", " << cameraTransform->rotYawPitchRoll[2] << ")");
                }
            }
        }
        
        // Check if we should exit based on --runForMs FIRST (before anything else)
        if (args.runForMs > 0) {
            double elapsedMs = (now - startTime) * 1000.0;
            if (elapsedMs >= args.runForMs) {
                LOG("Reached --runForMs limit (" << args.runForMs << "ms), exiting");
                break;
            }
        }
        
        clock.step(frame); // Fixed-step simulation
        
        // Log tick count every second
        if (now - lastLogTime >= 1.0) {
            double actualRate = clock.ticks / (now - startTime);
            LOG("Ticks: " << clock.ticks << " (accum: " << clock.accumulator << ", rate: " << actualRate << " Hz, target: " << config.tick_hz << " Hz)");
            lastLogTime = now;
        }
        
        // Process GLFW events and render
        if (!args.server) {
            // Check if window should close
            if (glContext.shouldClose()) {
                LOG("Window close requested, exiting");
                break;
            }
            
            // Check for shader hot-reload
            basicShader.reloadIfChanged();
            
            // Clear the screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            // Set up viewport
            int fbW = 0, fbH = 0;
            glContext.getFramebufferSize(&fbW, &fbH);
            glViewport(0, 0, fbW, fbH);
            
            // Get camera transform for view matrix
            auto* cameraTransform = g_registry.get<arena::ecs::Transform>(g_cameraEntityId);
            if (cameraTransform) {
                // View = R_x(-pitch) * R_y(-yaw) * T(-pos)
                glm::mat4 view = glm::mat4(1.0f);
                glm::vec3 camPos(
                    cameraTransform->pos[0],
                    cameraTransform->pos[1],
                    cameraTransform->pos[2]
                );
                float yaw   = cameraTransform->rotYawPitchRoll[0];
                float pitch = cameraTransform->rotYawPitchRoll[1];

                view = glm::rotate(view, -pitch, glm::vec3(1.0f, 0.0f, 0.0f));
                view = glm::rotate(view, -yaw,   glm::vec3(0.0f, 1.0f, 0.0f));
                view = glm::translate(view, -camPos);

                // Projection
                float aspect = static_cast<float>(fbW) / static_cast<float>(fbH);
                glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

                // VP
                glm::mat4 VP = projection * view;



                // Render grid
                glEnable(GL_DEPTH_TEST);
                glDisable(GL_CULL_FACE);

                basicShader.use();
                
                // Set view and projection matrices
                GLint viewLoc = basicShader.uni("uV");
                GLint projLoc = basicShader.uni("uP");
                GLint modelLoc = basicShader.uni("uM");
                
                if (viewLoc != -1) {
                    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
                }
                if (projLoc != -1) {
                    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
                }
                
                // Set sun lighting uniforms
                GLint sunDirLoc = basicShader.uni("uSunDir");
                GLint sunColorLoc = basicShader.uni("uSunColor");
                
                if (sunDirLoc != -1) {
                    glm::vec3 sunDir = g_sunLighting.getSunDirection();
                    glUniform3fv(sunDirLoc, 1, &sunDir[0]);
                }
                if (sunColorLoc != -1) {
                    glm::vec3 sunColor = g_sunLighting.getSunColor();
                    glUniform3fv(sunColorLoc, 1, &sunColor[0]);
                }
                
                // Set background color based on sun lighting
                glm::vec3 ambientColor = g_sunLighting.getAmbientColor();
                glClearColor(ambientColor.r, ambientColor.g, ambientColor.b, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // Render grid
                if (modelLoc != -1) {
                    glm::mat4 model = glm::mat4(1.0f);
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
                }
                gridMesh.bind();
                glDrawElements(GL_TRIANGLES, gridMesh.indexCount, GL_UNSIGNED_INT, 0);

                // Render coordinate axes
                coordinateAxesMesh.bind();
                glDrawElements(GL_TRIANGLES, coordinateAxesMesh.indexCount, GL_UNSIGNED_INT, 0);

                // Render center cube
                centerCubeMesh.bind();
                glDrawElements(GL_TRIANGLES, centerCubeMesh.indexCount, GL_UNSIGNED_INT, 0);

                // Set up OpenGL state for 2D overlay rendering
                glDisable(GL_DEPTH_TEST);  // Disable depth test for 2D overlay
                glEnable(GL_BLEND);        // Ensure blending is enabled for text
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                
                // Begin text HUD frame
                arena::hud::TextHud_BeginFrame(fbW, fbH);
                
                // Draw text HUD overlay
                arena::hud::TextHud_DrawStats(stats);
                
                // Draw sun time info
                char timeStr[32];
                snprintf(timeStr, sizeof(timeStr), "Sun Time: %.1f:00", g_sunLighting.getTimeOfDay());
                arena::hud::TextHud_DrawLine(10, 100, timeStr, 0.8f, 0.8f, 1.0f);
                
                // Draw sun direction info
                glm::vec3 sunDir = g_sunLighting.getSunDirection();
                snprintf(timeStr, sizeof(timeStr), "Sun Dir: (%.2f, %.2f, %.2f)", sunDir.x, sunDir.y, sunDir.z);
                arena::hud::TextHud_DrawLine(10, 120, timeStr, 0.8f, 0.8f, 1.0f);
            }
            
            // Swap buffers
            glContext.swapBuffers();
        }
        
        // Small sleep to prevent 100% CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    // Cleanup text HUD system
    if (!args.server) {
        arena::hud::TextHud_Shutdown();
        gridMesh.destroy();
        coordinateAxesMesh.destroy();
        centerCubeMesh.destroy();
    }
    
    // GL context cleanup is handled by destructor
    
    double totalTime = NowSeconds() - startTime;
    double finalRate = clock.ticks / totalTime;
    
    LOG("Final Results:");
    LOG("Total time: " << totalTime << " seconds");
    LOG("Total ticks: " << clock.ticks);
    LOG("Average rate: " << finalRate << " Hz (target: " << config.tick_hz << " Hz)");
    LOG("Rate accuracy: " << (finalRate / config.tick_hz * 100.0) << "%");
    
    return 0;
}
