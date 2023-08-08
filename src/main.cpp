/*
 *
 * made by Abd Alwahed Haj Omar
 *      and Hisham Kitaz
 * id: 40246177
 *
 * sources consulted for this quiz:
 * 1. chatGPT
 * 2. LearnOpenGL website and GitHub repository
 */

#include <iostream>

#define GLEW_STATIC 1
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"
#include "lib/Camera.h"
#include "lib/shader.h"
//#include "lib/shader_m.h"
#include "vao.h"

using namespace glm;
using namespace std;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void resetCamera();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
GLuint loadTexture(const char* filename);
void drawScene(Shader shader, GLuint initialCube, GLuint blueBigCube);
void buildBackground(Shader &shader, GLuint blueBigCube);
void buildBuildingAB(Shader &shader, GLuint initialCube);
void buildBuildingCD(Shader &shader, GLuint initialCube);
void buildScraper(Shader &shader, GLuint initialCube);
void buildTree(Shader &shader, GLuint initialCube);
void buildStreet(Shader &shader, GLuint initialCube);
void buildStreetDecor(Shader &shader, GLuint initialCube);

const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;
bool noShadows = false;
bool noShadowsKeyPressed = true;
Camera camera(glm::vec3(10.0f, 4.0f, 0.0f));
float lastX = (float) SCR_WIDTH / 2.0f;
float lastY = (float) SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct Location {
    float x;
    float y;
    float z;
};

mat4 translateMatrix;
mat4 rotateMatrix;
mat4 scaleMatrix;
mat4 modelMatrix;

mat4 translateMatrices[2];
mat4 rotateMatrices[2];
mat4 scaleMatrices[2];
mat4 modelMatrices[2];

mat4 armModelMatrix[2];
mat4 armTranslate[2];
mat4 armRotate[2]; // this is the global variable that controls all 4 rackets
mat4 armScale[2];

// this is to set the initial location of the rackets
Location locations[2] = {{0.0f, 0.0f, 13.0f},
                         {0.0f, 0.0f, -13.0f},};



// this variable will hold 0, 1, 2, 3 depending on the racket we have focus on. it also can have -1 if all rackets are selected
int activeRacket = 0;


// textures:
unsigned int buildingATexture;
unsigned int buildingBTexture;
unsigned int buildingCTexture;
unsigned int buildingDTexture;
unsigned int backgroundTexture;
unsigned int treeTexture;
unsigned int tree2Texture;
unsigned int woodTexture;
unsigned int streetTexture;
unsigned int floorTexture;
unsigned int scraperTexture;
unsigned int firehydrantTexture;
unsigned int stopTexture;

