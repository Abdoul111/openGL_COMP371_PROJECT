
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

vec3 initialCameraPos = vec3(0.0f, 5.5f, 0.0f);
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


mat4 translateMatrix;
mat4 rotateMatrix;
mat4 scaleMatrix;
mat4 modelMatrix;

/**
 * this struct will be used to save the positions of the already drawn squares
 * the bool operator< is used to compare the positions of the squares
 */
struct Position {
    float x;
    float z;

    bool operator<(const Position& other) const {
        if (x < other.x) return true;
        if (x > other.x) return false;
        return z < other.z;
    }

    bool operator==(const Position& other) const {
        return x == other.x && z == other.z;
    }
};

struct neighboringSides {
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
};

/**
 * this is for the location of the square in the world, it combines the position and the neighboring sides of the square.
 */
struct Location {
    Position position;
    neighboringSides sides;
};


map<Position, SquareConstants> drawnSquares;

// textures variables
unsigned int buildingTextures[4];unsigned int balloonTextures[2];unsigned int shopTextures[8];unsigned int scraperTextures[6];
unsigned int fountainTextures[3];unsigned int peopleTextures[8];
unsigned int backgroundTexture;unsigned int treesTexture[3];unsigned int woodTexture;unsigned int streetTexture;
unsigned int floorTexture;unsigned int firehydrantTexture;unsigned int stopTexture;
unsigned int signsTexture[7];unsigned int antennaTexture;unsigned int lightTexture[2];
unsigned int playgroundTextures[6];


// light sphere constants:
// light sphere
int resolution = 65;
int vertexCount = 6 * (resolution / 2) * resolution + 1;

// positions of the point lights
vector<vec3> pointLightPositions = {
        vec3(3, 10.15, 3),
        vec3(3 + 50, 10.15, 3),
        vec3(3, 10.15, 3 + 50),
        vec3(3 - 50, 10.15, 3),
        vec3(3 - 50, 10.15, 3 - 50),
};

bool isNight = false;
bool isSpotLightOn = false;

extern Location currentSquareLocation;
float currentSquareAreasHeight[4] = { 50.0f, 50.0f, 50.0f, 50.0f };

/**
 * this function draws 1 square depending of the x and z parameters passed to it.
 * it can call drawArea function to draw the 4 areas inside the square randomly or using already generated values from the drawnSquares map.
 */
void drawSquare(Shader shader, Shader lightShader, GLuint initialCube, GLuint sphere, float x, float z) {

    buildLightCube(lightShader, sphere, x, z);

    shader.use();
    buildStreetAndDecor(shader, initialCube, sphere, x, z);

    if (drawnSquares.find(Position{x, z}) != drawnSquares.end()) {
        // square is found
        int randomTexture = buildTree(shader, initialCube, x, z, drawnSquares[Position{x, z}].treeRandomTexture);

        AreaConstants area1 = drawArea(shader, initialCube, sphere, x, z, 12.5, 12.5, drawnSquares[Position{x, z}].areas[0].randomType, drawnSquares[Position{x, z}].areas[0].randomTexture, drawnSquares[Position{x, z}].areas[0].random3, drawnSquares[Position{x, z}].areas[0].random4, drawnSquares[Position{x, z}].areas[0].random5);
        AreaConstants area2 = drawArea(shader, initialCube, sphere, x, z, -12.5, 12.5, drawnSquares[Position{x, z}].areas[1].randomType, drawnSquares[Position{x, z}].areas[1].randomTexture, drawnSquares[Position{x, z}].areas[1].random3, drawnSquares[Position{x, z}].areas[1].random4, drawnSquares[Position{x, z}].areas[1].random5);
        AreaConstants area3 = drawArea(shader, initialCube, sphere, x, z, 12.5, -12.5, drawnSquares[Position{x, z}].areas[2].randomType, drawnSquares[Position{x, z}].areas[2].randomTexture, drawnSquares[Position{x, z}].areas[2].random3, drawnSquares[Position{x, z}].areas[2].random4, drawnSquares[Position{x, z}].areas[2].random5);
        AreaConstants area4 = drawArea(shader, initialCube, sphere, x, z, -12.5, -12.5, drawnSquares[Position{x, z}].areas[3].randomType, drawnSquares[Position{x, z}].areas[3].randomTexture, drawnSquares[Position{x, z}].areas[3].random3, drawnSquares[Position{x, z}].areas[3].random4, drawnSquares[Position{x, z}].areas[3].random5);
    } else {
        int randomTexture = buildTree(shader, initialCube, x, z, 0);
        AreaConstants area1 = drawArea(shader, initialCube, sphere, x, z, 12.5, 12.5, 0, 0, 0, 0, 0);
        AreaConstants area2 = drawArea(shader, initialCube, sphere, x, z, -12.5, 12.5, 0, 0, 0, 0, 0);
        AreaConstants area3 = drawArea(shader, initialCube, sphere, x, z, 12.5, -12.5, 0, 0, 0, 0, 0);
        AreaConstants area4 = drawArea(shader, initialCube, sphere, x, z, -12.5, -12.5, 0, 0, 0, 0, 0);

        drawnSquares.insert(pair<Position, SquareConstants>(Position{x, z}, SquareConstants{area1, area2, area3, area4, randomTexture}));
    }

}

