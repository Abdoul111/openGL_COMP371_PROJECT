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


#include <algorithm>
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

/**
 * this struct will be used to store the position of the squares in the world
 * the bool operator is necessairy for the program to be able to compare 2 positions
 */
struct Position {
    float x = 0.0f;
    float z = 0.0f;

    bool operator==(const Position& other) const {
        return x == other.x && z == other.z;
    }
};

/**
 * this struct will be used to store the boolean value that a square has another square on its side.
 */
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

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void drawScene(Shader shader, Shader lightShader, GLuint initialCube, GLuint sphere);
Position newSquarePosition(vec3 cameraPosition);
void addNewLocations();
neighboringSides findNeighboringSides(Position newPosition);

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
extern vector<vec3> pointLightPositions;
extern float currentSquareAreasHeight[];

extern bool isNight;
extern bool isSpotLightOn;

bool isYAxisActive = false;


// here we define the preloaded squares in the world. (at the beginning we have 5 squares)
Location squareLocation = {0.0f, 0.0f, true, true, true, true};
Location squareLocation2 = {50.0f, 0.0f, true, false, false, false};
Location squareLocation3 = {-50.0f, 0.0f, false, true, false, false};
Location squareLocation4 = {0.0f, 50.0f, false, false, true, false};
Location squareLocation5 = {0.0f, -50.0f, false, false, false, true};

/**
 * the following variable will be used to store the current square that the player is in.
 */
Location currentSquareLocation = squareLocation;

/**
 * the following vector stores the square's information that we will loop through later to draw the squares.
 */
vector<Location> squareLocations;

/**
 * the following vector will be used to store the positions of the squares that are stored already in squareLocations
 * this will help us manipulate the data easily later
 */
vector<Position> squarePositions;

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


    // adding the default squares to the vector
    squareLocations.push_back(squareLocation);
    squareLocations.push_back(squareLocation2);
    squareLocations.push_back(squareLocation3);
    squareLocations.push_back(squareLocation4);
    squareLocations.push_back(squareLocation5);


    // adding the default squares positions to the position vector.
    squarePositions.push_back(squareLocation.position);
    squarePositions.push_back(squareLocation2.position);
    squarePositions.push_back(squareLocation3.position);
    squarePositions.push_back(squareLocation4.position);
    squarePositions.push_back(squareLocation5.position);

    // Set the keyboard key and mouse callback functions
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glClearColor(30.0f/255, 48.0f/255, 143.0f/255, 1.0f);

    GLuint initialCube = createCubeCoordinate();
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

        //lightPos.z = static_cast<float>(sin(glfwGetTime() * 0.5) * 30.0);

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

        depthShader.setInt("nrLights", pointLightPositions.size());
        depthShader.setBool("isNight", isNight);
        depthShader.setBool("noShadows", noShadows);
        for (int m = 0; m < pointLightPositions.size(); m++) {
            for (unsigned int i = 0; i < 6; ++i)
                depthShader.setMat4("shadowMatrices[" + std::to_string(i * m + i) + "]", shadowTransforms[i]);
        }
        depthShader.setFloat("far_plane", far_plane);
        depthShader.setVec3("lightPos", lightPos);
        // the drawScene function draws the all the squares in the world (here we do the shadows)
        drawScene(depthShader, lightCubeShader, initialCube, sphere);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. render scene as normal
        // -------------------------
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        // the following function sets the necessary uniforms for the shader
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

        // the drawScene function draws the all the squares in the world
        drawScene(shader, lightCubeShader, initialCube, sphere);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        // End Frame
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Shutdown GLFW
    glfwTerminate();

    return 0;
}

/**
 * looping through the vector to draw the squares
 */
void drawScene(Shader shader, Shader lightShader, GLuint initialCube, GLuint sphere) {
    for (int i = 0; i < squareLocations.size(); i++) {
        drawSquare(shader, lightShader, initialCube, sphere, squareLocations[i].position.x, squareLocations[i].position.z);
    }
}

/**
 * this function checks if the camera is inside the current square
 * @return true if the camera is inside the current square, false otherwise
 */
bool cameraInsideCurrentSquare() {
    return camera.Position.z >= currentSquareLocation.position.z - 25.0f && camera.Position.z <= currentSquareLocation.position.z + 25.0f
           && camera.Position.x >= currentSquareLocation.position.x - 25.0f && camera.Position.x <= currentSquareLocation.position.x + 25.0f;
}

