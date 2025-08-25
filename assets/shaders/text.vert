#version 450 core
layout(location=0) in vec2 aPosPx;
uniform vec2 uScreen;
void main(){
  vec2 ndc;
  ndc.x = (aPosPx.x / uScreen.x) * 2.0 - 1.0;
  ndc.y = 1.0 - (aPosPx.y / uScreen.y) * 2.0;
  gl_Position = vec4(ndc, 0.0, 1.0);
}

