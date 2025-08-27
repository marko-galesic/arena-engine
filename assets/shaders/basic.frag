#version 450 core
uniform vec3 uSunDir; // normalized world dir
uniform vec3 uSunColor;
in vec3 vN; 
out vec4 FragColor;

void main() {
    float NdotL = max(dot(normalize(vN), normalize(-uSunDir)), 0.0);
    vec3 albedo = vec3(0.75, 0.75, 0.78);
    vec3 c = albedo * (0.1 + uSunColor * NdotL);
    FragColor = vec4(c, 1.0);
}
