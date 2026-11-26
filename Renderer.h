#pragma once
#include "Game.h"
#include <GLES3/gl3.h>
#include <vector>
namespace iv {
class Renderer {
public:
 bool initialize(); void resize(int w,int h); void render(const Game& game,float time); void shutdown();
private:
 GLuint shader(GLenum,const char*); GLuint makeProgram(const char*,const char*);
 void add(const V3& p,const V3& s,float r,float g,float b,float yaw=0.0f);
 void addSphere(const V3& p,const V3& s,float r,float g,float b,float yaw=0.0f);
 void addCylinder(const V3& p,const V3& s,float r,float g,float b,float yaw=0.0f);
 GLuint program_{},vao_{},vbo_{},sphereVbo_{},cylinderVbo_{},instanceVbo_{}; GLint vp_{-1},camera_{-1},time_{-1};
 int qualityLevel_{}; float lowFpsTime_{},highFpsTime_{},lastQualityTime_{};
 int width_{1},height_{1}; float cameraX_{},cameraZ_{}; std::vector<float> instances_,sphereInstances_,cylinderInstances_;
 GLsizei sphereVertexCount_{},cylinderVertexCount_{};
};
}
