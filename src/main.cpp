/*
 *
 * made by Abd Alwahed Haj Omar
 *      and Hicham Kitaz
 * id: 40246177
 * id: 40188246
 * sources consulted for this quiz:
 * 1. chatGPT
 * 2. LearnOpenGL website and GitHub repository
 */


#include <iostream>
#include <vector>

#define GLEW_STATIC 1
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "lib/Camera.h"
#include "lib/shader.h"
#include "vao.h"
#include "Backend.h"

using namespace glm;
using namespace std;


struct Location {
    float x = 0.0f;
    float z = 0.0f;
    bool rightSquare = false;
    bool leftSquare = false;
    bool upSquare = false;
    bool downSquare = false;
};

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void drawScene(Shader shader, GLuint initialCube, GLuint blueBigCube, GLuint sphere);
Location newSquareLocation(vec3 cameraPosition);
void addNewLocations();

extern const unsigned int SCR_WIDTH = 1024;
extern const unsigned int SCR_HEIGHT = 768;
extern bool noShadows;
extern bool noShadowsKeyPressed;

extern vec3 initialCameraPos;
extern Camera camera;
extern vec3 lightPos;
extern float lastX;
extern float lastY;
extern bool firstMouse;

extern float near_plane;
// change the max distance of light
extern float far_plane;
// light sphere constants:
// light sphere
extern int resolution;
extern int vertexCount;

extern float deltaTime;
extern float lastFrame;

// positions of the point lights
extern vec3 pointLightPositions[];

extern bool isNight;
extern bool isSpotLightOn;


Location squareLocation = {0.0f, 0.0f, true, true, true, true};
Location squareLocation2 = {50.0f, 0.0f, true, false, false, false};
Location squareLocation3 = {-50.0f, 0.0f, false, true, false, false};
Location squareLocation4 = {0.0f, 50.0f, false, false, true, false};
Location squareLocation5 = {0.0f, -50.0f, false, false, false, true};

Location currentSquareLocation = squareLocation;

vector<Location> squareLocations;

int main() {
    // Initialize GLFW and OpenGL version
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create Window and rendering context using GLFW, resolution is 1024 768
    GLFWwindow* window = glfwCreateWindow(1024, 768, "Comp371 - Quiz2", nullptr, nullptr);
    if (window == nullptr)
    {
        cerr << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        cerr << "Failed to create GLEW" << endl;
        glfwTerminate();
        return -1;
    }


    // adding the default squares
    squareLocations.push_back(squareLocation);
    squareLocations.push_back(squareLocation2);
    squareLocations.push_back(squareLocation3);
    squareLocations.push_back(squareLocation4);
    squareLocations.push_back(squareLocation5);

    // whenever we add a square, we check if it has neighboring squares, (obviously, it should have at least 1)
    // this will be done later when we add squares dynamically

    // 20
    // 15 + constant -> 0 - 10, (15, 25)


    // square                   15*15
    // (0,0) 7.5
    // LEFT -> 1 unit to the left, then updates the position coordinates -> (1,0),
    // then checks if this new position is still inside the square
    // LEFT, LEFT, LEFT, LEFT, LEFT, LEFT, LEFT, LEFT(8, 0) = 8 LEFT
    // then we are in the left square
    // I have the center of current square, then I can easily get the center of the new square
    // (0,0) -> (15, 0)
    // and we have the current position (8, 0)
    // then currentSquare = newSquare
    // generateNearbySquares() ->
    // whenever we create a new square, we store its neighbors squares in a list
    // then I know what are the missing squares around me
    // then I generate what is missing


    // random1 : specifies the type (building, shop, facilities, etc)


    // square be a square
    //if (building) -> random2 : number buildings (1, 2)
    //              -> random3 : specifies the height of the building (base= 10 unites + (5 to 10))
    //              -> random4 : specifies the number of trees around the building (0 to 5)
    //              -> random5 : specifies the texture of the building (6 choices)

    // else if (shop) -> random2 : specifies the texture of the shop (8 choices)
    //               -> random3 : specifies the height of the shop (base= 5 unites + (1 to 5 unites))


    // to ensure that the player cannot enter inside objects, we can save the boundaries of
    // the 4 elements inside the square in a list and then save those in a list of boundaries.
    // then for each move from the play, for each boundary, we check for each side if the player
    // have moved into an object. If yes, then we don't let the camera move in this direction and move it back
    // from where it moved

    // if building 1 is 50x50, then building 2 is 50x50, and in between we have 4 unites, then let square be 110x110.

    // Set the keyboard key and mouse callback functions
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glClearColor(30.0f/255, 48.0f/255, 143.0f/255, 1.0f);

    GLuint initialCube = createCubeCoordinate();
    GLuint blueBigCube = createCubeCoordinate();
    GLuint sphere = createSphere(resolution, 1);

    // Enable Backface culling and depth test
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    Shader shader("shaders/textureShader.vs", "shaders/textureShader.fs");
    Shader depthShader("shaders/depthShader.vs", "shaders/depthShader.fs", "shaders/depthShader.gs");
    Shader lightCubeShader("shaders/lightShader.vs", "shaders/lightShader.fs");

    unsigned int defaultTexture = buildTextures();

    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth cubemap texture
    unsigned int depthCubemap;
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);

    // resetting the framebuffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("material.diffuse", 0);
    shader.setInt("material.specular", 1);
    shader.setInt("depthMap", 2);

    // lighting info
    // -------------



    while (!glfwWindowShouldClose(window)) {

        auto currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        lightPos.z = static_cast<float>(sin(glfwGetTime() * 0.5) * 30.0);

        // render
        // ------
        glClearColor(30.0f/255, 48.0f/255, 143.0f/255, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 0. create depth cubemap transformation matrices
        // -----------------------------------------------
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);

        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));

        // 1. render scene to depth cubemap
        // --------------------------------
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        depthShader.use();
        for (unsigned int i = 0; i < 6; ++i)
            depthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        depthShader.setFloat("far_plane", far_plane);
        depthShader.setVec3("lightPos", lightPos);
        drawScene(shader, initialCube, blueBigCube, sphere);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. render scene as normal
        // -------------------------
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        setShaderValues(shader);
        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, defaultTexture);
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, defaultTexture);
        // DONT TOUCH
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

        drawScene(shader, initialCube, blueBigCube, sphere);
        buildLightCube(lightCubeShader, sphere);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        // End Frame
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Shutdown GLFW
    glfwTerminate();

    return 0;
}


