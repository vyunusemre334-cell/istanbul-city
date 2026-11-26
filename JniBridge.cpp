#include "Renderer.h"
#include <jni.h>
#include <chrono>
#include <algorithm>
#include <mutex>
namespace { iv::Renderer renderer;iv::Game game;bool ready=false;std::mutex stateMutex;auto last=std::chrono::steady_clock::now();float clockSeconds=0; }
extern "C" JNIEXPORT void JNICALL Java_com_istanbulvice_underground_MainActivity_nativeInit(JNIEnv*,jclass,jint savedStage,jint savedMissionId){std::lock_guard<std::mutex> lock(stateMutex);ready=renderer.initialize();game.initialize();game.loadProgress(savedStage,savedMissionId);last=std::chrono::steady_clock::now();clockSeconds=0;}
extern "C" JNIEXPORT void JNICALL Java_com_istanbulvice_underground_MainActivity_nativeResize(JNIEnv*,jclass,jint w,jint h){std::lock_guard<std::mutex> lock(stateMutex);if(ready)renderer.resize(w,h);}
extern "C" JNIEXPORT void JNICALL Java_com_istanbulvice_underground_MainActivity_nativeFrame(JNIEnv*,jclass){std::lock_guard<std::mutex> lock(stateMutex);if(!ready)return;auto now=std::chrono::steady_clock::now();float dt=std::chrono::duration<float>(now-last).count();last=now;dt=std::clamp(dt,0.0f,.05f);clockSeconds+=dt;game.update(dt);renderer.render(game,clockSeconds);}
extern "C" JNIEXPORT void JNICALL Java_com_istanbulvice_underground_MainActivity_nativeMove(JNIEnv*,jclass,jfloat x,jfloat y){std::lock_guard<std::mutex> lock(stateMutex);game.setMove(x,y);}
extern "C" JNIEXPORT void JNICALL Java_com_istanbulvice_underground_MainActivity_nativeButton(JNIEnv*,jclass,jint code,jboolean down){std::lock_guard<std::mutex> lock(stateMutex);switch(code){case 0:game.setSprint(down);break;case 1:if(down)game.jump();break;case 2:game.setCrouch(down);break;case 3:if(down)game.interact();break;case 4:if(down)game.toggleMission();break;default:break;}}
extern "C" JNIEXPORT jint JNICALL Java_com_istanbulvice_underground_MainActivity_nativeGetMissionId(JNIEnv*,jclass){std::lock_guard<std::mutex> lock(stateMutex);return game.missionId();}
extern "C" JNIEXPORT jint JNICALL Java_com_istanbulvice_underground_MainActivity_nativeGetMissionStage(JNIEnv*,jclass){std::lock_guard<std::mutex> lock(stateMutex);return game.missionStage();}
extern "C" JNIEXPORT jfloat JNICALL Java_com_istanbulvice_underground_MainActivity_nativeGetFps(JNIEnv*,jclass){std::lock_guard<std::mutex> lock(stateMutex);return game.fps();}
extern "C" JNIEXPORT jboolean JNICALL Java_com_istanbulvice_underground_MainActivity_nativeGetDriving(JNIEnv*,jclass){std::lock_guard<std::mutex> lock(stateMutex);return game.driving()?JNI_TRUE:JNI_FALSE;}
extern "C" JNIEXPORT jfloat JNICALL Java_com_istanbulvice_underground_MainActivity_nativeGetSpeed(JNIEnv*,jclass){std::lock_guard<std::mutex> lock(stateMutex);return game.vehicleSpeed();}
extern "C" JNIEXPORT jfloat JNICALL Java_com_istanbulvice_underground_MainActivity_nativeGetHealth(JNIEnv*,jclass){std::lock_guard<std::mutex> lock(stateMutex);return game.health();}