int main(int argc, char* argv[])
{
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

    // Set the keyboard key and mouse callback functions
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glClearColor(0.0f/255, 18.0f/255, 43.0f/255, 1.0f);

    GLuint initialCube = createCubeCoordinate();
    GLuint blueBigCube = createCubeCoordinate();

    // Enable Backface culling and depth test
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    Shader shader("shaders/textureShader.vs", "shaders/textureShader.fs");
    Shader depthShader("shaders/depthShader.vs", "shaders/depthShader.fs", "shaders/depthShader.gs");
    buildingATexture = loadTexture("rec/textures/building.png");
    buildingBTexture = loadTexture("rec/textures/buildingB.png");
    buildingCTexture = loadTexture("rec/textures/buildingC.png");
    buildingDTexture = loadTexture("rec/textures/buildingD.png");
    backgroundTexture = loadTexture("rec/textures/background.png");
    treeTexture = loadTexture("rec/textures/tree.png");
    tree2Texture = loadTexture("rec/textures/tree2.png");
    woodTexture = loadTexture("rec/textures/wood.png");
    streetTexture = loadTexture("rec/textures/street.png");
    floorTexture = loadTexture("rec/textures/brick.jpg");
    scraperTexture = loadTexture("rec/textures/scraper.png");
    firehydrantTexture = loadTexture("rec/textures/firehydrant.png");
    stopTexture = loadTexture("rec/textures/stop.png");
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
    shader.setInt("diffuseTexture", 0);
    shader.setInt("depthMap", 1);

    // lighting info
    // -------------
    glm::vec3 lightPos(0.0f, 6.0f, 0.0f);


    while (!glfwWindowShouldClose(window)) {

        auto currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        lightPos.z = static_cast<float>(sin(glfwGetTime() * 0.5) * 30.0);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 0. create depth cubemap transformation matrices
        // -----------------------------------------------
        float near_plane = 1.0f;
        // change the max distance of light
        float far_plane  = 500.0f;
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

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        depthShader.use();
        for (unsigned int i = 0; i < 6; ++i)
            depthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        depthShader.setFloat("far_plane", far_plane);
        depthShader.setVec3("lightPos", lightPos);
        drawScene(depthShader, initialCube, blueBigCube);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. render scene as normal
        // -------------------------
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projectionMatrix", projection);
        shader.setMat4("viewMatrix", view);

        // set lighting uniforms
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("viewPos", camera.Position);
        shader.setInt("noShadows", noShadows); // enable/disable noShadows by pressing 'SPACE'
        shader.setFloat("far_plane", far_plane);

        // TODO: change the texture as you want
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, backgroundTexture);

        // DONT TOUCH
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

        drawScene(shader, initialCube, blueBigCube);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // End Frame
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Shutdown GLFW
    glfwTerminate();

    return 0;
}
void buildBackground(Shader &shader, GLuint blueBigCube) {

    glBindVertexArray(blueBigCube);// Big Blue Cube
    translateMatrix = translate(mat4(1.0f), vec3(0.0f, 69.8f, 0.0f));
    rotateMatrix = rotate(mat4(1.0f), radians(0.0f), vec3(0.0f, 0.0f, 1.0f));
    scaleMatrix = scale(mat4(1.0f), vec3(500.0f, 70.0f, 500.0f));
    modelMatrix = translateMatrix * rotateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);

    //shader.setVec3("color", 129.0f/255, 174.0f/255, 208.0f/255);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
    glDisable(GL_CULL_FACE);
    shader.setInt("reverse_normals", 1);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    shader.setInt("reverse_normals", 0);
    glEnable(GL_CULL_FACE);

}

void drawScene(Shader shader, GLuint initialCube, GLuint blueBigCube) {
    buildBackground(shader, blueBigCube);
    buildBuildingAB(shader, initialCube);
    buildBuildingCD(shader, initialCube);
    buildScraper(shader, initialCube);
    buildTree(shader, initialCube);
    buildStreet(shader, initialCube);
    buildStreetDecor(shader, initialCube);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !noShadowsKeyPressed)
    {
        noShadows = !noShadows;
        noShadowsKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        noShadowsKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

}

