#version 450 core

in vec3 vColor;

out vec4 FragColor;

void main() {
    // Modified color for hot-reload test - updated again!
    FragColor = vec4(vColor * 0.8, 1.0);
}
