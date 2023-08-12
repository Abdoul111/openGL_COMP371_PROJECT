
#include <iostream>
#include <map>
#define GLEW_STATIC 1
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"
#include "lib/Camera.h"
#include "lib/shader.h"
#include "Backend.h"

using namespace glm;
using namespace std;


const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;
bool noShadows = false;
bool noShadowsKeyPressed = true;

vec3 initialCameraPos = vec3(10.0f, 4.0f, 0.0f);
vec3 lightPos(0.0f, 6.0f, 0.0f);
Camera camera(initialCameraPos);
float lastX = (float) SCR_WIDTH / 2.0f;
float lastY = (float) SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float near_plane = 1.0f;
// change the max distance of light
float far_plane  = 500.0f;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;



struct Wall {
    float maxX;
    float maxY;
    float maxZ;
    float minX;
    float minY;
    float minZ;
};


Wall bigWall = {125.0f, 125.0f, 125.0f,
                -125.0f, -1.0f, -125.0f};


mat4 translateMatrix;
mat4 rotateMatrix;
mat4 scaleMatrix;
mat4 modelMatrix;

// now we need to keep track of the already drawn objects so that the scene does not get
// changed every second
struct Position {
    float x;
    float z;

    bool operator<(const Position& other) const {
        if (x < other.x) return true;
        if (x > other.x) return false;
        return z < other.z;
    }

};


map<Position, SquareConstants> drawnSquares;

// textures variables
unsigned int buildingTextures[4];unsigned int balloonTextures[2];unsigned int shopTextures[8];unsigned int scraperTextures[6];
unsigned int fountainTextures[3];unsigned int peopleTextures[8];
unsigned int backgroundTexture;unsigned int treesTexture[2];unsigned int woodTexture;unsigned int streetTexture;
unsigned int floorTexture;unsigned int firehydrantTexture;unsigned int stopTexture;
unsigned int signsTexture[5];unsigned int antennaTexture;

// light sphere constants:
// light sphere
int resolution = 65;
int vertexCount = 6 * (resolution / 2) * resolution + 1;

// positions of the point lights
glm::vec3 pointLightPositions[] = {
        glm::vec3( 0.0f,  5.0f,  50.0f),
        glm::vec3( 0.0f, 5.0f, -50.0f)
};

bool isNight = false;
bool isSpotLightOn = false;


void drawSquare(Shader shader, GLuint initialCube, GLuint sphere, float x, float z) {

    buildTree(shader, initialCube, x, z);
    buildStreetAndDecor(shader, initialCube, sphere, x, z);

    if (drawnSquares.find(Position{x, z}) != drawnSquares.end()) {
        AreaConstants area1 = drawArea(shader, initialCube, sphere, x, z, 12.5, 12.5, drawnSquares[Position{x, z}].area1.random1);
        AreaConstants area2 = drawArea(shader, initialCube, sphere, x, z, -12.5, 12.5, drawnSquares[Position{x, z}].area2.random1);
        AreaConstants area3 = drawArea(shader, initialCube, sphere, x, z, 12.5, -12.5, drawnSquares[Position{x, z}].area3.random1);
        AreaConstants area4 = drawArea(shader, initialCube, sphere, x, z, -12.5, -12.5, drawnSquares[Position{x, z}].area4.random1);
    } else {
        AreaConstants area1 = drawArea(shader, initialCube, sphere, x, z, 12.5, 12.5, 0);
        AreaConstants area2 = drawArea(shader, initialCube, sphere, x, z, -12.5, 12.5, 0);
        AreaConstants area3 = drawArea(shader, initialCube, sphere, x, z, 12.5, -12.5, 0);
        AreaConstants area4 = drawArea(shader, initialCube, sphere, x, z, -12.5, -12.5, 0);

        drawnSquares.insert(pair<Position, SquareConstants>(Position{x, z}, SquareConstants{area1, area2, area3, area4}));
    }

}

