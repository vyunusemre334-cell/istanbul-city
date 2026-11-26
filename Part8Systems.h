#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <cstddef>
#include <vector>

namespace iv::p8 {
struct Vec3 { float x{},y{},z{}; };
struct Transform { Vec3 position{}; Vec3 rotation{}; Vec3 scale{1,1,1}; };

enum class Motion { Idle, Walk, Run, Sprint, Jump, Crouch, Attack, EnterVehicle, Drive, ExitVehicle, Dead };
struct BonePose { Vec3 translation{}; Vec3 rotation{}; };
class AnimationSystem {
public:
 static constexpr int BoneCount=16;
 void setMotion(Motion m) { motion_=m; time_=0; }
 void update(float dt,float speed,float grounded);
 const std::array<BonePose,BoneCount>& pose() const { return pose_; }
 Motion motion() const { return motion_; }
private: Motion motion_{Motion::Idle}; float time_{}; std::array<BonePose,BoneCount> pose_{};
};

struct VehicleInput { float throttle{},steer{}; bool brake{}; };
struct VehicleState { Vec3 position{}; float heading{},speed{},wheelAngle{},damage{}; };
class VehicleSystem {
public:
 void reset(Vec3 p={});
 void update(float dt,const VehicleInput& in);
 void collide(float impact);
 const VehicleState& state() const { return state_; }
private: VehicleState state_{};
};

class CharacterController {
public:
 void setInput(float x,float z,bool sprint,bool crouch);
 void jump();
 void update(float dt);
 Vec3 position() const { return p_; }
 float verticalSpeed() const { return vy_; }
 bool grounded() const { return grounded_; }
private: Vec3 p_{}; float x_{},z_{},vy_{}; bool sprint_{},crouch_{},grounded_{true};
};

struct CameraPose { Vec3 position{},target{}; };
class CameraSystem {
public:
 void setTarget(Vec3 target,float heading,bool vehicle);
 void update(float dt);
 const CameraPose& pose() const { return pose_; }
 void setDistance(float d);
private: Vec3 target_{}; float heading_{},distance_{5.2f}; bool vehicle_{}; CameraPose pose_{};
};

class CrimeSystem {
public:
 void addCrime(float severity);
 void triggerAlarm(float severity=1.f);
 void update(float dt,bool policeNearby);
 float wanted() const { return wanted_; }
 int stars() const;
private: float wanted_{};
};

class PhysicsSystem {
public:
 static bool resolveAabb(Vec3& position,Vec3& velocity,Vec3 halfExtents,
                         Vec3 boxCenter,Vec3 boxHalfExtents,float restitution=.12f);
};

class WorldSystem {
public:
 enum class Weather { Clear, Cloudy, Fog, Rain };
 void update(float dt);
 void setWeather(Weather w) { weather_=w; }
 float dayTime() const { return dayTime_; }
 float daylight() const;
 float wetness() const { return wetness_; }
 Weather weather() const { return weather_; }
private: float dayTime_{.28f},wetness_{}; Weather weather_{Weather::Clear};
};

struct NpcAgent { Vec3 position{},target{}; float speed{1.f}; bool fleeing{}; };
class NPCSystem {
public:
 void update(NpcAgent& npc,float dt,Vec3 threat);
};

struct PbrMaterial { float metallic{},roughness{.8f}; Vec3 albedo{1,1,1}; };
class MaterialSystem {
public: static PbrMaterial sanitize(PbrMaterial m);
};
class LightingSystem {
public: static Vec3 sunDirection(float dayTime);
 static float ambient(float dayTime,float weatherFactor);
};

struct MissionState { int id{},stage{}; bool complete{}; };
class MissionSystem {
public:
 void start(int id);
 void advance();
 void restore(MissionState s);
 MissionState state() const { return state_; }
private: MissionState state_{};
};

// Validates the GLB container header/chunk boundaries before an asset is handed
// to a renderer. It deliberately does not claim to decode meshes or upload GPU buffers.
struct GlbInfo { bool valid{}; std::uint32_t version{},length{}; std::uint32_t jsonBytes{},binaryBytes{}; };
class ModelLoader {
public: static GlbInfo inspectGlb(const std::uint8_t* bytes,std::size_t size);
};
} // namespace iv::p8
