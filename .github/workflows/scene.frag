#version 300 es
precision mediump float;
in vec3 vWorld; in vec3 vColor;
uniform vec3 uCamera;
uniform float uTime;
out vec4 frag;
void main(){
 vec3 dx=dFdx(vWorld),dy=dFdy(vWorld);vec3 n=normalize(cross(dx,dy));if(!gl_FrontFacing)n=-n;
 float phase=uTime*.035;vec3 l=normalize(vec3(-.38+sin(phase)*.22,.72+cos(phase)*.18,.31));
 float ndl=max(dot(n,l),0.0);float ambient=.24+.12*max(n.y,0.0);
 float spec=pow(max(dot(reflect(-l,n),normalize(uCamera-vWorld)),0.0),20.0)*.10;
 vec3 c=vColor*(ambient+ndl*.72)+spec*vec3(.72,.82,1.0);
 c=c/(vec3(1.0)+c*.22);
 float fog=smoothstep(52.0,104.0,distance(vWorld,uCamera));
 vec3 haze=mix(vec3(.075,.105,.15),vec3(.16,.19,.25),.5+.5*sin(uTime*.025));
 frag=vec4(mix(c,haze,fog),1.0);
}
