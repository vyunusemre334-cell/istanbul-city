#include "Renderer.h"
#include <android/log.h>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <array>
#include <iterator>
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"IstanbulVice",__VA_ARGS__)
namespace iv {
static const char* VS=R"(#version 300 es
precision highp float;
layout(location=0) in vec3 aPos;
layout(location=1) in vec4 iM0; layout(location=2) in vec4 iM1; layout(location=3) in vec4 iM2; layout(location=4) in vec4 iM3;
layout(location=5) in vec3 iColor;
uniform mat4 uVP; out vec3 vWorld; out vec3 vColor;
void main(){mat4 M=mat4(iM0,iM1,iM2,iM3);vec4 w=M*vec4(aPos,1.0);vWorld=w.xyz;vColor=iColor;gl_Position=uVP*w;})";
static const char* FS=R"(#version 300 es
precision mediump float;
in vec3 vWorld;in vec3 vColor;uniform vec3 uCamera;uniform float uTime;out vec4 frag;
void main(){
 vec3 dx=dFdx(vWorld),dy=dFdy(vWorld);vec3 n=normalize(cross(dx,dy));if(!gl_FrontFacing)n=-n;
 float sunPhase=uTime*.035;vec3 l=normalize(vec3(-.38+sin(sunPhase)*.22,.72+cos(sunPhase)*.18,.31));
 float ndl=max(dot(n,l),0.0);float ambient=.24+.12*max(n.y,0.0);
 float spec=pow(max(dot(reflect(-l,n),normalize(uCamera-vWorld)),0.0),20.0)*.10;
 // Low-cost stylized PBR: restrained specular, distance haze and gentle filmic contrast.
 vec3 c=vColor*(ambient+ndl*.72)+spec*vec3(.72,.82,1.0);
 c=c/(vec3(1.0)+c*.22);
 float dist=distance(vWorld,uCamera);float fog=smoothstep(52.0,104.0,dist);
 vec3 haze=mix(vec3(.075,.105,.15),vec3(.16,.19,.25),.5+.5*sin(uTime*.025));
 c=mix(c,haze,fog);frag=vec4(c,1.0);
})";
static const float cube[]={
 -0.5,-0.5,0.5,0.5,-0.5,0.5,0.5,0.5,0.5,
 -0.5,-0.5,0.5,0.5,0.5,0.5,-0.5,0.5,0.5,
 0.5,-0.5,-0.5,-0.5,-0.5,-0.5,-0.5,0.5,-0.5,
 0.5,-0.5,-0.5,-0.5,0.5,-0.5,0.5,0.5,-0.5,
 -0.5,-0.5,-0.5,-0.5,-0.5,0.5,-0.5,0.5,0.5,
 -0.5,-0.5,-0.5,-0.5,0.5,0.5,-0.5,0.5,-0.5,
 0.5,-0.5,0.5,0.5,-0.5,-0.5,0.5,0.5,-0.5,
 0.5,-0.5,0.5,0.5,0.5,-0.5,0.5,0.5,0.5,
 -0.5,-0.5,-0.5,0.5,-0.5,-0.5,0.5,-0.5,0.5,
 -0.5,-0.5,-0.5,0.5,-0.5,0.5,-0.5,-0.5,0.5,
 -0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,-0.5,
 -0.5,0.5,0.5,0.5,0.5,-0.5,-0.5,0.5,-0.5};
