#include "Game.h"
#include "Part8Systems.h"
#include <cassert>
#include <cmath>
#include <iostream>
int main(){
 iv::Game g;g.initialize();assert(g.missionStage()==0);assert(g.agents().size()==18);assert(g.gangCount()==5);float gangStart=g.agents().front().p.x;for(int i=0;i<80;++i){g.update(.05f);}assert(std::abs(g.agents().front().p.x-gangStart)>.01f);
 g.jump();assert(!g.grounded());g.jump(); // A second jump while airborne must be ignored.
 for(int i=0;i<100;++i){g.update(.016f);} assert(g.grounded()); assert(std::abs(g.player().y)<.001f); g.jump(); assert(!g.grounded()); for(int i=0;i<100;++i){g.update(.016f);} assert(g.grounded());
 g.setCrouch(true);assert(g.crouching());g.setSprint(true);assert(!g.sprinting());g.setCrouch(false);g.setSprint(true);assert(g.sprinting());
 g.setSprint(false);g.registerMission({99,"Test Mission",{5,0,5},{-5,0,-5},3,3});assert(g.startMission(99));assert(g.missionStage()==0);assert(!g.startMission(100));g.startMission(1);g.setMove(.7071f,.7071f);for(int i=0;i<170;++i)g.update(.05f);assert(g.missionStage()>=1);
 g.interact();assert(g.missionStage()==2);
 g.setMove(-.7071f,-.7071f);for(int i=0;i<390;++i)g.update(.05f);assert(g.missionComplete());assert(g.saveProgress()==3);
 g.toggleMission();assert(g.missionId()==2);assert(g.missionStage()==0);assert(std::abs(g.missionObjective().z+72.0f)<.001f);g.toggleMission();assert(g.missionId()==2);iv::Game restored;restored.initialize();restored.loadProgress(g.saveProgress(),g.saveMissionId());assert(restored.missionId()==2);assert(restored.missionStage()==0);
 iv::Game carTest;carTest.initialize();carTest.toggleVehicle();assert(carTest.driving());const float carStartZ=carTest.car().z;carTest.setMove(0,1);for(int i=0;i<40;++i)carTest.update(.05f);assert(carTest.car().z>carStartZ+1.0f);carTest.setMove(1,0);for(int i=0;i<20;++i)carTest.update(.05f);carTest.setMove(0,0);carTest.toggleVehicle();assert(!carTest.driving());
 std::cout<<"Istanbul City Part 7 smoke tests passed\n";
}