bool cameraInsideArea(int areaNumber) {
    struct Limits {
        float xMin;
        float xMax;
        float zMin;
        float zMax;
    };
    Limits limits[4] {
            {currentSquareLocation.position.x + 5, currentSquareLocation.position.x + 20, currentSquareLocation.position.z + 5, currentSquareLocation.position.z + 20},
            {currentSquareLocation.position.x - 20, currentSquareLocation.position.x - 5, currentSquareLocation.position.z + 5, currentSquareLocation.position.z + 20},
            {currentSquareLocation.position.x + 5, currentSquareLocation.position.x + 20, currentSquareLocation.position.z - 20, currentSquareLocation.position.z - 5},
            {currentSquareLocation.position.x - 20, currentSquareLocation.position.x - 5, currentSquareLocation.position.z - 20, currentSquareLocation.position.z - 5}
    };
    return camera.Position.x > limits[areaNumber].xMin && camera.Position.x < limits[areaNumber].xMax
        && camera.Position.z > limits[areaNumber].zMin && camera.Position.z < limits[areaNumber].zMax
        && camera.Position.y < currentSquareAreasHeight[areaNumber] + 0.5f;

}

bool cameraInsideAnyArea() {
    return cameraInsideArea(0) || cameraInsideArea(1) || cameraInsideArea(2) || cameraInsideArea(3);
}


void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
        isYAxisActive = !isYAxisActive;
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        camera.Position.y = 150.0f;
        camera.Front = vec3(0.0f, -1.0f, 0.0f);
        camera.Yaw = 0.0f;
        camera.Pitch = -90.0f;
        camera.updateCameraVectors();
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        camera.Position.y = 6.0f;
        camera.Yaw = -90.0f;
        camera.Pitch = 0.0f;
        camera.updateCameraVectors();

    }



    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.ProcessKeyboard(FORWARD, deltaTime, isYAxisActive); // move forward 1 unite

        if (cameraInsideAnyArea()) {
            camera.ProcessKeyboard(BACKWARD, deltaTime, isYAxisActive);
        }

        if (!cameraInsideCurrentSquare()) {
             Position newCurrentPosition = newSquarePosition(camera.Position);
             neighboringSides sides = findNeighboringSides(newCurrentPosition);
             currentSquareLocation = Location{newCurrentPosition, sides};
             cout << "camera position: " << camera.Position.x << ", " << camera.Position.z << endl;
             cout << "new square location: " << currentSquareLocation.position.x << ", " << currentSquareLocation.position.z << endl;
            updateCurrentSquareHights();
             // now we need to add the new locations to the vector so that they get drawn when we pass
             // to the while loop again.
             addNewLocations();
        }
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.ProcessKeyboard(BACKWARD, deltaTime, isYAxisActive);

        if (cameraInsideAnyArea()) {
            camera.ProcessKeyboard(FORWARD, deltaTime, isYAxisActive);
        }

        if (!cameraInsideCurrentSquare()) {
            Position newCurrentPosition = newSquarePosition(camera.Position);
            neighboringSides sides = findNeighboringSides(newCurrentPosition);
            currentSquareLocation = Location{newCurrentPosition, sides};
            cout << "camera position: " << camera.Position.x << ", " << camera.Position.z << endl;
            cout << "new square location: " << currentSquareLocation.position.x << ", " << currentSquareLocation.position.z << endl;
            updateCurrentSquareHights();
            // now we need to add the new locations to the vector so that they get drawn when we pass
            // to the while loop again.
            addNewLocations();
        }
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.ProcessKeyboard(LEFT, deltaTime, isYAxisActive);

        if (cameraInsideAnyArea()) {
            camera.ProcessKeyboard(RIGHT, deltaTime, isYAxisActive);
        }

        if (!cameraInsideCurrentSquare()) {
            Position newCurrentPosition = newSquarePosition(camera.Position);
            neighboringSides sides = findNeighboringSides(newCurrentPosition);
            currentSquareLocation = Location{newCurrentPosition, sides};
            cout << "camera position: " << camera.Position.x << ", " << camera.Position.z << endl;
            cout << "new square location: " << currentSquareLocation.position.x << ", " << currentSquareLocation.position.z << endl;
            updateCurrentSquareHights();
            // now we need to add the new locations to the vector so that they get drawn when we pass
            // to the while loop again.
            addNewLocations();
        }
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(RIGHT, deltaTime, isYAxisActive);

        if (cameraInsideAnyArea()) {
            camera.ProcessKeyboard(LEFT, deltaTime, isYAxisActive);
        }

        if (!cameraInsideCurrentSquare()) {
            Position newCurrentPosition = newSquarePosition(camera.Position);
            neighboringSides sides = findNeighboringSides(newCurrentPosition);
            currentSquareLocation = Location{newCurrentPosition, sides};
            cout << "camera position: " << camera.Position.x << ", " << camera.Position.z << endl;
            cout << "new square location: " << currentSquareLocation.position.x << ", " << currentSquareLocation.position.z << endl;
            updateCurrentSquareHights();
            // now we need to add the new locations to the vector so that they get drawn when we pass
            // to the while loop again.
            addNewLocations();
        }
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !noShadowsKeyPressed) {
        isNight = !isNight;
    }
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
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