// vector shops will store the bounderies of the buildings
// whenever we move, we check if we checked the boundary


/**
 * this function draws the 1 area inside the square depending on the insideX and insideZ parameters passed to it.
 * @param x the location of the square on the x axis
 * @param z the location of the square on the z axis
 * @param insideX the location of the area inside the square on the x axis
 * @param insideZ the location of the area inside the square on the z axis
 * @param type if type is 0, then it will generate a random number between 1 and 4, if it is not 0, then it will use the type passed to it as the type of the area (building, shop, etc...)
 * @return the random values generated for the area so that we can save them in the drawnSquares map
 */
AreaConstants drawArea(Shader shader, GLuint initialCube, GLuint sphere, float x, float z, float insideX, float insideZ, int type, int texture, int random3, int random4, int random5) {

    // generate random number between 1 and 4
    int random1 = !type ? ((rand() % 4) + 1) : type;
    AreaConstants area;
    if (random1 == 1) {
        // make it a building
        area = buildBuilding(shader, initialCube, x, z, insideX, insideZ, texture, random3, random4, random5);
    } else if (random1 == 2) {
        // make it a shop
        area = buildShops(shader, initialCube, x, z, insideX, insideZ, texture, random3, random4, random5);
    } else if (random1 == 3) {
        area = buildScrapers(shader, initialCube, x, z, insideX, insideZ, texture, random3, random4, random5);
    } else if (random1 == 4) {
        area = buildFountain(shader, initialCube, sphere, x, z, insideX, insideZ, texture, random3, random4, random5);
    }
    //this is for the blimp
    int randomBlimp = !random4 ? ((rand() % 8) + 1) : random4;
    buildBlimp(shader,initialCube,sphere,x,z,randomBlimp);

    return AreaConstants{area.randomType,area.randomTexture,area.random3,randomBlimp,area.random5};
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

/**
 * this function simply loads all the textures
 * @return the background texture as a default texture.
 */
unsigned int buildTextures() {
    buildingTextures[0] = loadTexture("rec/textures/building.png");buildingTextures[1] = loadTexture("rec/textures/buildingB.png");
    buildingTextures[2] = loadTexture("rec/textures/buildingC.png");buildingTextures[3] = loadTexture("rec/textures/buildingD.png");
    backgroundTexture = loadTexture("rec/textures/background.png");
    treesTexture[0] = loadTexture("rec/textures/tree.png");treesTexture[1] = loadTexture("rec/textures/tree2.png");
    treesTexture[2] = loadTexture("rec/textures/tree3.png");
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
    signsTexture[4] = loadTexture("rec/textures/welcome.png");signsTexture[5] = loadTexture("rec/textures/hicham.png");
    signsTexture[6] = loadTexture("rec/textures/abd.png");
    fountainTextures[0] = loadTexture("rec/textures/fountainbase.png");fountainTextures[1] = loadTexture("rec/textures/fountaintop.png");
    fountainTextures[2] = loadTexture("rec/textures/water.png");
    antennaTexture = loadTexture("rec/textures/antenna.png");
    peopleTextures[0] = loadTexture("rec/textures/cashier.png");peopleTextures[1] = loadTexture("rec/textures/mcworker.png");
    peopleTextures[2] = loadTexture("rec/textures/timemployee.png");peopleTextures[3] = loadTexture("rec/textures/SBemployee.png");
    peopleTextures[4] = loadTexture("rec/textures/fireman.png"); peopleTextures[5] = loadTexture("rec/textures/electrician.png");
    peopleTextures[6] = loadTexture("rec/textures/officer.png");peopleTextures[7] = loadTexture("rec/textures/banker.png");
    lightTexture[0]=loadTexture("rec/textures/lightpole.png");lightTexture[1]=loadTexture("rec/textures/lightpole2.png");
    playgroundTextures[0]= loadTexture("rec/textures/grass.png"); playgroundTextures[1]= loadTexture("rec/textures/white.png");
    playgroundTextures[2]= loadTexture("rec/textures/goal.png");playgroundTextures[3]= loadTexture("rec/textures/goalkeeper.png");
    playgroundTextures[4]= loadTexture("rec/textures/goalkeeper2.png");playgroundTextures[5]= loadTexture("rec/textures/ball.png");

    return backgroundTexture;
}

AreaConstants buildBuilding(Shader &shader, GLuint initialCube, float x, float z, float insideX, float insideZ, int texture, int random3, int random4, int random5) {

    int randomTexture = !texture ? ((rand() % 4) + 1) : texture;


    glBindVertexArray(initialCube);// CUBE BASE
    //Builds Building
    translateMatrix = translate(mat4(1.0f), vec3(insideX + x, 20.0f, insideZ + z));
    scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 20.0f, 25.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, buildingTextures[randomTexture-1]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    //building antenna
    translateMatrix = translate(mat4(1.0f), vec3(insideX + x, 43.0f, insideZ + z));
    scaleMatrix = scale(mat4(1.0f), vec3(10.0f, 4.0f, 0.1f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, antennaTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    return AreaConstants{1, randomTexture, 0, 0, 0};
}
//this builds the main buildings in the center
AreaConstants buildScrapers(Shader &shader, GLuint initialCube, float x, float z, float insideX, float insideZ, int texture, int random3, int random4, int random5) {

    int randomTexture = !texture ? ((rand() % 6) + 1) : texture;
    int randomTextureSign = !texture ? ((rand() % 5) + 1) : texture;
    int randomTexturePeople = !texture ? ((rand() % 8) + 1) : texture;

    glBindVertexArray(initialCube);// CUBE BASE
    translateMatrix = translate(mat4(1.0f), vec3(insideX + x, 40.0f, insideZ + z));
    scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 40.0f, 25.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scraperTextures[ (randomTexture-1) % 6]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    //this builds people
    translateMatrix = translate(mat4(1.0f), vec3(-3.0f + x, 2.0f, 6.0f + z));scaleMatrix = scaleMatrix = scale(mat4(1.0f), vec3(0.1f, 2.0f, 3.0f));
    modelMatrix = translateMatrix * scaleMatrix;shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D, peopleTextures[randomTexturePeople-1]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    translateMatrix = translate(mat4(1.0f), vec3(-3.0f + x, 2.0f, -6.0f + z));scaleMatrix = scaleMatrix = scale(mat4(1.0f), vec3(0.1f, 2.0f, 3.0f));
    modelMatrix = translateMatrix * scaleMatrix;shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D, peopleTextures[randomTexturePeople%8]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    //this builds signs
    translateMatrix = translate(mat4(1.0f), vec3( 6.0f + x, 1.5, -2.5f + z));
    scaleMatrix = scale(mat4(1.0f), vec3(5.5f, 1.5f, 0.25f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, signsTexture[(randomTextureSign-1) % 4]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    // This builds abd x hixham street sign
    translateMatrix = translate(mat4(1.0f), vec3( 8+ x, 8, 4+ z));
    scaleMatrix = scale(mat4(1.0f), vec3(0.25f, 0.25f, 4.5f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, signsTexture[5]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    translateMatrix = translate(mat4(1.0f), vec3( 8+ x, 7.55, 4+ z));
    scaleMatrix = scale(mat4(1.0f), vec3(4.5f, 0.25f, 0.25f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, signsTexture[6]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    translateMatrix = translate(mat4(1.0f), vec3( 8+ x, 3.0, 4+ z));
    scaleMatrix = scale(mat4(1.0f), vec3(0.25f, 4.3f, 0.25f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, lightTexture[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    return AreaConstants{2, randomTexture, 0, 0, 0};
}
int buildTree(Shader &shader, GLuint initialCube, float x, float z, int randomTexture) {
    glBindVertexArray(initialCube);// CUBE BASE
    int randomTreeTexture = !randomTexture? ((rand() % 3)+1):randomTexture;
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

        glBindTexture(GL_TEXTURE_2D, treesTexture[randomTreeTexture-1]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    return randomTreeTexture;
}

void buildStreetAndDecor(Shader &shader, GLuint initialCube, GLuint sphere, float x, float z) {
    // this builds the street
    glBindVertexArray(initialCube);// CUBE BASE

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

    translateMatrix = translate(mat4(1.0f), vec3(0.0f + x, -0.01f, -25.0f + z));
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




    //THIS DRAWS THE LIGHTS base
    for(int i =0;i<2;i++) {
        if(i==0){translateMatrix = translate(mat4(1.0f), vec3(3 + x, 5, 3 + z));}
        if(i==1){translateMatrix = translate(mat4(1.0f), vec3(3 + x, 5, -3 + z));}
        scaleMatrix = scale(mat4(1.0f), vec3(1.25f, 5.0f, 1.25f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, lightTexture[0]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    //THIS DRAWS THE LIGHT top
    for(int i =0;i<2;i++) {
        if(i==0){translateMatrix = translate(mat4(1.0f), vec3(3+ x, 10.15, 3 + z));}
        if(i==1){translateMatrix = translate(mat4(1.0f), vec3(3 + x, 10.15, -3 + z));}
        scaleMatrix = scale(mat4(1.0f), vec3(1.25f, 0.15f, 7.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, lightTexture[1]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}
void buildBlimp(Shader &shader, GLuint initialCube, GLuint sphere, float x, float z,int randomValue){

    //this draws the balloon
    if(randomValue==1) {
        float time = glfwGetTime(); // Get the current time
        float yOffset = cos(time) * 45.0f;
        translateMatrix = translate(mat4(1.0f), vec3(90.0f + x, 90.0f + yOffset, 0.0f + z));
        scaleMatrix = scale(mat4(1.0f), vec3(15.0f, 5.0f, 15.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, balloonTextures[0]);
        glBindVertexArray(sphere);
        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, (void *) 0);

        //this draws the blimp
        translateMatrix = translate(mat4(1.0f), vec3(yOffset + x, 100.0f, 0.0f + z));
        scaleMatrix = scale(mat4(1.0f), vec3(70.0f, 5.0f, 15.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, balloonTextures[1]);
        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, (void *) 0);
        // this draws the welcome sign
        glBindVertexArray(initialCube);
        //sign on blimp
        translateMatrix = translate(mat4(1.0f), vec3(yOffset - 27 + x, 100.0f, 0.0f + z));
        scaleMatrix = scale(mat4(1.0f), vec3(20.0f, 2.0f, 0.1f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, signsTexture[0]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

AreaConstants buildShops(Shader &shader, GLuint initialCube, float x, float z, float insideX, float insideZ, int texture, int random3, int random4, int random5) {

    int randomTexture = !texture ? ((rand() % 8) + 1) : texture;

    translateMatrix = translate(mat4(1.0f), vec3(insideX + x + 3.75, 7.0f, insideZ + z));
    scaleMatrix = scale(mat4(1.0f), vec3(12.5f, 7.0f, 25.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shopTextures[randomTexture - 1]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    translateMatrix = translate(mat4(1.0f), vec3(insideX - 3.75 + x, 7.0f, insideZ + z));
    scaleMatrix = scale(mat4(1.0f), vec3(12.5f, 7.0f, 25.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shopTextures[randomTexture % 8]);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    return AreaConstants{3, randomTexture, 0, 0, 0};

}
AreaConstants buildFountain(Shader &shader, GLuint initialCube, GLuint sphere, float x, float z, float insideX, float insideZ, int texture, int random3, int random4, int random5) {

    // the next variable decides if we draw a fountain, a playground or a garden
    int randomSpecificType = !random3 ? ((rand() % 2) + 1) : random3;
    // this builds a fountain
    if (randomSpecificType == 1) {
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
        translateMatrix = translate(mat4(1.0f), vec3(insideX - xOffset + x, 3.99f, insideZ + xOffset + z));
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
        translateMatrix = translate(mat4(1.0f), vec3(insideX + zOffset + x, 3.996f, insideZ - zOffset + z));
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

    } else {
        // this builds a playground
        glBindVertexArray(initialCube);
        translateMatrix = translate(mat4(1.0f), vec3(insideX + x, 0.01f, insideZ + z));
        scaleMatrix = scale(mat4(1.0f), vec3(27.0f, 0.01f, 27.0));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, playgroundTextures[0]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //white floor
        for (int i = 0; i < 2; i++) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, playgroundTextures[1]);
            translateMatrix = translate(mat4(1.0f), vec3(insideX + x, 0.02f, insideZ + 7.0f -i* 13.3 + z));
            scaleMatrix = scale(mat4(1.0f), vec3(27.0f, 0.01f, 2.0f));
            modelMatrix = translateMatrix * scaleMatrix;
            shader.setMat4("modelMatrix", modelMatrix);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            translateMatrix = translate(mat4(1.0f), vec3(insideX + 7.0f -i* 13.3 + x, 0.02f,insideZ + 0.5f + z));
            scaleMatrix = scale(mat4(1.0f), vec3(2.0f, 0.01f, 27.0f));
            modelMatrix = translateMatrix * scaleMatrix;
            shader.setMat4("modelMatrix", modelMatrix);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        // the goals
        for (int i = 0; i < 2; i++) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, playgroundTextures[2]);
            translateMatrix = translate(mat4(1.0f), vec3(insideX + x + 7, 2.5f, insideZ + z));
            if(i==0){translateMatrix = translate(mat4(1.0f), vec3(insideX + x - 7, 2.5f, insideZ + z));}
            scaleMatrix = scale(mat4(1.0f), vec3(0.01f, 2.5f, 27.0));
            modelMatrix = translateMatrix * scaleMatrix;
            shader.setMat4("modelMatrix", modelMatrix);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        // the goalkeepers
        float currentTime = glfwGetTime();
        float radius = 5.6f;
        float speed = 1.0f;
        float xPos = radius * cos(currentTime * speed);
        float zPos = radius * sin(currentTime * speed);
        for (int i = 0; i < 2; i++) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, playgroundTextures[4]);
            if(i==0){glBindTexture(GL_TEXTURE_2D, playgroundTextures[3]);}
            translateMatrix = translate(mat4(1.0f), vec3(insideX + x + 6, 2.5f, insideZ + z+zPos));
            if(i==0){translateMatrix = translate(mat4(1.0f), vec3(insideX + x - 6, 2.5f, insideZ + z-zPos));}
            scaleMatrix = scale(mat4(1.0f), vec3(0.01f, 2.5f, 7.0));
            modelMatrix = translateMatrix * scaleMatrix;
            shader.setMat4("modelMatrix", modelMatrix);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        //the ball
        glBindVertexArray(sphere);
        translateMatrix = translate(mat4(1.0f), vec3(insideX + x+xPos, 1.0f, insideZ + z+zPos));
        scaleMatrix = scale(mat4(1.0f), vec3(2.0f, 0.75f, 2.0));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, playgroundTextures[5]);
        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, (void *) 0);

    }
    return AreaConstants{4, 0, randomSpecificType, 0, 0};
}




void buildLightCube(Shader &shader, GLuint sphere,float x,float z) {

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
    glm::mat4 view = camera.GetViewMatrix();

    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    glm::mat4 model = glm::mat4(1.0f);
    //Inner lights
    vec3 position = vec3(3+ x, 10.0, 1.50 + z);
    // adding the position to the vector
    if (std::find(pointLightPositions.begin(), pointLightPositions.end(), position) == pointLightPositions.end()) {
        // not found
        pointLightPositions.push_back(position);
    }

    for(int i=0;i<4;i++){
        model = glm::mat4(1.0f);
        //Inner lights
        if(i==0){model = glm::translate(model, vec3(3+ x, 10.0, 1.50 + z));}
        if(i==1){model = glm::translate(model, vec3(3+ x, 10.0, -1.50 + z));}
        //Outer lights
        if(i==2){model = glm::translate(model, vec3(3+ x, 10.0, 4.50 + z));}
        if(i==3){model = glm::translate(model, vec3(3+ x, 10.0, -4.50 + z));}

        model = glm::scale(model, glm::vec3(0.25f, 0.15f, 0.25f)); // a smaller cube
        shader.setMat4("model", model);

        glBindVertexArray(sphere);
        glDrawElements(GL_TRIANGLES, vertexCount ,GL_UNSIGNED_INT,(void*)0);
    }

}

void setShaderValues(Shader &shader) {
    shader.setVec3("viewPos", camera.Position);
    shader.setFloat("material.shininess", 32.0f);
    shader.setBool("isSpotLightOn", isSpotLightOn);

    // directional light
    shader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    if (!isNight) shader.setVec3("dirLight.ambient", 0.6f, 0.6f, 0.6f);
    else shader.setVec3("dirLight.ambient", 0.03f, 0.03f, 0.03f);
    shader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    shader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

    // point lights
    shader.setInt("currentPointLightNb", pointLightPositions.size());
    shader.setBool("isPointLightOn", isNight);
    for (int i = 0; i < pointLightPositions.size(); i++) {
        shader.setVec3("pointLights[" + std::to_string(i) + "].position", pointLightPositions[i]);
        shader.setVec3("pointLights[" + std::to_string(i) + "].ambient", 0.25f, 0.25f, 0.25f);
        shader.setVec3("pointLights[" + std::to_string(i) + "].diffuse", 0.8f, 0.8f, 0.8f);
        shader.setVec3("pointLights[" + std::to_string(i) + "].specular", 1.0f, 1.0f, 1.0f);
        shader.setFloat("pointLights[" + std::to_string(i) + "].constant", 1.0f);
        shader.setFloat("pointLights[" + std::to_string(i) + "].linear", 0.09f);
        shader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", 0.032f);
    }

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

int areaType(int areaNumber) {
    return drawnSquares[Position{currentSquareLocation.position.x, currentSquareLocation.position.z}].areas[areaNumber].randomType;
}

float areaHeight(int areaType) {
    if (areaType == 1) {
        return 45.0f;
    } else if (areaType == 2) {
        return 16.0f;
    } else if (areaType == 3) {
        return 80.0f;
    } else {
        return 10.0f;
    }
}

void updateCurrentSquareHights() {
    currentSquareAreasHeight[0] = areaHeight(areaType(0));
    currentSquareAreasHeight[1] = areaHeight(areaType(1));
    currentSquareAreasHeight[2] = areaHeight(areaType(2));
    currentSquareAreasHeight[3] = areaHeight(areaType(3));
}