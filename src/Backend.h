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


// each variable will store the random variable of the object that was drawn
struct AreaConstants {
    // the randomType variable will be between 0 and 4, and will store the type of area (Building, store, facilities).
    int randomType;
    // the randomTexture variable, and will store the type of the first object in the area.
    int randomTexture;
    int random3;
    int random4;
    int random5;
};

/**
 * this will store the random variables of all the areas combined, meaning that all 4 areas inside 1 square.
 */
struct SquareConstants {
    AreaConstants area1;
    AreaConstants area2;
    AreaConstants area3;
    AreaConstants area4;
};

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

GLuint loadTexture(const char* filename);
unsigned int buildTextures();

void drawSquare(Shader shader, GLuint initialCube, GLuint sphere, float x, float z);
void buildBackground(Shader &shader, GLuint blueBigCube, float x, float z, float insideX, float insideZ);
AreaConstants buildBuilding(Shader &shader, GLuint initialCube, float x, float z, float insideX, float insideZ, int texture, int random3, int random4, int random5);

AreaConstants buildScrapers(Shader &shader, GLuint initialCube, float x, float z, float insideX, float insideZ, int texture, int random3, int random4, int random5);
void buildTree(Shader &shader, GLuint initialCube, float x, float z);
void buildStreetAndDecor(Shader &shader, GLuint initialCube, GLuint sphere, float x, float z);
AreaConstants buildShops(Shader &shader, GLuint initialCube, float x, float z, float insideX, float insideZ, int texture, int random3, int random4, int random5);
AreaConstants buildFountain(Shader &shader, GLuint initialCube, GLuint sphere, float x, float z, float insideX, float insideZ, int texture, int random3, int random4, int random5);
void buildPeople(Shader &shader, GLuint initialCube, float x, float z, float insideX, float insideZ);

void setShaderValues(Shader &shader);

void buildLightCube(Shader &shader, GLuint sphere);

bool collisionDetection();

AreaConstants drawArea(Shader shader, GLuint initialCube, GLuint sphere, float x, float z, float insideX, float insideZ, int type, int texture, int random3, int random4, int random5);

#endif //OPENGL_COMP371_PROJECT_BACKEND_H
