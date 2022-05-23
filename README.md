# MagmaVK
Simple library to begin a new vulkan application
it uses glm, and glfw
also here is a simple shader reader, also a code to calculate MVP matrix
DepthTest is writed
Have fun
A simple example using it:

#include <iostream>

#include "MagmaVK.hpp"

using namespace std;

using namespace glm;

MagmaVK engine;

float sensivity = 1000;

dvec2 rawm;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
   if (key == GLFW_KEY_W && action == GLFW_REPEAT){
        engine.pos.z = engine.pos.z + 0.1;
        engine.pos.x = engine.pos.x - engine.rot.x / 10;
    }
    if (key == GLFW_KEY_A && action == GLFW_REPEAT){
        engine.pos.x = engine.pos.x + 0.1;
        engine.pos.z = engine.pos.z + engine.rot.x / 10;
    }
    if (key == GLFW_KEY_S && action == GLFW_REPEAT){
        engine.pos.z = engine.pos.z - 0.1;
        engine.pos.x = engine.pos.x + engine.rot.x / 10;
    }
    if (key == GLFW_KEY_D && action == GLFW_REPEAT){
        engine.pos.x = engine.pos.x - 0.1;
        engine.pos.z = engine.pos.z - engine.rot.x / 10;
    }
}

int main(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    engine.window = glfwCreateWindow(engine.resolution.x, engine.resolution.y, "VK", NULL, NULL);
    engine.Init();
    double lastTime = glfwGetTime();
    int nbFrames = 0;
    string tittle;
    double currentTime;
    glfwSetInputMode(engine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    while(!glfwWindowShouldClose(engine.window)){
        glfwPollEvents();
        glfwSetKeyCallback(engine.window, key_callback);
        glfwGetCursorPos(engine.window, &rawm.x, &rawm.y);
        engine.rot.x = rawm.x / sensivity;
        engine.rot.y = -rawm.y / sensivity;
        if(engine.rot.x < -1.4){
            engine.rot.x = -1.38;
        }
        if(engine.rot.x > 1.4){
            engine.rot.x = 1.38;
        }
        engine.Draw();
        currentTime = glfwGetTime();
        nbFrames++;
        if ( currentTime - lastTime >= 1.0 ){
            tittle = to_string(nbFrames);
            glfwSetWindowTitle(engine.window, tittle.c_str());
            nbFrames = 0;
            lastTime += 1.0;
        }
    }    
    vkDeviceWaitIdle(engine.device);
    engine.Destroy();
    glfwDestroyWindow(engine.window);
}