void drawScene(Shader shader, GLuint initialCube, GLuint blueBigCube, GLuint sphere) {

    for (int i = 0; i < squareLocations.size(); i++) {
        drawSquare(shader, initialCube, blueBigCube, sphere, squareLocations[i].x, squareLocations[i].z);
    }
}

bool cameraInsideCurrentSquare() {
    return camera.Position.z >= currentSquareLocation.z - 25.0f && camera.Position.z <= currentSquareLocation.z + 25.0f
           && camera.Position.x >= currentSquareLocation.x - 25.0f && camera.Position.x <= currentSquareLocation.x + 25.0f;
}


void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.ProcessKeyboard(FORWARD, deltaTime);
        if (!cameraInsideCurrentSquare()) {
             Location newCurrentLocation = newSquareLocation(camera.Position);
             currentSquareLocation = newCurrentLocation;
             cout << "camera position: " << camera.Position.x << ", " << camera.Position.z << endl;
             cout << "new square location: " << currentSquareLocation.x << ", " << currentSquareLocation.z << endl;

             // now we need to add the new locations to the vector so that they get drawn when we pass
             // to the while loop again.
             addNewLocations();
        }
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (!cameraInsideCurrentSquare()) {
            Location newCurrentLocation = newSquareLocation(camera.Position);
            currentSquareLocation = newCurrentLocation;
            cout << "camera position: " << camera.Position.x << ", " << camera.Position.z << endl;
            cout << "new square location: " << currentSquareLocation.x << ", " << currentSquareLocation.z << endl;

            // now we need to add the new locations to the vector so that they get drawn when we pass
            // to the while loop again.
            addNewLocations();
        }
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.ProcessKeyboard(LEFT, deltaTime);
        if (!cameraInsideCurrentSquare()) {
            Location newCurrentLocation = newSquareLocation(camera.Position);
            currentSquareLocation = newCurrentLocation;
            cout << "camera position: " << camera.Position.x << ", " << camera.Position.z << endl;
            cout << "new square location: " << currentSquareLocation.x << ", " << currentSquareLocation.z << endl;

            // now we need to add the new locations to the vector so that they get drawn when we pass
            // to the while loop again.
            addNewLocations();
        }
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(RIGHT, deltaTime);
        if (!cameraInsideCurrentSquare()) {
            Location newCurrentLocation = newSquareLocation(camera.Position);
            currentSquareLocation = newCurrentLocation;
            cout << "camera position: " << camera.Position.x << ", " << camera.Position.z << endl;
            cout << "new square location: " << currentSquareLocation.x << ", " << currentSquareLocation.z << endl;

            // now we need to add the new locations to the vector so that they get drawn when we pass
            // to the while loop again.
            addNewLocations();
        }
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !noShadowsKeyPressed) {
        isNight = !isNight;
    }
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE) {
        isSpotLightOn = !isSpotLightOn;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !noShadowsKeyPressed) {
        noShadows = !noShadows;
        noShadowsKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        noShadowsKeyPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

Location newSquareLocation(vec3 cameraPosition) {
    Location newLocation;
    if (cameraPosition.x > currentSquareLocation.x + 25) {
        newLocation = {currentSquareLocation.x + 50.0f, currentSquareLocation.z , false, false, false, true};
    } else if (cameraPosition.x < currentSquareLocation.x - 25) {
        newLocation = {currentSquareLocation.x - 50.0f, currentSquareLocation.z , false, false, true, false};
    } else if (cameraPosition.z > currentSquareLocation.z + 25) {
        newLocation = {currentSquareLocation.x , currentSquareLocation.z + 50.0f, false, true, false, false};
    } else if (cameraPosition.z < currentSquareLocation.z - 25) {
        newLocation = {currentSquareLocation.x , currentSquareLocation.z - 50.0f, true, false, false, false};
    }
    return newLocation;
}

void addNewLocations() {
    if (!currentSquareLocation.rightSquare) {
        Location newLocation = {currentSquareLocation.x + 50.0f, currentSquareLocation.z , false, false, false, true};
        squareLocations.push_back(newLocation);
    }
    if (!currentSquareLocation.leftSquare) {
        Location newLocation = {currentSquareLocation.x - 50.0f, currentSquareLocation.z , false, false, true, false};
        squareLocations.push_back(newLocation);
    }
    if (!currentSquareLocation.upSquare) {
        Location newLocation = {currentSquareLocation.x , currentSquareLocation.z + 50.0f, false, true, false, false};
        squareLocations.push_back(newLocation);
    }
    if (!currentSquareLocation.downSquare) {
        Location newLocation = {currentSquareLocation.x , currentSquareLocation.z - 50.0f, true, false, false, false};
        squareLocations.push_back(newLocation);
    }
}