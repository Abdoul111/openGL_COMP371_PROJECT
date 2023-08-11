//
// Created by zizoo on 8/10/2023.
//


#include <iostream>

#define GLEW_STATIC 1
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#ifndef OPENGL_COMP371_PROJECT_BACKEND_H
#define OPENGL_COMP371_PROJECT_BACKEND_H


void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

GLuint loadTexture(const char* filename);
unsigned int buildTextures();

void drawSquare(Shader shader, GLuint initialCube, GLuint blueBigCube, GLuint sphere, float x, float z);
void buildBackground(Shader &shader, GLuint blueBigCube, float x, float z);
void buildBuildingABCD(Shader &shader, GLuint initialCube, float x, float z);

void buildScrapers(Shader &shader, GLuint initialCube, float x, float z);
void buildTree(Shader &shader, GLuint initialCube, float x, float z);
void buildStreetAndDecor(Shader &shader, GLuint initialCube, GLuint sphere, float x, float z);
void buildShops(Shader &shader, GLuint initialCube, float x, float z);
void buildFountain(Shader &shader, GLuint initialCube, GLuint sphere, float x, float z);
void buildPeople(Shader &shader, GLuint initialCube, float x, float z);

void setShaderValues(Shader &shader);

void buildLightCube(Shader &shader, GLuint sphere);

bool collisionDetection();


#endif //OPENGL_COMP371_PROJECT_BACKEND_H