GLuint Renderer::shader(GLenum t,const char*s){GLuint id=glCreateShader(t);glShaderSource(id,1,&s,nullptr);glCompileShader(id);GLint ok=0;glGetShaderiv(id,GL_COMPILE_STATUS,&ok);if(!ok){char log[1024]{};glGetShaderInfoLog(id,sizeof(log),nullptr,log);LOGE("shader: %s",log);glDeleteShader(id);return 0;}return id;}
GLuint Renderer::makeProgram(const char*v,const char*f){GLuint a=shader(GL_VERTEX_SHADER,v),b=shader(GL_FRAGMENT_SHADER,f);if(!a||!b){if(a)glDeleteShader(a);if(b)glDeleteShader(b);return 0;}GLuint p=glCreateProgram();glAttachShader(p,a);glAttachShader(p,b);glLinkProgram(p);glDeleteShader(a);glDeleteShader(b);GLint ok=0;glGetProgramiv(p,GL_LINK_STATUS,&ok);if(!ok){char log[1024]{};glGetProgramInfoLog(p,sizeof(log),nullptr,log);LOGE("link: %s",log);glDeleteProgram(p);return 0;}return p;}
bool Renderer::initialize(){
 program_=makeProgram(VS,FS);if(!program_)return false;
 glGenVertexArrays(1,&vao_);glGenBuffers(1,&vbo_);glGenBuffers(1,&sphereVbo_);glGenBuffers(1,&cylinderVbo_);glGenBuffers(1,&instanceVbo_);
 glBindVertexArray(vao_);glBindBuffer(GL_ARRAY_BUFFER,vbo_);glBufferData(GL_ARRAY_BUFFER,sizeof(cube),cube,GL_STATIC_DRAW);
 glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);
 glBindBuffer(GL_ARRAY_BUFFER,instanceVbo_);const GLsizei stride=19*sizeof(float);
 for(int i=0;i<4;++i){glEnableVertexAttribArray(1+i);glVertexAttribPointer(1+i,4,GL_FLOAT,GL_FALSE,stride,reinterpret_cast<void*>(static_cast<std::uintptr_t>(i*4*sizeof(float))));glVertexAttribDivisor(1+i,1);}
 glEnableVertexAttribArray(5);glVertexAttribPointer(5,3,GL_FLOAT,GL_FALSE,stride,reinterpret_cast<void*>(16*sizeof(float)));glVertexAttribDivisor(5,1);
 std::vector<float> sphere;constexpr int rings=10,sectors=16;for(int r=0;r<rings;++r){float a0=-1.5707963f+3.1415926f*float(r)/rings,a1=-1.5707963f+3.1415926f*float(r+1)/rings;for(int q=0;q<sectors;++q){float b0=6.2831853f*float(q)/sectors,b1=6.2831853f*float(q+1)/sectors;auto v=[](float a,float b){return std::array<float,3>{.5f*std::cos(a)*std::cos(b),.5f*std::sin(a),.5f*std::cos(a)*std::sin(b)};};auto p00=v(a0,b0),p01=v(a0,b1),p10=v(a1,b0),p11=v(a1,b1);for(auto p:{p00,p10,p11,p00,p11,p01})sphere.insert(sphere.end(),p.begin(),p.end());}}
 sphereVertexCount_=static_cast<GLsizei>(sphere.size()/3);glBindBuffer(GL_ARRAY_BUFFER,sphereVbo_);glBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(sphere.size()*sizeof(float)),sphere.data(),GL_STATIC_DRAW);
 std::vector<float> cylinder;constexpr int sides=18;for(int i=0;i<sides;++i){float a=6.2831853f*float(i)/sides,b=6.2831853f*float(i+1)/sides;float x0=.5f*std::cos(a),z0=.5f*std::sin(a),x1=.5f*std::cos(b),z1=.5f*std::sin(b);float tri[]={x0,-.5f,z0,x1,-.5f,z1,x1,.5f,z1,x0,-.5f,z0,x1,.5f,z1,x0,.5f,z0};cylinder.insert(cylinder.end(),std::begin(tri),std::end(tri));}
 cylinderVertexCount_=static_cast<GLsizei>(cylinder.size()/3);glBindBuffer(GL_ARRAY_BUFFER,cylinderVbo_);glBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(cylinder.size()*sizeof(float)),cylinder.data(),GL_STATIC_DRAW);
 glBindVertexArray(0);vp_=glGetUniformLocation(program_,"uVP");camera_=glGetUniformLocation(program_,"uCamera");time_=glGetUniformLocation(program_,"uTime");glEnable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glClearDepthf(1);instances_.reserve(1800*19);sphereInstances_.reserve(300*19);cylinderInstances_.reserve(300*19);return true;
}
void Renderer::resize(int w,int h){width_=w>0?w:1;height_=h>0?h:1;glViewport(0,0,width_,height_);}
void Renderer::add(const V3&p,const V3&s,float r,float g,float b,float yaw){float dx=p.x-cameraX_,dz=p.z-cameraZ_;float d2=dx*dx+dz*dz;const float maxDistance=qualityLevel_==0?110.0f:(qualityLevel_==1?90.0f:74.0f);if(d2>maxDistance*maxDistance)return; // Distance culling.
 // Conservative camera-frustum rejection for the forward-facing city view.
 if(dz> -3.0f && std::abs(dx)>std::max(18.0f,dz*.92f+18.0f))return;
 const float cs=std::cos(yaw),sn=std::sin(yaw);const float m[16]={cs*s.x,0,-sn*s.x,0,0,s.y,0,0,sn*s.z,0,cs*s.z,0,p.x,p.y,p.z,1};instances_.insert(instances_.end(),m,m+16);instances_.push_back(r);instances_.push_back(g);instances_.push_back(b);
}
void Renderer::addSphere(const V3&p,const V3&s,float r,float gg,float b,float yaw){float dx=p.x-cameraX_,dz=p.z-cameraZ_;float maxDistance=qualityLevel_==0?110.0f:(qualityLevel_==1?90.0f:74.0f);if(dx*dx+dz*dz>maxDistance*maxDistance)return;if(dz>-3.0f&&std::abs(dx)>std::max(18.0f,dz*.92f+18.0f))return;float cs=std::cos(yaw),sn=std::sin(yaw);const float m[16]={cs*s.x,0,-sn*s.x,0,0,s.y,0,0,sn*s.z,0,cs*s.z,0,p.x,p.y,p.z,1};sphereInstances_.insert(sphereInstances_.end(),m,m+16);sphereInstances_.push_back(r);sphereInstances_.push_back(gg);sphereInstances_.push_back(b);}
void Renderer::addCylinder(const V3&p,const V3&s,float r,float gg,float b,float yaw){float dx=p.x-cameraX_,dz=p.z-cameraZ_;float maxDistance=qualityLevel_==0?110.0f:(qualityLevel_==1?90.0f:74.0f);if(dx*dx+dz*dz>maxDistance*maxDistance)return;if(dz>-3.0f&&std::abs(dx)>std::max(18.0f,dz*.92f+18.0f))return;float cs=std::cos(yaw),sn=std::sin(yaw);const float m[16]={cs*s.x,0,-sn*s.x,0,0,s.y,0,0,sn*s.z,0,cs*s.z,0,p.x,p.y,p.z,1};cylinderInstances_.insert(cylinderInstances_.end(),m,m+16);cylinderInstances_.push_back(r);cylinderInstances_.push_back(gg);cylinderInstances_.push_back(b);}
void Renderer::render(const Game&g,float time){
 const float qdt=lastQualityTime_>0?std::clamp(time-lastQualityTime_,0.0f,.25f):0.0f;lastQualityTime_=time;
 if(g.fps()<44.0f){lowFpsTime_+=qdt;highFpsTime_=0;}else if(g.fps()>56.0f){highFpsTime_+=qdt;lowFpsTime_=0;}else{lowFpsTime_=std::max(0.0f,lowFpsTime_-qdt*.5f);highFpsTime_=0;}
 if(lowFpsTime_>3.0f&&qualityLevel_<2){++qualityLevel_;lowFpsTime_=0;}
 if(highFpsTime_>8.0f&&qualityLevel_>0){--qualityLevel_;highFpsTime_=0;}
 cameraX_=g.player().x;cameraZ_=g.player().z;instances_.clear();sphereInstances_.clear();cylinderInstances_.clear();
 // Continuous city ground, asphalt avenues, sidewalks and painted lane markings.
 add({0,-.62f,0},{220,.55f,220},.16f,.19f,.20f);
 for(int i=-5;i<=5;++i){
  const float lane=float(i)*18.0f;
  add({lane,-.28f,0},{7.2f,.12f,210},.105f,.115f,.13f);
  add({0,-.28f,lane},{210,.12f,7.2f},.105f,.115f,.13f);
  for(float k=-96;k<=96;k+=12){
   add({lane,-.205f,k},{.13f,.025f,4.2f},.72f,.68f,.48f);
   add({k,-.205f,lane},{4.2f,.025f,.13f},.72f,.68f,.48f);
  }
  for(float side : {-5.0f,5.0f}){
   add({lane+side,-.20f,0},{1.7f,.20f,210},.34f,.34f,.32f);
   add({0,-.20f,lane+side},{210,.20f,1.7f},.34f,.34f,.32f);
  }
 }
 // Bosphorus waterfront and promenade.
 add({0,-.38f,-105},{220,.18f,18},.025f,.20f,.31f);
 add({0,-.18f,-94},{220,.22f,3.0f},.48f,.46f,.39f);
 // Crosswalk markings at central junctions.
 for(int j=-2;j<=2;++j)for(int k=-3;k<=3;++k){
  add({float(j)*.72f,-.18f,float(k)*1.0f},{.38f,.025f,.62f},.78f,.78f,.72f);
  add({float(k)*1.0f,-.18f,float(j)*.72f},{.62f,.025f,.38f},.78f,.78f,.72f);
 }
 // Sparse street lamps.
 for(int i=-5;i<=5;i+=2)for(int side : {-1,1}){
  float x=float(i)*18.0f+float(side)*6.1f;
  add({x,2.0f,0},{.13f,4.0f,.13f},.20f,.21f,.22f);
  add({x,4.05f,0},{.48f,.20f,.48f},.95f,.72f,.38f);
 }
 // Original city landmarks with distinct silhouettes.
 add({-39,8,-38},{8,16,8},.62f,.53f,.39f);
 add({-39,17,-38},{4.2f,2.5f,4.2f},.72f,.62f,.45f);
 add({-39,20,-38},{1.2f,4.0f,1.2f},.76f,.67f,.49f);
 add({38,5,-38},{15,10,12},.70f,.62f,.49f);
 add({38,11,-38},{8,3,8},.78f,.69f,.55f);
 add({38,13,-38},{3,1.2f,3},.82f,.73f,.59f);
 add({54,15,40},{9,30,9},.28f,.42f,.53f);
 add({54,31,40},{7,3,7},.34f,.49f,.59f);
 // Architectural pass: varied Istanbul apartment blocks with stone plinths, cornices,
 // recessed-looking window glass, balcony rails, roof terraces and shopfronts.
 for(const auto&b:g.buildings()){
  const float base=.12f+.07f*b.shade, glow=qualityLevel_==0?.92f+.08f*std::sin(time*.35f+b.shade*4):1.0f;
  const float h=b.s.y, wx=b.s.x, wz=b.s.z;
  add(b.p,b.s,base*glow,(base+.025f)*glow,(base+.065f)*glow);
  add({b.p.x,.16f,b.p.z},{wx+.35f,.30f,wz+.35f},.37f,.34f,.29f);
  add({b.p.x,b.p.y+h*.5f+.12f,b.p.z},{wx+.42f,.24f,wz+.42f},.27f,.29f,.31f);
  const int floors=std::max(2,int(h/3.0f));
  for(int f=0;f<floors;++f){float y=1.25f+float(f)*2.65f;if(y>h-.65f)break;
   for(int side=-1;side<=1;++side){float x=b.p.x+float(side)*wx*.29f;
    add({x,y,b.p.z-wz*.502f},{.72f,1.15f,.055f},.055f,.105f,.16f);
    add({x,y+.60f,b.p.z-wz*.525f},{.82f,.09f,.12f},.54f,.49f,.39f);
    add({x,y-.60f,b.p.z-wz*.525f},{.82f,.09f,.12f},.54f,.49f,.39f);
    add({x,y,b.p.z-wz*.55f},{.055f,1.28f,.07f},.48f,.44f,.36f);
   }
   for(int side=-1;side<=1;++side){float z=b.p.z+float(side)*wz*.29f;
    add({b.p.x+wx*.502f,y,z},{.055f,1.12f,.72f},.06f,.11f,.16f);
    add({b.p.x+wx*.55f,y-.60f,z},{.07f,.09f,.82f},.48f,.44f,.36f);
   }
   if(f>0&&f%2==0){add({b.p.x,y-.72f,b.p.z-wz*.60f},{1.15f,.12f,.35f},.40f,.38f,.34f);add({b.p.x,y-.40f,b.p.z-wz*.72f},{1.15f,.08f,.06f},.58f,.56f,.50f);}
  }
  // Rooftop parapet and small water tank / HVAC details.
  add({b.p.x,b.p.y+h*.5f+.35f,b.p.z},{wx+.25f,.55f,wz+.25f},.32f,.32f,.31f);
  add({b.p.x+wx*.22f,b.p.y+h*.5f+.85f,b.p.z-wz*.18f},{.85f,.75f,.72f},.38f,.42f,.43f);
  // Ground-floor shop windows and sign fascia.
  add({b.p.x,b.p.y+.92f,b.p.z-wz*.53f},{wx*.72f,1.35f,.07f},.055f,.13f,.17f);
  add({b.p.x,.42f,b.p.z-wz*.56f},{wx*.82f,.13f,.10f},.68f,.48f,.25f);
 }
 // Sidewalk greenery, planters, benches and bins bring scale to the streets.
 for(int block=-4;block<=4;++block)for(int side : {-1,1}){
  float x=float(block)*18.0f+float(side)*6.3f;
  for(int z=-72;z<=72;z+=36){
   add({x,.48f,float(z)},{.48f,.95f,.48f},.34f,.20f,.12f);
   addSphere({x,1.65f,float(z)},{2.55f,2.75f,2.55f},.12f,.31f,.19f);
   add({x+1.4f,.48f,float(z)+2.5f},{1.35f,.12f,.48f},.35f,.22f,.12f);
   add({x+1.4f,.78f,float(z)+2.5f},{1.35f,.10f,.10f},.42f,.29f,.16f);
  }
 }
 // A recognizable neighborhood mosque silhouette on the waterfront side.
 add({-39,3.0f,-38},{11,6,10},.70f,.62f,.48f);
 add({-39,7.0f,-38},{8.5f,3.0f,8.5f},.76f,.68f,.54f);
 addSphere({-39,9.1f,-38},{5.4f,2.5f,5.4f},.79f,.71f,.58f);
 for(float sx : {-45.0f,-33.0f}){addCylinder({sx,9.0f,-38},{.75f,15.0f,.75f},.72f,.65f,.53f);add({sx,16.7f,-38},{1.15f,.65f,1.15f},.79f,.70f,.57f);addCylinder({sx,19.0f,-38},{.16f,4.2f,.16f},.83f,.73f,.59f);}
 // Street lights with warm lantern heads and traffic signals at the main junction.
 for(int axis=-1;axis<=1;axis+=2){add({axis*6.0f,3.1f,8.0f},{.16f,6.2f,.16f},.20f,.22f,.24f);add({axis*6.0f,6.1f,8.0f},{.85f,.18f,.18f},.92f,.70f,.38f);}
 for(int i=-1;i<=1;++i){float x=6.8f+float(i)*.0f;add({x,3.0f,7.2f},{.16f,5.8f,.16f},.13f,.15f,.17f);add({x,5.5f,7.2f},{.62f,.82f,.25f},.12f,.15f,.17f);add({x,5.75f,7.2f},{.22f,.22f,.22f},i==0?.90f:.18f,.10f,.08f);}
 // Pulsing 3D mission beacon marks the active destination in the world.
 if(!g.missionComplete()){V3 m=g.missionObjective();float pulse=.82f+.18f*std::sin(time*3.0f);add({m.x,1.15f,m.z},{.34f,2.3f,.34f},.95f,.58f,.12f);add({m.x,2.45f,m.z},{1.05f,.18f,1.05f},1.0f,.72f,.18f);add({m.x,3.05f,m.z},{.55f,.55f,.55f},1.0f*pulse,.78f*pulse,.25f*pulse);}
 const V3&p=g.player();if(!g.driving()){
  float stride=g.moving()?std::sin(time*(g.sprinting()?13.0f:8.0f))*.24f:0.0f;float bob=g.moving()&&g.grounded()?std::abs(std::sin(time*(g.sprinting()?13.0f:8.0f)))*.045f:0.0f;
  float yaw=g.facing();
  // Articulated-looking human silhouette: jacket torso, shoulders, arms, legs, shoes and head.
  addSphere({p.x,p.y+.98f+bob,p.z},{.66f,.92f,.42f},.10f,.25f,.48f,yaw);
  addSphere({p.x,p.y+1.43f+bob,p.z},{.57f,.26f,.43f},.13f,.30f,.56f,yaw);
  addSphere({p.x,p.y+1.73f+bob,p.z},{.34f,.38f,.34f},.67f,.43f,.29f,yaw);
  add({p.x,p.y+1.89f+bob,p.z-.035f},{.27f,.12f,.30f},.12f,.10f,.09f,yaw);
  addCylinder({p.x-.40f,p.y+1.04f+bob,p.z},{.20f,.70f,.22f},.62f,.39f,.26f,yaw+stride*.35f);
  addCylinder({p.x+.40f,p.y+1.04f+bob,p.z},{.20f,.70f,.22f},.62f,.39f,.26f,yaw-stride*.35f);
  addCylinder({p.x-.17f,p.y+.36f+stride,p.z},{.23f,.66f,.25f},.13f,.16f,.22f,yaw);
  addCylinder({p.x+.17f,p.y+.36f-stride,p.z},{.23f,.66f,.25f},.13f,.16f,.22f,yaw);
  add({p.x-.17f,p.y+.08f+stride,p.z+.07f},{.28f,.16f,.42f},.055f,.06f,.07f,yaw);
  add({p.x+.17f,p.y+.08f-stride,p.z+.07f},{.28f,.16f,.42f},.055f,.06f,.07f,yaw);
 }
 // Player car: sculpted body, cabin, glass, bumpers, lights and four dark wheels.
 const auto&c=g.car();const float carYaw=g.vehicleHeading(),cc=std::cos(carYaw),ss=std::sin(carYaw);auto carPoint=[&](float x,float y,float z){return V3{c.x+cc*x+ss*z,c.y+y,c.z-ss*x+cc*z};};
 add({c.x,c.y+.50f,c.z},{2.12f,.68f,3.72f},.39f,.055f,.48f,carYaw);
 add({c.x,c.y+.94f,c.z-.18f},{1.62f,.52f,1.92f},.075f,.12f,.18f,carYaw);
 add(carPoint(0,.95f,.79f),{1.42f,.38f,.055f},.12f,.25f,.34f,carYaw);
 add(carPoint(0,.95f,-.95f),{1.42f,.38f,.055f},.12f,.25f,.34f,carYaw);
 add(carPoint(0,.24f,1.91f),{2.18f,.22f,.16f},.17f,.17f,.19f,carYaw);
 add(carPoint(0,.28f,-1.91f),{2.18f,.20f,.16f},.17f,.17f,.19f,carYaw);
 for(float sx : {-1.0f,1.0f})for(float sz : {-1.18f,1.18f}){addCylinder(carPoint(sx*1.02f,.38f,sz),{.24f,.58f,.58f},.035f,.04f,.05f,carYaw);add(carPoint(sx*1.08f,.62f,sz),{.10f,.12f,.34f},.16f,.17f,.18f,carYaw);}
 add(carPoint(-.62f,.55f,1.91f),{.38f,.22f,.08f},.95f,.83f,.54f,carYaw);add(carPoint(.62f,.55f,1.91f),{.38f,.22f,.08f},.95f,.83f,.54f,carYaw);
 for(const auto&v:g.traffic()){
  add({v.p.x,.48f,v.p.z},{1.86f,.64f,3.25f},v.r,v.g,v.b);add({v.p.x,.91f,v.p.z-.18f},{1.48f,.46f,1.72f},.10f,.17f,.22f);
  for(float sx : {-1.0f,1.0f})for(float sz : {-1.0f,1.0f})addCylinder({v.p.x+sx*.91f,.35f,v.p.z+sz*1.05f},{.18f,.52f,.58f},.025f,.03f,.035f);
  add({v.p.x-.57f,.54f,v.p.z+1.64f},{.28f,.18f,.06f},.95f,.82f,.55f);add({v.p.x+.57f,.54f,v.p.z+1.64f},{.28f,.18f,.06f},.95f,.82f,.55f);
 }
 for(const auto&n:g.pedestrians()){
  addSphere({n.p.x,.72f,n.p.z},{.46f,.88f,.32f},.38f,.18f,.48f);addCylinder({n.p.x-.12f,.20f,n.p.z},{.16f,.42f,.17f},.16f,.18f,.22f);addCylinder({n.p.x+.12f,.20f,n.p.z},{.16f,.42f,.17f},.16f,.18f,.22f);addSphere({n.p.x,1.28f,n.p.z},{.31f,.34f,.31f},.65f,.43f,.30f);
 }
 for(const auto&a:g.agents()){float d2=(a.p.x-p.x)*(a.p.x-p.x)+(a.p.z-p.z)*(a.p.z-p.z);if(qualityLevel_==2&&d2>32*32)continue;float lod=d2>42*42?.72f:1.0f;float r=.35f,gg=.35f,bb=.35f;if(a.kind==AgentKind::Gang){r=.48f;gg=.10f;bb=.78f;}else if(a.kind==AgentKind::Police){r=.08f;gg=.22f;bb=.72f;}else{r=.55f;gg=.48f;bb=.35f;}if(a.kind==AgentKind::Gang&&a.state==AgentState::Attack){r=.75f;gg=.12f;bb=.85f;}float anim=(a.state==AgentState::Chase||a.state==AgentState::Flee)?std::sin(time*10.0f+a.phase)*.13f:std::sin(time*2.0f+a.phase)*.035f;float yaw=std::atan2(a.target.x-a.p.x,a.target.z-a.p.z);addSphere({a.p.x,.67f*lod,a.p.z},{.44f*lod,.94f*lod,.38f*lod},r,gg,bb,yaw);if(qualityLevel_<2){addCylinder({a.p.x-.25f*lod,.66f*lod+anim,a.p.z},{.13f*lod,.58f*lod,.14f*lod},r*.9f,gg*.9f,bb*.9f,yaw);addCylinder({a.p.x+.25f*lod,.66f*lod-anim,a.p.z},{.13f*lod,.58f*lod,.14f*lod},r*.9f,gg*.9f,bb*.9f,yaw);addSphere({a.p.x,1.25f*lod,a.p.z},{.32f*lod,.34f*lod,.32f*lod},.28f,.19f,.15f,yaw);}}
 const float aspect=float(width_)/float(height_),f=1.0f/std::tan(.72f*.5f);float P[16]={f/aspect,0,0,0,0,f,0,0,0,0,-1.002f,-1,0,0,-.2002f,0};float V[16]={1,0,0,0,0,1,0,0,0,0,1,0,-cameraX_,-2.8f,-(19.0f+cameraZ_),1};float VP[16]{};for(int col=0;col<4;++col)for(int row=0;row<4;++row){VP[col*4+row]=0;for(int k=0;k<4;++k)VP[col*4+row]+=P[k*4+row]*V[col*4+k];}
 glViewport(0,0,width_,height_);glClearColor(.018f,.028f,.060f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glUseProgram(program_);glUniformMatrix4fv(vp_,1,GL_FALSE,VP);glUniform3f(camera_,cameraX_,0,cameraZ_);glUniform1f(time_,time);glBindVertexArray(vao_);glBindBuffer(GL_ARRAY_BUFFER,instanceVbo_);auto draw=[&](const std::vector<float>& data,GLuint mesh,GLsizei vertices){if(data.empty())return;glBindBuffer(GL_ARRAY_BUFFER,mesh);glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);glBindBuffer(GL_ARRAY_BUFFER,instanceVbo_);glBufferData(GL_ARRAY_BUFFER,data.size()*sizeof(float),nullptr,GL_STREAM_DRAW);glBufferSubData(GL_ARRAY_BUFFER,0,data.size()*sizeof(float),data.data());glDrawArraysInstanced(GL_TRIANGLES,0,vertices,static_cast<GLsizei>(data.size()/19));};draw(instances_,vbo_,36);draw(sphereInstances_,sphereVbo_,sphereVertexCount_);draw(cylinderInstances_,cylinderVbo_,cylinderVertexCount_);}
void Renderer::shutdown(){if(program_)glDeleteProgram(program_);if(vbo_)glDeleteBuffers(1,&vbo_);if(sphereVbo_)glDeleteBuffers(1,&sphereVbo_);if(cylinderVbo_)glDeleteBuffers(1,&cylinderVbo_);if(instanceVbo_)glDeleteBuffers(1,&instanceVbo_);if(vao_)glDeleteVertexArrays(1,&vao_);program_=vao_=vbo_=sphereVbo_=cylinderVbo_=instanceVbo_=0;}
}
