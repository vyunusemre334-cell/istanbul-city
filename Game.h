#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace iv {
struct V3 { float x{}, y{}, z{}; };
struct Building { V3 p, s; float shade{}; };
struct Pedestrian { V3 p; float phase{}, speed{}, lane{}, timer{}; int state{}; };
struct Vehicle { V3 p; float speed{}, phase{}, r{}, g{}, b{}; };
enum class AgentKind : std::uint8_t { Gang, Civilian, Police };
enum class AgentState : std::uint8_t { Patrol, Suspicious, Chase, Attack, Search, Flee };
struct Agent {
 V3 p{}, home{}, target{};
 AgentKind kind{AgentKind::Civilian}; AgentState state{AgentState::Patrol};
 float speed{1.0f}, phase{}, timer{}, think{}, attackCooldown{};
 int waypoint{};
};
struct Mission { int id{}; int stage{}; bool complete{}; };
struct MissionDefinition { int id{}; std::string title; V3 eventZone{}, safeZone{}; float eventRadius{9.0f}, safeRadius{9.0f}; };

class Game {
public:
 void initialize(); void update(float dt);
 void setMove(float x,float y); void setSprint(bool); void setCrouch(bool); void jump(); void interact(); void toggleMission(); void toggleVehicle();
 void registerMission(const MissionDefinition& definition); bool startMission(int id);
 bool driving() const noexcept { return driving_; }
 float vehicleSpeed() const noexcept { return carSpeed_; }
 float vehicleHeading() const noexcept { return carHeading_; }
 bool grounded() const noexcept { return grounded_; }
 bool crouching() const noexcept { return crouching_; }
 bool sprinting() const noexcept { return sprinting_; }
 bool moving() const noexcept { return moveX_*moveX_+moveY_*moveY_>.025f; }
 float facing() const noexcept { return facing_; }
 float fps() const noexcept { return fps_; }
 float health() const noexcept { return health_; }
 int missionStage() const noexcept { return mission_.stage; }
 bool missionComplete() const noexcept { return mission_.complete; }
 int gangCount() const noexcept;
 const V3& player() const noexcept { return player_; } const V3& car() const noexcept { return car_; }
 V3 missionObjective() const noexcept;
 const std::vector<Vehicle>& traffic() const noexcept { return traffic_; }
 const std::vector<Building>& buildings() const noexcept { return buildings_; }
 const std::vector<Pedestrian>& pedestrians() const noexcept { return pedestrians_; }
 const std::vector<Agent>& agents() const noexcept { return agents_; }
 void loadProgress(int stage,int missionId=1); int saveProgress() const noexcept { return mission_.stage; } int saveMissionId() const noexcept { return mission_.id; } int missionId() const noexcept { return mission_.id; }
private:
 void updateAgent(Agent&,float); void updateMission(); void chooseWaypoint(Agent&); const MissionDefinition* activeMission() const; bool near(const V3&,const V3&,float) const;
 V3 player_{0,0,2}, car_{3,0,8}; float moveX_{},moveY_{},clock_{},carHeading_{},carSpeed_{},verticalVelocity_{},facing_{},health_{100},fps_{60},fpsTimer_{},fpsAccum_{};
 bool driving_{},grounded_{true},sprinting_{},crouching_{}; float jumpLock_{},playerAttackCooldown_{};
 Mission mission_{}; std::vector<MissionDefinition> missionDefinitions_; float wanted_{},policeSearchTimer_{};
 std::vector<Building> buildings_; std::vector<Pedestrian> pedestrians_; std::vector<Vehicle> traffic_; std::vector<Agent> agents_;
};
}
