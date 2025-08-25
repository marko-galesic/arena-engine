#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include "app/Clock.hpp"
#include "app/Config.hpp"
#include "arena/input.hpp"
#include "arena/ecs/registry.hpp"
#include "arena/ecs/camera_system.hpp"
#include "arena/text.hpp"

// Define this to include GLFW in the unified OpenGL API header
#define ARENA_NEED_GLFW

// Unified OpenGL API header
#include "arena/gl_api.hpp"

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

// Input state accessor
arena::InputState& getInputState() { return g_inputState; }

// GLFW callback functions
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
    
    GLFWwindow* window = nullptr;
    
    if (args.server) {
        LOG("Running in HEADLESS mode (--server)");
    } else {
        LOG("Running in WINDOWED mode");
        
        // Initialize GLFW
        if (!glfwInit()) {
            LOG("ERROR: Failed to initialize GLFW");
            return -1;
        }
        
        // Create window
        window = glfwCreateWindow(config.window_w, config.window_h, "Arena Engine", nullptr, nullptr);
        if (!window) {
            LOG("ERROR: Failed to create GLFW window");
            glfwTerminate();
            return -1;
        }
        
        // Set input callbacks
        glfwSetKeyCallback(window, keyCallback);
        glfwSetCursorPosCallback(window, cursorPosCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        
        // Make context current
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
        
        LOG("OpenGL context created successfully");
        
        // Initialize GLAD2 after creating the context
        if (!arena_load_gl()) { 
            fprintf(stderr, "gladLoadGL failed\n"); 
            std::abort(); 
        }
        
        // Quick runtime sanity (helps catch accidental mixing)
        fprintf(stderr, "GL: %s | %s | %s\n",
          (const char*)glGetString(GL_VENDOR),
          (const char*)glGetString(GL_RENDERER),
          (const char*)glGetString(GL_VERSION));
        
        if (!glCreateShader || !glBufferData || !glDrawArrays) {
          fprintf(stderr, "GL function pointers are null (likely wrong GLAD combo)\n");
          std::abort();
        }
        
        // Check for OpenGL errors
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            LOG("OpenGL error after GLAD2 initialization: " << err);
        }
        
        LOG("GLFW window initialized successfully");
        
        // Initialize text HUD system
        LOG("About to initialize text HUD system");
        arena::hud::TextHud_Init();
        
        LOG("Text HUD system initialized successfully");
        
        // Create camera entity with Transform and CameraController components
        g_cameraEntityId = g_registry.create();
        g_registry.add<arena::ecs::Transform>(g_cameraEntityId, {{0, 0, 5}, {0, 0, 0}, {1, 1, 1}});
        g_registry.add<arena::ecs::CameraController>(g_cameraEntityId, {5.0f, 0.002f});
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
        
        // Process GLFW events
        if (!args.server) {
            glfwPollEvents();
            
            // Check if window should close
            if (glfwWindowShouldClose(window)) {
                LOG("Window close requested, exiting");
                break;
            }
            
            // Clear the screen with a dark blue color
            glClearColor(0.1f, 0.2f, 0.4f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            // Set up basic OpenGL state for 3D rendering
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            
            // Set up viewport
            glViewport(0, 0, config.window_w, config.window_h);
            
            // TODO: Add 3D scene rendering here when ready
            
            // Set up OpenGL state for 2D overlay rendering
            glDisable(GL_DEPTH_TEST);  // Disable depth test for 2D overlay
            glEnable(GL_BLEND);        // Ensure blending is enabled for text
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            // Begin text HUD frame
            int fbW = 0, fbH = 0;
            glfwGetFramebufferSize(window, &fbW, &fbH);
            arena::hud::TextHud_BeginFrame(fbW, fbH);
            
            // Draw text HUD overlay
            arena::hud::TextHud_DrawStats(stats);
            
            // Swap buffers
            glfwSwapBuffers(window);
        }
        
        // Small sleep to prevent 100% CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    // Cleanup text HUD system
    if (!args.server) {
        arena::hud::TextHud_Shutdown();
    }
    
    // Cleanup GLFW
    if (!args.server) {
        glfwTerminate();
    }
    
    double totalTime = NowSeconds() - startTime;
    double finalRate = clock.ticks / totalTime;
    
    LOG("Final Results:");
    LOG("Total time: " << totalTime << " seconds");
    LOG("Total ticks: " << clock.ticks);
    LOG("Average rate: " << finalRate << " Hz (target: " << config.tick_hz << " Hz)");
    LOG("Rate accuracy: " << (finalRate / config.tick_hz * 100.0) << "%");
    
    return 0;
}
