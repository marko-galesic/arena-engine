#pragma once

#include "arena/gfx/shader.hpp"

namespace arena::gfx {

struct FrameParams { 
    int fbW, fbH; 
    float alpha; 
};

class Renderer {
public:
    bool init();
    void shutdown();
    void beginFrame(const FrameParams& fp);
    void endFrame();
    Shader& basic(); // access to a default shader

private:
    Shader basic_;
};

} // namespace arena::gfx