AreaConstants drawArea(Shader shader, GLuint initialCube, GLuint sphere, float x, float z, float insideX, float insideZ, int type) {

    // generate random number between 1 and 4
    int random1 = !type ? ((rand() % 4) + 1) : type;
    AreaConstants area;
    if (random1 == 1) {
        // make it a building
        area = buildBuilding(shader, initialCube, x, z, insideX, insideZ);
    } else if (random1 == 2) {
        // make it a shop
        area = buildShops(shader, initialCube, x, z, insideX, insideZ);
    } else if (random1 == 3) {
        area = buildScrapers(shader, initialCube, x, z, insideX, insideZ);
    } else if (random1 == 4) {
        area = buildFountain(shader, initialCube, sphere, x, z, insideX, insideZ);
    }

    return area;
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

GLuint loadTexture(const char* filename) {
    // Step1 Create and bind textures
    GLuint textureId = 0;
    glGenTextures(1, &textureId);
    assert(textureId != 0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    // Step2 Set filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Step3 Load Textures with dimension data
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data)
    {
        std::cerr << "Error::Texture could not load texture file:" << filename << std::endl;
        return 0;
    }
    // Step4 Upload the texture to the PU
    GLenum format = 0;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height,
                 0, format, GL_UNSIGNED_BYTE, data);
    // Step5 Free resources
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureId;
}


unsigned int buildTextures() {
    buildingTextures[0] = loadTexture("rec/textures/building.png");buildingTextures[1] = loadTexture("rec/textures/buildingB.png");
    buildingTextures[2] = loadTexture("rec/textures/buildingC.png");buildingTextures[3] = loadTexture("rec/textures/buildingD.png");
    backgroundTexture = loadTexture("rec/textures/background.png");
    treesTexture[0] = loadTexture("rec/textures/tree.png");treesTexture[1] = loadTexture("rec/textures/tree2.png");
    woodTexture = loadTexture("rec/textures/wood.png");
    streetTexture = loadTexture("rec/textures/street.png");
    floorTexture = loadTexture("rec/textures/brick.jpg");
    scraperTextures[0] = loadTexture("rec/textures/scraper.png");scraperTextures[1] = loadTexture("rec/textures/scraper2.png");
    scraperTextures[2] = loadTexture("rec/textures/scraper3.png");scraperTextures[3] = loadTexture("rec/textures/scraper4.png");
    scraperTextures[4] = loadTexture("rec/textures/scraper5.png");scraperTextures[5] = loadTexture("rec/textures/scraper6.png");
    firehydrantTexture = loadTexture("rec/textures/firehydrant.png");
    stopTexture = loadTexture("rec/textures/stop.png");
    shopTextures[0] = loadTexture("rec/textures/JC.png");shopTextures[1] = loadTexture("rec/textures/MC.png");
    shopTextures[2] = loadTexture("rec/textures/TH.png");shopTextures[3] = loadTexture("rec/textures/SB.png");
    shopTextures[4] = loadTexture("rec/textures/FS.png");shopTextures[5] = loadTexture("rec/textures/HQ.png");
    shopTextures[6] = loadTexture("rec/textures/police.png");shopTextures[7] = loadTexture("rec/textures/RBC.png");
    balloonTextures[0] = loadTexture("rec/textures/balloon.png");balloonTextures[1] = loadTexture("rec/textures/blimp.png");
    signsTexture[0] = loadTexture("rec/textures/hey.png");signsTexture[1] = loadTexture("rec/textures/adventure.png");
    signsTexture[2] = loadTexture("rec/textures/haveyou.png");signsTexture[3] = loadTexture("rec/textures/visit.png");
    signsTexture[4] = loadTexture("rec/textures/welcome.png");
    fountainTextures[0] = loadTexture("rec/textures/fountainbase.png");fountainTextures[1] = loadTexture("rec/textures/fountaintop.png");
    fountainTextures[2] = loadTexture("rec/textures/water.png");
    antennaTexture = loadTexture("rec/textures/antenna.png");
    peopleTextures[0] = loadTexture("rec/textures/cashier.png");peopleTextures[1] = loadTexture("rec/textures/mcworker.png");
    peopleTextures[2] = loadTexture("rec/textures/timemployee.png");peopleTextures[3] = loadTexture("rec/textures/SBemployee.png");
    peopleTextures[4] = loadTexture("rec/textures/fireman.png"); peopleTextures[5] = loadTexture("rec/textures/electrician.png");
    peopleTextures[6] = loadTexture("rec/textures/officer.png");peopleTextures[7] = loadTexture("rec/textures/banker.png");

    return backgroundTexture;
}

