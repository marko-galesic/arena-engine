#version 450 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;
uniform mat4 uM; uniform mat4 uV; uniform mat4 uP;
out vec3 vN; out vec3 vWPos; out vec2 vUV;
void main(){
  vec4 wpos = uM * vec4(aPos,1.0);
  vWPos = wpos.xyz;
  vN = mat3(uM) * aNormal;
  vUV = aUV;
  gl_Position = uP * uV * wpos;
}