void resetCamera() {
    // Set the camera position to the active racket
    camera.Position = vec3(locations[activeRacket].x, locations[activeRacket].y + 7.0f, locations[activeRacket].z);

    // Calculate the direction vector from the camera position to the world center
    vec3 directionToCenter = normalize(-camera.Position);

    // Calculate yaw and pitch angles from the direction vector
    float yaw = atan2(directionToCenter.x, -directionToCenter.z);
    float pitch = asin(directionToCenter.y);

    // Convert the angles from radians to degrees and set them in the camera
    camera.Yaw = glm::degrees(yaw) - 90;
    camera.Pitch = glm::degrees(pitch);

    // Update the camera vectors to match the new orientation.
    camera.updateCameraVectors();
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
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


GLuint loadTexture(const char* filename)
{
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

void buildBuildingAB(Shader &shader, GLuint initialCube) {

    glBindVertexArray(initialCube);// CUBE BASE
    for(int i=0;i<3;i++) {
        //Builds BuildingA
        translateMatrix = translate(mat4(1.0f), vec3(-115.0f+i*100, 20.0f, -115.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 20.0f, 25.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, buildingATexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //builds Building B
        translateMatrix = translate(mat4(1.0f), vec3(-90.0f+i*100, 20.0f, -115.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 20.0f, 25.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, buildingBTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //Builds BuildingA other side
        translateMatrix = translate(mat4(1.0f), vec3(-115.0f+i*100, 20.0f, 115.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 20.0f, 25.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, buildingATexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //builds Building B other side
        translateMatrix = translate(mat4(1.0f), vec3(-90.0f+i*100, 20.0f, 115.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 20.0f, 25.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, buildingBTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}
// this builds buildings CD

void buildBuildingCD(Shader &shader, GLuint initialCube) {
    glBindVertexArray(initialCube);// CUBE BASE
    for(int i=0;i<2;i++) {
        //Builds Building C
        translateMatrix = translate(mat4(1.0f), vec3(-65.0f+i*100, 20.0f, -115.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 20.0f, 25.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        //Builds BuildingD
        glBindTexture(GL_TEXTURE_2D, buildingCTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        translateMatrix = translate(mat4(1.0f), vec3(-40.0f+i*100, 20.0f, -115.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 20.0f, 25.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, buildingDTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //Builds Building C other side
        translateMatrix = translate(mat4(1.0f), vec3(-65.0f+i*100, 20.0f, 115.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 20.0f, 25.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, buildingCTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //Builds Building D other side
        translateMatrix = translate(mat4(1.0f), vec3(-40.0f+i*100, 20.0f, 115.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 20.0f, 25.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, buildingDTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}
//this builds the main building in the center
void buildScraper(Shader &shader, GLuint initialCube) {
    glBindVertexArray(initialCube);// CUBE BASE
    translateMatrix = translate(mat4(1.0f), vec3(12.5f, 40.0f, 0.0f));
    scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 40.0f, 25.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scraperTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}
void buildTree(Shader &shader, GLuint initialCube) {
    glBindVertexArray(initialCube);// CUBE BASE

    for(int i=0;i<10;i++) {
        // building bark
        translateMatrix = translate(mat4(1.0f), vec3(-102.5f+i*25, 7.5f, -115.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(7.5f, 7.5f, 7.5f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //building upper tree
        translateMatrix = translate(mat4(1.0f), vec3(-102.5f+i*25, 15.0f, -115.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(17.5f, 7.5f, 17.5f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        if (i % 2 == 1) {
            glBindTexture(GL_TEXTURE_2D, tree2Texture);
        } else {
            glBindTexture(GL_TEXTURE_2D, treeTexture);
        }
        glDrawArrays(GL_TRIANGLES, 0, 36);
        translateMatrix = translate(mat4(1.0f), vec3(-102.5f+i*25, 7.5f, 115.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(7.5f, 7.5f, 7.5f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //building upper tree
        translateMatrix = translate(mat4(1.0f), vec3(-102.5f+i*25, 15.0f, 115.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(17.5f, 7.5f, 17.5f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        if (i % 2 == 1) {
            glBindTexture(GL_TEXTURE_2D, tree2Texture);
        } else {
            glBindTexture(GL_TEXTURE_2D, treeTexture);
        }
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}
void buildStreet(Shader &shader, GLuint initialCube) {
    glBindVertexArray(initialCube);// CUBE BASE
    // this builds the street
    for (int i = 0; i < 2; i++) {
    translateMatrix = translate(mat4(1.0f), vec3(0.0f, -0.01f, -100.0f+i*200));
    scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 0.0f, 500.0f));
    rotateMatrix = rotate(mat4(1.0f), radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = translateMatrix * rotateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, streetTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    for(int i =0;i<3;i++) {
        translateMatrix = translate(mat4(1.0f), vec3(-100.0f*i+100, -0.01f, 0.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 0.0f, 420.0f));
        rotateMatrix = rotate(mat4(1.0f), radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = translateMatrix * rotateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}
void buildStreetDecor(Shader &shader, GLuint initialCube) {
    //this builds the floor
    translateMatrix = translate(mat4(1.0f), vec3(0.0f, -0.09f, 0.0f));
    scaleMatrix = scale(mat4(1.0f), vec3(500.0f, 0.0f, 500.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    //this builds the firehydrants
    for (int i = 0; i < 5; i++) {
        translateMatrix = translate(mat4(1.0f), vec3(-100.0f+i*50, 4.0f, -108.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(5.0f, 4.0f, 5.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, firehydrantTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        translateMatrix = translate(mat4(1.0f), vec3(-100.0f+i*50, 4.0f, 108.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(5.0f, 4.0f, 5.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, firehydrantTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    for (int i = 0; i < 3; i++) {
        translateMatrix = translate(mat4(1.0f), vec3(-90.0f+i*99, 8.0f, -90.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(0.5f, 8.0f, 5.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, stopTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        translateMatrix = translate(mat4(1.0f), vec3(-90.0f+i*99, 8.0f, 90.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(0.5f, 8.0f, 5.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, stopTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}