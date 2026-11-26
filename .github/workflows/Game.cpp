#include "Game.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace iv {
namespace { float sq(float v){return v*v;} }
int Game::gangCount() const noexcept { int count=0; for(const auto& a:agents_) if(a.kind==AgentKind::Gang) ++count; return count; }
V3 Game::missionObjective() const noexcept { const auto* m=activeMission(); if(!m)return {}; return mission_.stage>=2?m->safeZone:m->eventZone; }
bool Game::near(const V3&a,const V3&b,float r) const { return sq(a.x-b.x)+sq(a.z-b.z)<=r*r; }
void Game::initialize(){
 buildings_.clear();pedestrians_.clear();traffic_.clear();agents_.clear();
 player_={0,0,2};car_={3,0,8};moveX_=moveY_=clock_=carHeading_=carSpeed_=verticalVelocity_=facing_=0;health_=100;fps_=60;fpsTimer_=fpsAccum_=0;
 driving_=false;grounded_=true;sprinting_=crouching_=false;jumpLock_=playerAttackCooldown_=wanted_=policeSearchTimer_=0;missionDefinitions_.clear();missionDefinitions_.push_back({1,"Mor Bölge",{21,0,19},{-28,0,-18},9.0f,9.0f});missionDefinitions_.push_back({2,"Sahil Operasyonu",{0,0,-72},{72,0,36},10.0f,10.0f});missionDefinitions_.push_back({3,"Köprü Hattı",{-72,0,36},{0,0,72},10.0f,10.0f});mission_={1,0,false};
 std::mt19937 rng(91273);std::uniform_real_distribution<float> h(5,16),tint(0,1);
 constexpr float block=18;
 for(int z=-5;z<=5;++z)for(int x=-5;x<=5;++x){if(x==0||z==0||((x+z)%5==0))continue;float height=h(rng);buildings_.push_back({{x*block+6.5f,height*.5f,z*block+6.5f},{5.5f,height,5.5f},tint(rng)});}
 for(int i=0;i<12;++i)pedestrians_.push_back({{-18.0f+float(i)*3.1f,0,-7.0f-float(i%4)*3.1f},float(i)*.63f,.55f+.08f*float(i%5),float(i%2?1:-1),0,0});
 traffic_={{ {-26,0,-9},2.5f,0,.75f,.18f,.12f },{{22,0,9},-2.0f,1,.12f,.48f,.78f},{{-48,0,-9},1.8f,2,.82f,.72f,.16f}};
 // Purple rival crew patrols a compact territory east of the starting block.
 for(int i=0;i<5;++i){Agent a;a.kind=AgentKind::Gang;a.p=a.home={18.0f+float(i%3)*3.2f,0,15.0f+float(i/3)*5.0f};a.speed=1.15f+.12f*float(i%3);a.phase=float(i)*1.7f;a.target={a.home.x+4,0,a.home.z-3};agents_.push_back(a);}
 for(int i=0;i<10;++i){Agent a;a.kind=AgentKind::Civilian;a.p=a.home={-24.0f+float(i%5)*8.0f,0,-18.0f+float(i/5)*12.0f};a.speed=.65f+.12f*float(i%4);a.phase=float(i)*.9f;a.target={a.home.x+5,0,a.home.z+4};agents_.push_back(a);}
 for(int i=0;i<3;++i){Agent a;a.kind=AgentKind::Police;a.p=a.home={-12.0f+float(i)*12.0f,0,-28.0f};a.speed=1.65f;a.phase=float(i)*2.1f;a.target={a.home.x+8,0,a.home.z};agents_.push_back(a);}
}
void Game::setMove(float x,float y){float len=std::sqrt(x*x+y*y);if(len>1){x/=len;y/=len;}moveX_=x;moveY_=y;}
void Game::setSprint(bool v){sprinting_=v&&!crouching_&&!driving_;}
void Game::setCrouch(bool v){crouching_=v&&!driving_;if(crouching_)sprinting_=false;}
void Game::jump(){if(!driving_&&grounded_&&jumpLock_<=0){verticalVelocity_=6.2f;grounded_=false;jumpLock_=.18f;}}
void Game::interact(){if(driving_){toggleVehicle();return;}const auto*m=activeMission();if(m&&mission_.stage==1&&near(player_,m->eventZone,m->eventRadius)){mission_.stage=2;wanted_=1.0f;}else if(near(player_,car_,4))toggleVehicle();}
void Game::toggleMission(){ if(mission_.complete){int next=mission_.id%3+1;mission_={next,0,false};wanted_=0;health_=std::max(health_,65.0f);} }
void Game::registerMission(const MissionDefinition& definition){if(definition.id<=0)return;for(auto&d:missionDefinitions_)if(d.id==definition.id){d=definition;return;}missionDefinitions_.push_back(definition);}
const MissionDefinition* Game::activeMission() const {for(const auto&d:missionDefinitions_)if(d.id==mission_.id)return &d;return missionDefinitions_.empty()?nullptr:&missionDefinitions_.front();}
bool Game::startMission(int id){for(const auto&d:missionDefinitions_)if(d.id==id){mission_={id,0,false};wanted_=0;return true;}return false;}
void Game::loadProgress(int stage,int missionId){bool found=false;for(const auto&d:missionDefinitions_)if(d.id==missionId)found=true;mission_.id=found?missionId:1;mission_.stage=std::clamp(stage,0,3);mission_.complete=mission_.stage>=3;wanted_=mission_.stage==2?1.0f:0.0f;}
void Game::toggleVehicle(){if(!driving_){if(near(player_,car_,10)){driving_=true;player_=car_;sprinting_=crouching_=false;}}else{driving_=false;player_.x=car_.x+2;player_.z=car_.z+1.5f;carSpeed_=0;}}
void Game::chooseWaypoint(Agent&a){const float ang=a.phase+float(a.waypoint)*2.399f;float radius=a.kind==AgentKind::Gang?5.0f:8.0f;a.target={a.home.x+std::cos(ang)*radius,0,a.home.z+std::sin(ang)*radius};++a.waypoint;}
void Game::updateMission(){const auto*m=activeMission();if(!m)return;if(mission_.stage==0&&near(player_,m->eventZone,m->eventRadius+1.0f))mission_.stage=1;if(mission_.stage==2&&near(player_,m->safeZone,m->safeRadius)){mission_.stage=3;mission_.complete=true;wanted_=0;}}
void Game::updateAgent(Agent&a,float dt){
 const float d2=sq(a.p.x-player_.x)+sq(a.p.z-player_.z);const float d=std::sqrt(d2);a.attackCooldown=std::max(0.0f,a.attackCooldown-dt);a.timer+=dt;
 if(a.kind==AgentKind::Gang){
  if(d<15&&a.state!=AgentState::Attack)a.state=AgentState::Chase;
  if(a.state==AgentState::Chase){a.target=player_;if(d<2.25f){a.state=AgentState::Attack;a.timer=0;}}
  else if(a.state==AgentState::Attack){a.target=player_;if(d>3.4f)a.state=AgentState::Chase;else if(a.attackCooldown<=0){health_=std::max(0.0f,health_-3.0f);a.attackCooldown=1.15f;} }
  if(d>25){a.state=AgentState::Patrol;if(a.timer>3){chooseWaypoint(a);a.timer=0;}}
 } else if(a.kind==AgentKind::Police){
  if(wanted_>0.1f&&d<42){if(a.state!=AgentState::Suspicious&&a.state!=AgentState::Chase){a.state=AgentState::Suspicious;a.timer=0;}if(a.state==AgentState::Suspicious&&a.timer>1.0f)a.state=AgentState::Chase;if(a.state==AgentState::Chase)a.target=player_;}
  else if(wanted_>0.1f&&d>=42){if(a.state!=AgentState::Search){a.state=AgentState::Search;a.timer=0;}}
  else if(a.state==AgentState::Search&&a.timer>8)a.state=AgentState::Patrol;
  if(a.state==AgentState::Chase&&d<2.8f&&a.attackCooldown<=0){health_=std::max(0.0f,health_-1.0f);a.attackCooldown=1.5f;}
  if(a.state==AgentState::Patrol&&a.timer>4){chooseWaypoint(a);a.timer=0;}
  if(a.state==AgentState::Search){a.target={player_.x+std::sin(clock_+a.phase)*9,0,player_.z+std::cos(clock_+a.phase)*9};}
 }else{
  if(d<6.5f){a.state=AgentState::Flee;float dx=a.p.x-player_.x,dz=a.p.z-player_.z;float l=std::max(.01f,std::sqrt(dx*dx+dz*dz));a.target={a.p.x+dx/l*10,0,a.p.z+dz/l*10};}
  else if(a.state==AgentState::Flee&&a.timer>2.5f){a.state=AgentState::Patrol;a.timer=0;chooseWaypoint(a);}
  else if(a.state==AgentState::Patrol&&a.timer>4){chooseWaypoint(a);a.timer=0;}
 }
 float dx=a.target.x-a.p.x,dz=a.target.z-a.p.z,dist=std::sqrt(dx*dx+dz*dz);
 if(dist>.25f){float speed=a.speed*(a.state==AgentState::Flee?1.45f:(a.state==AgentState::Chase?1.35f:1.0f));float step=std::min(dist,speed*dt);a.p.x+=dx/dist*step;a.p.z+=dz/dist*step;}
 // Cheap local separation keeps agents from occupying the same point.
 for(const auto&o:agents_){if(&o==&a)continue;float ox=a.p.x-o.p.x,oz=a.p.z-o.p.z,r2=ox*ox+oz*oz;if(r2>0.001f&&r2<1.8f){float inv=1.0f/std::sqrt(r2);a.p.x+=ox*inv*dt*1.4f;a.p.z+=oz*inv*dt*1.4f;}}
 for(const auto&v:traffic_){float vx=a.p.x-v.p.x,vz=a.p.z-v.p.z,r2=vx*vx+vz*vz;if(r2>0.001f&&r2<8.0f){float inv=1.0f/std::sqrt(r2);a.p.x+=vx*inv*dt*2.2f;a.p.z+=vz*inv*dt*2.2f;}}
 a.p.x=std::clamp(a.p.x,-92.0f,92.0f);a.p.z=std::clamp(a.p.z,-92.0f,92.0f);
}
void Game::update(float dt){
 dt=std::clamp(dt,0.0f,.05f);clock_+=dt;fpsAccum_+=dt;++fpsTimer_;if(fpsAccum_>=.5f){fps_=float(fpsTimer_)/fpsAccum_;fpsAccum_=0;fpsTimer_=0;}jumpLock_=std::max(0.0f,jumpLock_-dt);playerAttackCooldown_=std::max(0.0f,playerAttackCooldown_-dt);
 if(driving_){
  // Joystick up accelerates; down brakes/reverses. Steering scales with speed.
  const float throttle=moveY_;
  if(throttle>.12f)carSpeed_+=10.5f*throttle*dt;
  else if(throttle<-.12f)carSpeed_+=8.5f*throttle*dt;
  else carSpeed_*=std::max(0.0f,1.0f-1.15f*dt);
  carSpeed_=std::clamp(carSpeed_,-4.0f,18.0f);
  const float steering=std::clamp(std::abs(carSpeed_)/5.0f,.18f,1.0f);
  carHeading_+=moveX_*1.55f*steering*dt*(carSpeed_>=0?1.0f:-1.0f);
  const float oldX=car_.x,oldZ=car_.z;
  car_.x+=std::sin(carHeading_)*carSpeed_*dt;
  car_.z+=std::cos(carHeading_)*carSpeed_*dt;
  car_.x=std::clamp(car_.x,-103.0f,103.0f);
  car_.z=std::clamp(car_.z,-103.0f,103.0f);
  // Lightweight solid-building collision response.
  for(const auto& b:buildings_){
   const float halfX=b.s.x*.5f+1.25f,halfZ=b.s.z*.5f+1.9f;
   if(std::abs(car_.x-b.p.x)<halfX&&std::abs(car_.z-b.p.z)<halfZ){
    car_.x=oldX;car_.z=oldZ;carSpeed_=-carSpeed_*.18f;break;
   }
  }
  player_=car_;
 }
 else{
  const float input=std::sqrt(moveX_*moveX_+moveY_*moveY_);float speed=crouching_?1.65f:(sprinting_?7.0f:4.2f);if(!grounded_)speed*=.62f;
  if(input>.03f){facing_=std::atan2(moveX_,moveY_);const float oldX=player_.x,oldZ=player_.z;float nx=oldX+moveX_*speed*dt,nz=oldZ+moveY_*speed*dt;
   // Solid building footprints with axis sliding keep the character on streets and pavements.
   auto blocked=[&](float x,float z){for(const auto&b:buildings_)if(std::abs(x-b.p.x)<b.s.x*.5f+.42f&&std::abs(z-b.p.z)<b.s.z*.5f+.42f)return true;return false;};
   if(!blocked(nx,nz)){player_.x=nx;player_.z=nz;}else{if(!blocked(nx,oldZ))player_.x=nx;if(!blocked(player_.x,nz))player_.z=nz;}
  }
  player_.x=std::clamp(player_.x,-95.0f,95.0f);player_.z=std::clamp(player_.z,-95.0f,95.0f);
  if(!grounded_){verticalVelocity_-=16.5f*dt;player_.y+=verticalVelocity_*dt;if(player_.y<=0){player_.y=0;verticalVelocity_=0;grounded_=true;}}
 }
 for(auto&n:pedestrians_){n.timer+=dt;n.p.x+=std::sin(clock_+n.phase)*n.speed*dt;n.p.z+=n.lane*n.speed*.18f*dt;for(const auto&v:traffic_){float vx=n.p.x-v.p.x,vz=n.p.z-v.p.z,r2=vx*vx+vz*vz;if(r2>0.001f&&r2<7.0f){float inv=1.0f/std::sqrt(r2);n.p.x+=vx*inv*dt*2.0f;n.p.z+=vz*inv*dt*2.0f;}}if(n.p.z>5)n.p.z=-20;if(n.p.z< -22)n.p.z=4;}
 for(auto&v:traffic_){v.p.x+=v.speed*dt;if(v.p.x>55)v.p.x=-55;if(v.p.x< -55)v.p.x=55;}
 // Distance-tiered AI: nearby actors think at 10 Hz; distant actors at ~2.5 Hz.
 for(auto&a:agents_){float d2=sq(a.p.x-player_.x)+sq(a.p.z-player_.z);if(d2>65.0f*65.0f)continue;a.think-=dt;if(a.think<=0){float step=d2<25.0f*25.0f?.10f:.38f;a.think=step;updateAgent(a,step);}}
 updateMission();if(wanted_>0&&mission_.stage!=2)wanted_=std::max(0.0f,wanted_-dt*.012f);
 if(health_<=0.0f){health_=100.0f;player_={0,0,2};driving_=false;carSpeed_=0.0f;wanted_=0.0f;sprinting_=crouching_=false;}
}
}
