#include "Part8Systems.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace iv::p8 {
namespace { constexpr float pi=3.14159265358979323846f; float clampf(float v,float a,float b){return std::max(a,std::min(b,v));} float approach(float a,float b,float t){return a+(b-a)*clampf(t,0,1);} }
void AnimationSystem::update(float dt,float speed,float grounded){
 time_+=clampf(dt,0,.1f); for(auto& b:pose_) b={};
 const float gait=(motion_==Motion::Sprint?12.f:motion_==Motion::Run?9.f:6.f);
 const float amp=(motion_==Motion::Walk||motion_==Motion::Run||motion_==Motion::Sprint)?clampf(speed/7.f,0,1):0;
 const float swing=std::sin(time_*gait)*.62f*amp;
 pose_[0].rotation.x=swing; pose_[1].rotation.x=-swing;
 pose_[2].rotation.x=-swing*.55f; pose_[3].rotation.x=swing*.55f;
 pose_[4].rotation.z=std::sin(time_*gait+pi)*.12f*amp;
 pose_[5].rotation.z=-pose_[4].rotation.z;
 if(motion_==Motion::Jump&&!grounded){pose_[0].rotation.x=-.35f;pose_[1].rotation.x=-.35f;pose_[2].rotation.x=.65f;pose_[3].rotation.x=.65f;}
 if(motion_==Motion::Crouch){pose_[6].translation.y=-.38f;pose_[0].rotation.x=-.5f;pose_[1].rotation.x=-.5f;}
 if(motion_==Motion::Attack){pose_[4].rotation.x=-1.15f;pose_[4].rotation.z=-.65f;}
 if(motion_==Motion::Dead){pose_[6].rotation.z=1.35f;pose_[0].rotation.x=.8f;pose_[1].rotation.x=.8f;}
 if(motion_==Motion::EnterVehicle||motion_==Motion::ExitVehicle){pose_[0].rotation.x=-.7f;pose_[1].rotation.x=.7f;pose_[6].rotation.x=.45f;}
}
void VehicleSystem::reset(Vec3 p){state_={};state_.position=p;}
void VehicleSystem::update(float dt,const VehicleInput& in){
 dt=clampf(dt,0,.05f);const float throttle=clampf(in.throttle,-1,1),steer=clampf(in.steer,-1,1);
 if(in.brake)state_.speed=approach(state_.speed,0,dt*3.8f);
 else if(std::abs(throttle)>.04f)state_.speed+=throttle*(throttle>0?10.5f:7.0f)*dt;
 else state_.speed=approach(state_.speed,0,dt*.55f);
 state_.speed=clampf(state_.speed,-5.5f,31.f);
 const float speedFactor=clampf(std::abs(state_.speed)/5.f,.12f,1.f);
 state_.heading+=steer*1.25f*speedFactor*dt*(state_.speed>=0?1.f:-1.f);
 state_.position.x+=std::sin(state_.heading)*state_.speed*dt;
 state_.position.z+=std::cos(state_.heading)*state_.speed*dt;
 state_.wheelAngle=clampf(steer*.48f,-.48f,.48f);
}
void VehicleSystem::collide(float impact){float hit=clampf(std::abs(impact),0,45);state_.damage=clampf(state_.damage+hit/45.f*.28f,0,1);state_.speed=-state_.speed*clampf(.18f+hit/100.f,.18f,.55f);}
void CharacterController::setInput(float x,float z,bool sprint,bool crouch){float l=std::sqrt(x*x+z*z);if(l>1){x/=l;z/=l;}x_=x;z_=z;sprint_=sprint&&!crouch;crouch_=crouch;}
void CharacterController::jump(){if(grounded_){vy_=6.2f;grounded_=false;}}
void CharacterController::update(float dt){dt=clampf(dt,0,.05f);float speed=crouch_?1.5f:(sprint_?7.f:4.2f);p_.x+=x_*speed*dt;p_.z+=z_*speed*dt;if(!grounded_){vy_-=16.5f*dt;p_.y+=vy_*dt;if(p_.y<=0){p_.y=0;vy_=0;grounded_=true;}}}
void CameraSystem::setTarget(Vec3 t,float h,bool v){target_=t;heading_=h;vehicle_=v;}
void CameraSystem::setDistance(float d){distance_=clampf(d,2.2f,9.f);}
void CameraSystem::update(float dt){dt=clampf(dt,0,.1f);float d=vehicle_?distance_+1.8f:distance_;Vec3 wanted{target_.x-std::sin(heading_)*d,target_.y+(vehicle_?2.0f:2.35f),target_.z-std::cos(heading_)*d};float k=1.f-std::exp(-dt*8.f);pose_.position.x=approach(pose_.position.x,wanted.x,k);pose_.position.y=approach(pose_.position.y,wanted.y,k);pose_.position.z=approach(pose_.position.z,wanted.z,k);pose_.target=target_;}
void CrimeSystem::addCrime(float s){wanted_=clampf(wanted_+std::max(0.f,s),0,5);}
void CrimeSystem::triggerAlarm(float s){addCrime(std::max(1.f,s));}
void CrimeSystem::update(float dt,bool nearby){if(!nearby)wanted_=std::max(0.f,wanted_-clampf(dt,0,.1f)*.012f);}
int CrimeSystem::stars()const{return std::min(5,static_cast<int>(std::ceil(wanted_)));}
bool PhysicsSystem::resolveAabb(Vec3& p,Vec3& v,Vec3 h,Vec3 c,Vec3 b,float rest){
 float dx=p.x-c.x,dy=p.y-c.y,dz=p.z-c.z;float ox=h.x+b.x-std::abs(dx),oy=h.y+b.y-std::abs(dy),oz=h.z+b.z-std::abs(dz);
 if(ox<=0||oy<=0||oz<=0)return false;
 if(ox<oy&&ox<oz){p.x+=dx<0?-ox:ox;v.x=-v.x*clampf(rest,0,1);}
 else if(oy<oz){p.y+=dy<0?-oy:oy;v.y=-v.y*clampf(rest,0,1);}
 else{p.z+=dz<0?-oz:oz;v.z=-v.z*clampf(rest,0,1);}
 return true;
}
void WorldSystem::update(float dt){dayTime_=std::fmod(dayTime_+clampf(dt,0,.1f)/1200.f,1.f);float target=weather_==Weather::Rain?1.f:0.f;wetness_=approach(wetness_,target,dt*(target>.5f?.08f:.025f));}
float WorldSystem::daylight()const{return .08f+.92f*clampf((std::sin((dayTime_-.25f)*2*pi)+.12f)/1.12f,0,1);}
void NPCSystem::update(NpcAgent& n,float dt,Vec3 threat){float dx=n.position.x-threat.x,dz=n.position.z-threat.z,d=std::sqrt(dx*dx+dz*dz);if(d<7.f&&d>.001f){n.fleeing=true;n.target={n.position.x+dx/d*12.f,n.position.y,n.position.z+dz/d*12.f};}else if(d>12)n.fleeing=false;dx=n.target.x-n.position.x;dz=n.target.z-n.position.z;float len=std::sqrt(dx*dx+dz*dz);if(len>.1f){float step=std::min(len,n.speed*(n.fleeing?1.5f:1.f)*clampf(dt,0,.05f));n.position.x+=dx/len*step;n.position.z+=dz/len*step;}}
PbrMaterial MaterialSystem::sanitize(PbrMaterial m){m.metallic=clampf(m.metallic,0,1);m.roughness=clampf(m.roughness,.04f,1);m.albedo.x=clampf(m.albedo.x,0,1);m.albedo.y=clampf(m.albedo.y,0,1);m.albedo.z=clampf(m.albedo.z,0,1);return m;}
Vec3 LightingSystem::sunDirection(float t){float a=(t-.25f)*2*pi;return {std::cos(a),std::max(.08f,std::sin(a)),.25f};}
float LightingSystem::ambient(float t,float w){float night=1.f-clampf((std::sin((t-.25f)*2*pi)+.12f)/1.12f,0,1);return clampf((.18f+.62f*(1-night))*(1-clampf(w,0,1)*.25f),.08f,.85f);}
void MissionSystem::start(int id){state_={id,0,false};}
void MissionSystem::advance(){if(state_.id<=0||state_.complete)return;++state_.stage;if(state_.stage>=3)state_.complete=true;}
void MissionSystem::restore(MissionState s){state_=s;state_.stage=std::max(0,std::min(3,state_.stage));state_.complete=state_.complete||state_.stage>=3;}
static std::uint32_t u32(const std::uint8_t*p){return std::uint32_t(p[0])|(std::uint32_t(p[1])<<8)|(std::uint32_t(p[2])<<16)|(std::uint32_t(p[3])<<24);}
GlbInfo ModelLoader::inspectGlb(const std::uint8_t* b,std::size_t n){GlbInfo o{};if(!b||n<20||std::memcmp(b,"glTF",4)!=0)return o;o.version=u32(b+4);o.length=u32(b+8);if(o.version!=2||o.length>n||o.length<20)return o;std::size_t at=12;while(at+8<=o.length){auto len=u32(b+at),type=u32(b+at+4);at+=8;if(len>o.length-at)return {};if(type==0x4E4F534A)o.jsonBytes=len;else if(type==0x004E4942)o.binaryBytes=len;at+=len;}o.valid=at==o.length&&o.jsonBytes>0;return o;}
} // namespace iv::p8