void buildBackground(Shader &shader, GLuint blueBigCube, float x, float z) {
    glBindVertexArray(blueBigCube);// Big Blue Cube
    translateMatrix = translate(mat4(1.0f), vec3(0.0f + x, 69.8f, 0.0f + z));
    rotateMatrix = rotate(mat4(1.0f), radians(0.0f), vec3(0.0f, 0.0f, 1.0f));
    scaleMatrix = scale(mat4(1.0f), vec3(500.0f, 70.0f, 500.0f));
    modelMatrix = translateMatrix * rotateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    //shader.setVec3("color", 129.0f/255, 174.0f/255, 208.0f/255);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
    // bind specular map
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
    glDisable(GL_CULL_FACE);
    shader.setInt("reverse_normals", 1);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    shader.setInt("reverse_normals", 0);
    glEnable(GL_CULL_FACE);

}
AreaConstants buildBuilding(Shader &shader, GLuint initialCube, float x, float z, float insideX, float insideZ) {
    glBindVertexArray(initialCube);// CUBE BASE
    //Builds Building
    translateMatrix = translate(mat4(1.0f), vec3(insideX + x, 20.0f, insideZ + z));
    scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 20.0f, 25.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, buildingTextures[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    //building antenna
    translateMatrix = translate(mat4(1.0f), vec3(insideX + x, 24.0f, insideZ + z));
    scaleMatrix = scale(mat4(1.0f), vec3(10.0f, 4.0f, 0.1f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, antennaTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    return AreaConstants{1, 0, 0, 0, 0};
}
//this builds the main buildings in the center
AreaConstants buildScrapers(Shader &shader, GLuint initialCube, float x, float z, float insideX, float insideZ) {
    glBindVertexArray(initialCube);// CUBE BASE
    translateMatrix = translate(mat4(1.0f), vec3(insideX + x, 40.0f, insideZ + z));
    scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 40.0f, 25.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scraperTextures[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    return AreaConstants{2, 0, 0, 0, 0};
}
void buildTree(Shader &shader, GLuint initialCube, float x, float z) {
    glBindVertexArray(initialCube);// CUBE BASE

    for(int i=0;i<6;i++) {
        // building bark
        translateMatrix = translate(mat4(1.0f), vec3(21.5 + x, 3.0f, 21.5-i*8.5 + z));
        scaleMatrix = scale(mat4(1.0f), vec3(3.0f, 3.0f, 3.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //building upper tree
        translateMatrix = translate(mat4(1.0f), vec3(21.5 + x, 7.0f, 21.5f-i*8.5 + z));
        scaleMatrix = scale(mat4(1.0f), vec3(7.5f, 3.0f, 7.5f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, treesTexture[0]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}
void buildStreetAndDecor(Shader &shader, GLuint initialCube, GLuint sphere, float x, float z) {
    // this builds the street
    for (int i = 0; i < 2; i++) {
        translateMatrix = translate(mat4(1.0f), vec3(0.0f + x, -0.01f, 0.0f + z));
        scaleMatrix = scale(mat4(1.0f), vec3(10.0f, 0.0f, 100.0f));
        rotateMatrix = rotate(mat4(1.0f), radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
        if(i==1){rotateMatrix = rotate(mat4(1.0f), radians(0.0f), vec3(0.0f, 1.0f, 0.0f));}
        modelMatrix = translateMatrix * rotateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, streetTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    // making a temp wall streets
    translateMatrix = translate(mat4(1.0f), vec3(25.0f + x, -0.01f, 0.0f + z));
    scaleMatrix = scale(mat4(1.0f), vec3(10.0f, 0.0f, 100.0f));
    rotateMatrix = rotate(mat4(1.0f), radians(0.0f), vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = translateMatrix * rotateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    translateMatrix = translate(mat4(1.0f), vec3(-25.0f + x, -0.01f, 0.0f + z));
    scaleMatrix = scale(mat4(1.0f), vec3(10.0f, 0.0f, 100.0f));
    rotateMatrix = rotate(mat4(1.0f), radians(0.0f), vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = translateMatrix * rotateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    translateMatrix = translate(mat4(1.0f), vec3(0.0f + x, -0.01f, 25.0f + z));
    scaleMatrix = scale(mat4(1.0f), vec3(10.0f, 0.0f, 100.0f));
    rotateMatrix = rotate(mat4(1.0f), radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = translateMatrix * rotateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    translateMatrix = translate(mat4(1.0f), vec3(0.0f + x, -0.01f, -25.0f + x));
    scaleMatrix = scale(mat4(1.0f), vec3(10.0f, 0.0f, 100.0f));
    rotateMatrix = rotate(mat4(1.0f), radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = translateMatrix * rotateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    //this builds the flooring

    translateMatrix = translate(mat4(1.0f), vec3(12.5f + x, -0.09f, 12.5f + z));
    scaleMatrix = scale(mat4(1.0f), vec3(50.0f, 0.0f, 50.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    translateMatrix = translate(mat4(1.0f), vec3(12.5f + x, -0.09f, -12.5f + z));
    scaleMatrix = scale(mat4(1.0f), vec3(50.0f, 0.0f, 50.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    translateMatrix = translate(mat4(1.0f), vec3(-12.5f + x, -0.09f, 12.5f + z));
    scaleMatrix = scale(mat4(1.0f), vec3(50.0f, 0.0f, 50.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    translateMatrix = translate(mat4(1.0f), vec3(-12.5f + x, -0.09f, -12.5f + z));
    scaleMatrix = scale(mat4(1.0f), vec3(50.0f, 0.0f, 50.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    //this builds the stops
    for (int i = 0; i < 4; i++) {
        glm::vec3 position;
        if (i == 0) {position = glm::vec3(4.0f, 2.5f, 4.0f);}
        else if (i == 1) {position = glm::vec3(4.0f, 2.5f, -4.0f);}
        else if (i == 2) {position = glm::vec3(-4.0f, 2.5f, 4.0f);}
        else if (i == 3) {position = glm::vec3(-4.0f, 2.5f, -4.0f);}
        translateMatrix = glm::translate(mat4(1.0), glm::vec3(position.x + x, position.y, position.z + z));
        scaleMatrix = glm::scale(mat4(1.0f), glm::vec3(1.5f, 2.5f, 1.5f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, stopTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    // this builds the stop signs

    /*

    translateMatrix = translate(mat4(1.0f), vec3(0.0f + x, 8.0f, 0.0f + z));
    scaleMatrix = scale(mat4(1.0f), vec3(0.5f, 8.0f, 5.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, firehydrantTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);

     */

    //this draws the balloon
    float time = glfwGetTime(); // Get the current time
    float yOffset = cos(time) * 45.0f;
    translateMatrix = translate(mat4(1.0f), vec3(90.0f + x, 90.0f + yOffset, 0.0f + z));
    scaleMatrix = scale(mat4(1.0f), vec3(15.0f, 5.0f, 15.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, balloonTextures[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // testing drawing a sphere
    //////////////// 1 //////////////
    translateMatrix = translate(mat4(1.0f), vec3(0.0f + x, 50.0f, 20.0f + z));
    scaleMatrix = scale(mat4(1.0f), vec3(20.0f, 5.0f, 20.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, balloonTextures[0]);
    glBindVertexArray(sphere);
    glDrawElements(GL_TRIANGLES, vertexCount ,GL_UNSIGNED_INT,(void*)0);

    //this draws the blimp
    translateMatrix = translate(mat4(1.0f), vec3( yOffset + x, 100.0f, 0.0f + z));
    scaleMatrix = scale(mat4(1.0f), vec3(70.0f, 5.0f, 15.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, balloonTextures[1]);
    glDrawElements(GL_TRIANGLES, vertexCount ,GL_UNSIGNED_INT,(void*)0);
    // this draws the welcome sign
    glBindVertexArray(initialCube);
    //sign on blimp
    translateMatrix = translate(mat4(1.0f), vec3( yOffset-31 + x, 100.0f, 0.0f + z));
    scaleMatrix = scale(mat4(1.0f), vec3(50.0f, 2.0f, 0.1f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, signsTexture[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    translateMatrix = translate(mat4(1.0f), vec3( 6.0f + x, 1.5, -2.5f + z));
    scaleMatrix = scale(mat4(1.0f), vec3(5.5f, 1.5f, 0.25f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, signsTexture[4]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

AreaConstants buildShops(Shader &shader, GLuint initialCube, float x, float z, float insideX, float insideZ) {
    translateMatrix = translate(mat4(1.0f), vec3(insideX + x, 7.0f, insideZ + z));
    scaleMatrix = scale(mat4(1.0f), vec3(12.5f, 7.0f, 32.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shopTextures[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    translateMatrix = translate(mat4(1.0f), vec3(insideX - 7.5f + x, 7.0f, insideZ + z));
    scaleMatrix = scale(mat4(1.0f), vec3(12.5f, 7.0f, 32.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shopTextures[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    return AreaConstants{3, 0, 0, 0, 0};

}
AreaConstants buildFountain(Shader &shader, GLuint initialCube, GLuint sphere, float x, float z, float insideX, float insideZ) {

    float time = glfwGetTime() * 5; // Get the current time
    float xOffset = sin(time) * 0.3f;
    float zOffset = cos(time) * 0.3f;
    //bottom of the fountain
    glBindVertexArray(initialCube);
    translateMatrix = translate(mat4(1.0f), vec3(insideX + x, 2.0f, insideZ + z));
    scaleMatrix = scale(mat4(1.0f), vec3(27.0f, 2.0f, 27.0));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fountainTextures[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    // animating water
    glBindVertexArray(sphere);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fountainTextures[2]);
    translateMatrix = translate(mat4(1.0f), vec3(insideX - xOffset + x, 3.99f, insideZ + zOffset + z));
    scaleMatrix = scale(mat4(1.0f), vec3(23.0f, 0.1f, 23.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, (void *) 0);
    translateMatrix = translate(mat4(1.0f), vec3(insideX + zOffset + x, 3.99f, insideZ - xOffset + z));
    scaleMatrix = scale(mat4(1.0f), vec3(23.0f, 0.1f, 23.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, (void *) 0);
    translateMatrix = translate(mat4(1.0f), vec3(insideX - xOffset + x, 3.99f, insideZ + zOffset + z));
    scaleMatrix = scale(mat4(1.0f), vec3(23.0f, 0.1f, 23.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, (void *) 0);
    translateMatrix = translate(mat4(1.0f), vec3(insideX + zOffset + x, 3.99f, insideZ - zOffset + z));
    scaleMatrix = scale(mat4(1.0f), vec3(23.0f, 0.1f, 23.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, (void *) 0);
    translateMatrix = translate(mat4(1.0f), vec3(insideX - xOffset + x, 3.99f, -12.0f + xOffset + z));
    scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 0.1f, 25.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, (void *) 0);
    glBindVertexArray(initialCube);
    //animating water as squares
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fountainTextures[2]);
    translateMatrix = translate(mat4(1.0f), vec3(insideX + x, 3.998f, insideZ + z));
    scaleMatrix = scale(mat4(1.0f), vec3(26.0f, 0.01f, 26.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    translateMatrix = translate(mat4(1.0f), vec3(12.5f + zOffset + x, 3.996f, insideZ - zOffset + z));
    scaleMatrix = scale(mat4(1.0f), vec3(23.0f, 0.01f, 23.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    translateMatrix = translate(mat4(1.0f), vec3(insideX - xOffset + x, 3.999f, insideZ + xOffset + z));
    scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 0.01f, 25.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    //fountain bricks
    for (int i = 0; i < 2; i++) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        translateMatrix = translate(mat4(1.0f), vec3(insideX + x, 3.999f, insideZ + 7.0f -i* 13.3 + z));
        scaleMatrix = scale(mat4(1.0f), vec3(27.0f, 0.25f, 2.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    for (int i = 0; i < 2; i++) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        translateMatrix = translate(mat4(1.0f), vec3(insideX + 7.0f -i* 13.3 + x, 3.999f,insideZ + 0.5f + z));
        scaleMatrix = scale(mat4(1.0f), vec3(2.0f, 0.25f, 27.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    return AreaConstants{4, 0, 0, 0, 0};
}


void buildPeople(Shader &shader, GLuint initialCube, float x, float z){
    glBindVertexArray(initialCube);
    translateMatrix = translate(mat4(1.0f), vec3(-3.0f + x, 2.0f, 6.0f + z));scaleMatrix = scaleMatrix = scale(mat4(1.0f), vec3(0.1f, 2.0f, 3.0f));
    modelMatrix = translateMatrix * scaleMatrix;shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D, peopleTextures[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    translateMatrix = translate(mat4(1.0f), vec3(-3.0f + x, 2.0f, -6.0f + z));scaleMatrix = scaleMatrix = scale(mat4(1.0f), vec3(0.1f, 2.0f, 3.0f));
    modelMatrix = translateMatrix * scaleMatrix;shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D, peopleTextures[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}
void buildLightCube(Shader &shader, GLuint sphere) {

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
    glm::mat4 view = camera.GetViewMatrix();

    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    glm::mat4 model;

    //////////////// 1 //////////////
    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(1.0f)); // a smaller cube
    shader.setMat4("model", model);

    glBindVertexArray(sphere);
    glDrawElements(GL_TRIANGLES, vertexCount ,GL_UNSIGNED_INT,(void*)0);

    //////////////// 2 //////////////
    model = glm::mat4(1.0f);
    model = glm::translate(model, pointLightPositions[0]);
    model = glm::scale(model, glm::vec3(1.0f)); // a smaller cube
    shader.setMat4("model", model);

    glBindVertexArray(sphere);
    glDrawElements(GL_TRIANGLES, vertexCount ,GL_UNSIGNED_INT,(void*)0);

    //////////////// 3 //////////////
    model = glm::mat4(1.0f);
    model = glm::translate(model, pointLightPositions[1]);
    model = glm::scale(model, glm::vec3(1.0f)); // a smaller cube
    shader.setMat4("model", model);

    glBindVertexArray(sphere);
    glDrawElements(GL_TRIANGLES, vertexCount ,GL_UNSIGNED_INT,(void*)0);
}

bool collisionDetection() {
    return true;
    //(camera.Position.y < bigWall.maxY && camera.Position.y > bigWall.minY)  &&  (camera.Position.x < bigWall.maxX && camera.Position.x > bigWall.minX)  &&  (camera.Position.z < bigWall.maxZ && camera.Position.z > bigWall.minZ);
}


void setShaderValues(Shader &shader) {
    shader.setVec3("viewPos", camera.Position);
    shader.setFloat("material.shininess", 32.0f);
    shader.setBool("isSpotLightOn", isSpotLightOn);

    // directional light
    shader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    if (!isNight) shader.setVec3("dirLight.ambient", 0.6f, 0.6f, 0.6f);
    else shader.setVec3("dirLight.ambient", 0.1f, 0.1f, 0.1f);
    shader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    shader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    // point light 1
    shader.setVec3("pointLights[0].position", pointLightPositions[0]);
    shader.setVec3("pointLights[0].ambient", 0.25f, 0.25f, 0.25f);
    shader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    shader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    shader.setFloat("pointLights[0].constant", 1.0f);
    shader.setFloat("pointLights[0].linear", 0.09f);
    shader.setFloat("pointLights[0].quadratic", 0.032f);
    // point light 2
    shader.setVec3("pointLights[1].position", pointLightPositions[1]);
    shader.setVec3("pointLights[1].ambient", 0.25f, 0.25f, 0.25f);
    shader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
    shader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
    shader.setFloat("pointLights[1].constant", 1.0f);
    shader.setFloat("pointLights[1].linear", 0.09f);
    shader.setFloat("pointLights[1].quadratic", 0.032f);
    // spotLight

    shader.setVec3("spotLight.position", camera.Position);
    shader.setVec3("spotLight.direction", camera.Front);
    shader.setVec3("spotLight.ambient", 0.2f, 0.2f, 0.2f);
    shader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    shader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    shader.setFloat("spotLight.constant", 1.0f);
    shader.setFloat("spotLight.linear", 0.09f);
    shader.setFloat("spotLight.quadratic", 0.032f);
    shader.setFloat("spotLight.cutOff", glm::cos(glm::radians(17.5f)));
    shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(25.0f)));

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
    glm::mat4 view = camera.GetViewMatrix();
    shader.setMat4("projectionMatrix", projection);
    shader.setMat4("viewMatrix", view);

    // set lighting uniforms
    shader.setVec3("lightPos", lightPos);
    shader.setVec3("viewPos", camera.Position);
    shader.setInt("noShadows", noShadows); // enable/disable noShadows by pressing 'SPACE'
    shader.setFloat("far_plane", far_plane);

}