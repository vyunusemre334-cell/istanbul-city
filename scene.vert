#version 300 es
precision highp float;
layout(location=0) in vec3 aPos;
layout(location=1) in vec4 iM0; layout(location=2) in vec4 iM1; layout(location=3) in vec4 iM2; layout(location=4) in vec4 iM3;
layout(location=5) in vec3 iColor;
uniform mat4 uVP; out vec3 vWorld; out vec3 vColor;
void main(){mat4 M=mat4(iM0,iM1,iM2,iM3);vec4 w=M*vec4(aPos,1.0);vWorld=w.xyz;vColor=iColor;gl_Position=uVP*w;}