/**
 * this function is called when the current square is changed.
 * @param cameraPosition it takes the camera position as a parameter
 * @return the new square position ( the square that the camera is in now)
 */
Position newSquarePosition(vec3 cameraPosition) {
    Position newPosition;
    if (cameraPosition.x > currentSquareLocation.position.x + 25) {
        newPosition = {currentSquareLocation.position.x + 50.0f, currentSquareLocation.position.z};
    } else if (cameraPosition.x < currentSquareLocation.position.x - 25) {
        newPosition = {currentSquareLocation.position.x - 50.0f, currentSquareLocation.position.z};
    } else if (cameraPosition.z > currentSquareLocation.position.z + 25) {
        newPosition = {currentSquareLocation.position.x , currentSquareLocation.position.z + 50.0f};
    } else if (cameraPosition.z < currentSquareLocation.position.z - 25) {
        newPosition = {currentSquareLocation.position.x , currentSquareLocation.position.z - 50.0f};
    }
    return newPosition;
}

/**
 * this function adds the new locations to the vector of locations depending on the current square neighboring sides.
 */
void addNewLocations() {
    if (!currentSquareLocation.sides.right) {
        Position newPosition = {currentSquareLocation.position.x + 50.0f, currentSquareLocation.position.z};
        squarePositions.push_back(newPosition);
        neighboringSides sides = findNeighboringSides(newPosition);
        Location newLocation = {newPosition , sides};
        squareLocations.push_back(newLocation);
        cout << "new right square location: " << newLocation.position.x << ", " << newLocation.position.z << endl;

    }
    if (!currentSquareLocation.sides.left) {
        Position newPosition = {currentSquareLocation.position.x - 50.0f, currentSquareLocation.position.z};
        squarePositions.push_back(newPosition);
        neighboringSides sides = findNeighboringSides(newPosition);
        Location newLocation = {newPosition, sides};
        squareLocations.push_back(newLocation);
        cout << "new left square location: " << newLocation.position.x << ", " << newLocation.position.z << endl;
    }
    if (!currentSquareLocation.sides.up) {
        Position newPosition = {currentSquareLocation.position.x , currentSquareLocation.position.z - 50.0f};
        squarePositions.push_back(newPosition);
        neighboringSides sides = findNeighboringSides(newPosition);
        Location newLocation = {newPosition, sides};
        squareLocations.push_back(newLocation);
        cout << "new up square location: " << newLocation.position.x << ", " << newLocation.position.z << endl;

    }
    if (!currentSquareLocation.sides.down) {
        Position newPosition = {currentSquareLocation.position.x , currentSquareLocation.position.z + 50.0f};
        squarePositions.push_back(newPosition);
        neighboringSides sides = findNeighboringSides(newPosition);
        Location newLocation = {newPosition, sides};
        squareLocations.push_back(newLocation);
        cout << "new down square location: " << newLocation.position.x << ", " << newLocation.position.z << endl;
    }
}

/**
 * this function is able to find the neighboring sides of a square
 * @param newPosition the new current position (where the camera is now)
 * @return a neighboringSides instance that contains the neighboring sides of the current square
 */
neighboringSides findNeighboringSides(Position newPosition) {

    // there are 4 possibilities for the neighboring sides:
    // 1. right
    Position rightPosition = {newPosition.x + 50.0f, newPosition.z};
    // 2. left
    Position leftPosition = {newPosition.x - 50.0f, newPosition.z};
    // 3. up
    Position upPosition = {newPosition.x, newPosition.z - 50.0f};
    // 4. down
    Position downPosition = {newPosition.x, newPosition.z + 50.0f};

    neighboringSides sides = {true, true, true, true};

    if (std::find(squarePositions.begin(), squarePositions.end(), rightPosition) == squarePositions.end()) {
        // not found
        sides.right = false;
    }
    if (std::find(squarePositions.begin(), squarePositions.end(), leftPosition) == squarePositions.end()) {
        // not found
        sides.left = false;
    }
    if (std::find(squarePositions.begin(), squarePositions.end(), upPosition) == squarePositions.end()) {
        // not found
        sides.up = false;
    }
    if (std::find(squarePositions.begin(), squarePositions.end(), downPosition) == squarePositions.end()) {
        // not found
        sides.down = false;
    }
    return sides;
}
