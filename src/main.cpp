/*
 *
 * made by Abd Alwahed Haj Omar
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
#include "lib/shader_m.h"
#include "vao.h"

using namespace glm;
using namespace std;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void resetCamera();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
GLuint loadTexture(const char* filename);
//void buildCoordinateVectors(Shader shader, GLuint axisX, GLuint axisY, GLuint axisZ);
void buildRacket(Shader &shader, GLuint initialCube, float offsetX, float offsetY, float offsetZ, int id);
void build_O_letter(Shader &shader, GLuint initialCube, float offsetX, float offsetY, float offsetZ, int id);
void build_M_letter(Shader &shader, GLuint initialCube, float offsetX, float offsetY, float offsetZ, int id);
void build_A_letter(Shader &shader, GLuint initialCube, float offsetX, float offsetY, float offsetZ, int id);
void build_R_letter(Shader &shader, GLuint initialCube, float offsetX, float offsetY, float offsetZ, int id);
void buildNet(Shader &shader, GLuint initialCube);
//void buildGround(Shader &shader, GLuint VAO, GLuint initialCube);
void drawScene(Shader shader, GLuint initialCube, GLuint blueBigCube);
void buildBlueCube(Shader &shader, GLuint blueBigCube);
void drawShape();

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

// the following 3 variables hold the values of the scaleMatrix to facilitate modifications later on
float scaleX[2] = {0.35f, 0.35f};
float scaleY[2] = {0.35f, 0.35f};
float scaleZ[2] = {0.35f, 0.35f};

// the following is the current angle of the hand
float angleOfRackets[2] = {radians(0.0f), radians(0.0f), };

// this part is for rendering type: triangles, lines, points
bool isTriangles = true;
bool isLines = false;
bool isPoints = false;

// this variable will hold 0, 1, 2, 3 depending on the racket we have focus on. it also can have -1 if all rackets are selected
int activeRacket = 0;

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

    unsigned int tattooTexture = loadTexture("rec/textures/tattoo.jpg");
    unsigned int woodTexture = loadTexture("rec/textures/wood.png");
    //unsigned int paperTexture = loadTexture("rec/textures/paper.jpg");
    unsigned int metalTexture = loadTexture("rec/textures/metal.jpg");
    //unsigned int redRocksTexture = loadTexture("rec/textures/redRocks.jpg");
    unsigned int goldTexture = loadTexture("rec/textures/gold.jpg");

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

        //os.z = static_cast<float>(sin(glfwGetTime() * 0.5) * 5.0);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 0. create depth cubemap transformation matrices
        // -----------------------------------------------
        float near_plane = 1.0f;
        float far_plane  = 100.0f;
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
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projectionMatrix", projection);
        shader.setMat4("viewMatrix", view);

        // set lighting uniforms
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("viewPos", camera.Position);
        shader.setInt("noShadows", noShadows); // enable/disable noShadows by pressing 'SPACE'
        shader.setFloat("far_plane", far_plane);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
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

void buildRacket(Shader &shader, GLuint initialCube, float offsetX, float offsetY, float offsetZ, int id) {

    glBindVertexArray(initialCube);// CUBE BASE

    armTranslate[id] = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));

    vec3 trans = vec3(offsetX/3, offsetY/3, offsetZ/3);
    mat4 translate1 = translate(mat4(1.0f), trans);
    mat4 translate2 = translate(mat4(1.0f), -trans);

    armRotate[id] = rotate(mat4(1.0f), angleOfRackets[id], vec3(0.0f, 1.0f, 0.0f));
    armScale[id] = scale(mat4(1.0f), vec3(scaleX[id], scaleY[id], scaleZ[id]));
    armModelMatrix[id] = armTranslate[id] * translate1 * armRotate[id] * translate2 * armScale[id];

    translateMatrices[id] = translate(mat4(1.0f), vec3(3.0f + offsetX, 3.0f + offsetY, -1.0f + offsetZ));
    rotateMatrices[id] = rotate(mat4(1.0f), radians(-45.0f), vec3(0.0f, 0.0f, 1.0f));
    scaleMatrices[id] = scale(mat4(1.0f), vec3(1.0f, 2.0f, 1.0f));
    modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);
    shader.setMat4("modelMatrix", modelMatrices[id]);


    //shader.setVec3("color", 255.0f/255,238.0f/255,109.0f/255);
    // DRAWING LOWER ARM
    drawShape();

    rotateMatrices[id] = rotate(mat4(1.0f), radians(0.0f), vec3(0.0f, 0.0f, 1.0f));
    translateMatrices[id] = translate(mat4(1.0f), vec3(4.33f + offsetX, 6.29f + offsetY, -1.0f + offsetZ));
    modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);
    shader.setMat4("modelMatrix", modelMatrices[id]);
    // DRAWING UPPER ARM
    drawShape();

    rotateMatrices[id] = rotate(mat4(1.0f), radians(45.0f), vec3(0.0f, 0.0f, 1.0f));
    translateMatrices[id] = translate(mat4(1.0f), vec3(4.33f + offsetX, 6.29f * 1.34 + offsetY, -1.0f + offsetZ));
    scaleMatrices[id] = scale(mat4(1.0f), vec3(3.0f, 0.5f, 1.0f));
    modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);
    shader.setMat4("modelMatrix", modelMatrices[id]);
    drawShape();

    // Fingers
    for (int i = 0; i <5 ; i++) {
        float translation = (float)i * 0.29f;
        rotateMatrices[id] = rotate(mat4(1.0f), radians( 45.0f), vec3(0.0f, 0.0f, 1.0f));
        translateMatrices[id] = translate(mat4(1.0f), vec3(3.4f + translation + offsetX, (6.1f * 1.4) + translation + offsetY, -1.0f + offsetZ));

        if (i == 4) {
            rotateMatrices[id] = rotate(mat4(1.0f), radians(-45.0f), vec3(0.0f, 0.0f, 1.0f));
            translateMatrices[id] = translate(mat4(1.0f), vec3(5.1f + offsetX , (6.1f * 1.5) + offsetY, -1.0f + offsetZ));
        }
        scaleMatrices[id] = scale(mat4(1.0f), vec3(0.50f, 0.5f, 0.5f));
        modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);
        shader.setMat4("modelMatrix", modelMatrices[id]);

        // DRAWING Fingers
        drawShape();
    }

    float defaultRacketOrientation = -20.0f;
    float racketHandleX = 4.7f + offsetX;
    float racketHandleY = 6.7f * 1.5f + offsetY;
    float racketHandleZ = -1.0f + offsetZ;

    // DRAWING Racket handle
    rotateMatrices[id] = rotate(mat4(1.0f), radians(defaultRacketOrientation), vec3(0.0f, 0.0f, 1.0f));
    translateMatrices[id] = translate(mat4(1.0f), vec3(racketHandleX, racketHandleY, racketHandleZ));
    scaleMatrices[id] = scale(mat4(1.0f), vec3(1.0f, 1.0f, 1.0f));
    modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);

    //shader.setVec3("color", 253.0f/255.,251.0f/255,236.0f/255);
    shader.setMat4("modelMatrix", modelMatrices[id]);
    drawShape();

    // DRAWING right base of racket
    rotateMatrices[id] = rotate(mat4(1.0f), radians(defaultRacketOrientation-45.0f), vec3(0.0f, 0.0f, 1.0f));
    translateMatrices[id] = translate(mat4(1.0f), vec3(racketHandleX + 1.0f, racketHandleY + 1.0f, racketHandleZ));
    scaleMatrices[id] = scale(mat4(1.0f), vec3(1.0f, 1.0f, 0.8f));
    modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);

    //shader.setVec3("color", 222.0f/255,62.0f/255,28.0f/255);
    shader.setMat4("modelMatrix", modelMatrices[id]);
    drawShape();

    // DRAWING top right of racket
    rotateMatrices[id] = rotate(mat4(1.0f), radians(defaultRacketOrientation + 45.0f), vec3(0.0f, 0.0f, 1.0f));
    translateMatrices[id] = translate(mat4(1.0f), vec3(racketHandleX + 2.0f, racketHandleY + 4.0f, racketHandleZ));
    scaleMatrices[id] = scale(mat4(1.0f), vec3(1.0f, 1.0f, 0.8f));
    modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);

    shader.setMat4("modelMatrix", modelMatrices[id]);
    drawShape();

    // DRAWING left base of racket
    rotateMatrices[id] = rotate(mat4(1.0f), radians(defaultRacketOrientation + 45.0f), vec3(0.0f, 0.0f, 1.0f));
    translateMatrices[id] = translate(mat4(1.0f), vec3(racketHandleX -0.25f, racketHandleY + 1.5f, racketHandleZ));
    scaleMatrices[id] = scale(mat4(1.0f), vec3(1.0f, 1.0f, 0.8f));
    modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);

    shader.setMat4("modelMatrix", modelMatrices[id]);
    drawShape();

    // DRAWING top left of racket
    rotateMatrices[id] = rotate(mat4(1.0f), radians(defaultRacketOrientation-45.0f), vec3(0.0f, 0.0f, 1.0f));
    translateMatrices[id] = translate(mat4(1.0f), vec3(racketHandleX +0.8f, racketHandleY + 4.6f, racketHandleZ));
    scaleMatrices[id] = scale(mat4(1.0f), vec3(1.0f, 1.0f, 0.8f));
    modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);

    shader.setMat4("modelMatrix", modelMatrices[id]);
    drawShape();

    // DRAWING right leg of racket
    rotateMatrices[id] = rotate(mat4(1.0f), radians(defaultRacketOrientation), vec3(0.0f, 0.0f, 1.0f));
    translateMatrices[id] = translate(mat4(1.0f), vec3(racketHandleX + 2.15f, racketHandleY + 2.25f, racketHandleZ));
    scaleMatrices[id] = scale(mat4(1.0f), vec3(1.0f, 1.0f, 0.75f));
    modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);

    //shader.setVec3("color", 253.0f/255.,251.0f/255,236.0f/255);
    shader.setMat4("modelMatrix", modelMatrices[id]);
    drawShape();

    // DRAWING left leg of racket
    rotateMatrices[id] = rotate(mat4(1.0f), radians(defaultRacketOrientation), vec3(0.0f, 0.0f, 1.0f));
    translateMatrices[id] = translate(mat4(1.0f), vec3(racketHandleX - .35f, racketHandleY + 3.25f, racketHandleZ));
    scaleMatrices[id] = scale(mat4(1.0f), vec3(1.0f, 1.0f, 0.75f));
    modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);

    shader.setMat4("modelMatrix", modelMatrices[id]);
    drawShape();

    //drawing racket grid down up
    float space = 0.0f;
    for (int i = 0; i < 7; i++) {
        space += 0.5f;
        rotateMatrices[id] = rotate(mat4(1.0f), radians(  defaultRacketOrientation + 90.0f), vec3(0.0f, 0.0f, 1.0f));
        translateMatrices[id] = translate(mat4(1.0f), vec3(4.9f + space*0.3f + offsetX, 6.3f * 1.72 + space + offsetY, -1.0f + offsetZ));
        scaleMatrices[id] = scale(mat4(1.0f), vec3(0.01f, 1.2f, 0.6f));
        if(i==6 || i==0)
            scaleMatrices[id] = scale(mat4(1.0f), vec3(0.01f, 0.5f, 0.6f));
        modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);

        //shader.setVec3("color", 0.2,0.2,0.2);
        shader.setMat4("modelMatrix", modelMatrices[id]);
        drawShape();
    }

    //Drawing grid of racket left to right
    float spacing = 0.0f;
    for (int i = 0; i < 6 ; i++) {
        spacing += 0.5f;
        rotateMatrices[id] = rotate(mat4(1.0f), radians(defaultRacketOrientation), vec3(0.0f, 0.0f, 1.0f));
        translateMatrices[id] = translate(mat4(1.0f), vec3(3.9f + spacing + offsetX, 7.5f * 1.80 - spacing*0.4f + offsetY, -1.0f + offsetZ));
        scaleMatrices[id] = scale(mat4(1.0f), vec3(0.09f, 1.15f, 0.6f));
        if(i==1||i==2||i==3||i==4)
            scaleMatrices[id] = scale(mat4(1.0f), vec3(0.09f, 1.55f, 0.6f));
        modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);

        shader.setMat4("modelMatrix", modelMatrices[id]);
        drawShape();
    }

    translateMatrices[id] = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));//important to reset the translation matrix
    rotateMatrices[id] = rotate(mat4(1.0f), radians(0.0f), vec3(0.0f, 1.0f, 0.0f));
    modelMatrices[id] = translateMatrices[id] * rotateMatrices[id];
    shader.setMat4("modelMatrix", modelMatrices[id]);
}

void build_O_letter(Shader &shader, GLuint initialCube, float offsetX, float offsetY, float offsetZ, int id) {

    // the letter O can be modelled using 4 edges: lower, upper, right, left
    offsetY+=4.0f;
    glBindVertexArray(initialCube);// CUBE BASE

    for (int i = 0; i < 3; i++) {
        // DRAWING LOWER EDGE
        translateMatrices[id] = translate(mat4(1.0f), vec3(3.0f + offsetX, 3.0f + offsetY + 10.0f, -2.0f + offsetZ + 0.6f*(float)i));
        rotateMatrices[id] = rotate(mat4(1.0f), radians(0.0f), vec3(0.0f, 0.0f, 1.0f));
        scaleMatrices[id] = scale(mat4(1.0f), vec3(7.0f, 0.5f, 0.5f));
        modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);
        shader.setMat4("modelMatrix", modelMatrices[id]);

        //shader.setVec3("color", 0.3f * (float)(i+1),0.0f,0.0f);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // DRAWING UPPER EDGE
        translateMatrices[id] = translate(mat4(1.0f), vec3(3.0f + offsetX, 3.0f + offsetY + 14.0f, -2.0f + offsetZ + 0.6f*(float)i));
        rotateMatrices[id] = rotate(mat4(1.0f), radians(0.0f), vec3(0.0f, 0.0f, 1.0f));
        scaleMatrices[id] = scale(mat4(1.0f), vec3(7.0f, 0.5f, 0.5f));
        modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);
        shader.setMat4("modelMatrix", modelMatrices[id]);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // DRAW RIGHT EDGE
        translateMatrices[id] = translate(mat4(1.0f), vec3(3.0f + offsetX - 2.0f, 3.0f + offsetY + 12.0f, -2.0f + offsetZ + 0.6f*(float)i));
        rotateMatrices[id] = rotate(mat4(1.0f), radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
        scaleMatrices[id] = scale(mat4(1.0f), vec3(7.0f, 0.5f, 0.5f));
        modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);
        shader.setMat4("modelMatrix", modelMatrices[id]);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // DRAW LEFT EDGE
        translateMatrices[id] = translate(mat4(1.0f), vec3(3.0f + offsetX + 2.0f, 3.0f + offsetY + 12.0f, -2.0f + offsetZ + 0.6f*(float)i));
        rotateMatrices[id] = rotate(mat4(1.0f), radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
        scaleMatrices[id] = scale(mat4(1.0f), vec3(7.0f, 0.5f, 0.5f));
        modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);
        shader.setMat4("modelMatrix", modelMatrices[id]);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        translateMatrices[id] = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));//important to reset the translation matrix
        rotateMatrices[id] = rotate(mat4(1.0f), radians(0.0f), vec3(0.0f, 1.0f, 0.0f));
        modelMatrices[id] = translateMatrices[id] * rotateMatrices[id];
        shader.setMat4("modelMatrix", modelMatrices[id]);
    }
}

void build_M_letter(Shader &shader, GLuint initialCube, float offsetX, float offsetY, float offsetZ, int id) {

    // the letter M can be modelled using 4 edges: goingUp1, goingDown1, goingUp2, goingDown2
    offsetY+=9.0f;
    glBindVertexArray(initialCube);// CUBE BASE

    for (int i = 0; i < 3; i++) {

        // DRAWING GOING_UP_1 EDGE
        translateMatrices[id] = translate(mat4(1.0f), vec3(3.0f + offsetX - 2.25f, 3.0f + offsetY + 12.0f, -2.0f + offsetZ + 0.6f*(float)i));
        rotateMatrices[id] = rotate(mat4(1.0f), radians(60.0f), vec3(0.0f, 0.0f, 1.0f));
        scaleMatrices[id] = scale(mat4(1.0f), vec3(7.0f, 0.5f, 0.5f));
        modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);
        shader.setMat4("modelMatrix", modelMatrices[id]);
        //shader.setVec3("color", 0.0f, 0.3f * (float)(i+1), 0.0f);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // DRAWING GOING_DOWN_1 EDGE
        translateMatrices[id] = translate(mat4(1.0f), vec3(3.0f + offsetX - 0.75f, 3.0f + offsetY + 12.0f, -2.0f + offsetZ + 0.6f*(float)i));
        rotateMatrices[id] = rotate(mat4(1.0f), radians(-60.0f), vec3(0.0f, 0.0f, 1.0f));
        scaleMatrices[id] = scale(mat4(1.0f), vec3(7.0f, 0.5f, 0.5f));
        modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);
        shader.setMat4("modelMatrix", modelMatrices[id]);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // DRAW GOING_UP_2 EDGE
        translateMatrices[id] = translate(mat4(1.0f), vec3(3.0f + offsetX + 0.75f, 3.0f + offsetY + 12.0f, -2.0f + offsetZ + 0.6f*(float)i));
        rotateMatrices[id] = rotate(mat4(1.0f), radians(60.0f), vec3(0.0f, 0.0f, 1.0f));
        scaleMatrices[id] = scale(mat4(1.0f), vec3(7.0f, 0.5f, 0.5f));
        modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);
        shader.setMat4("modelMatrix", modelMatrices[id]);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // DRAW GOING_DOWN_2 EDGE
        translateMatrices[id] = translate(mat4(1.0f), vec3(3.0f + offsetX + 2.25f, 3.0f + offsetY + 12.0f, -2.0f + offsetZ + 0.6f*(float)i));
        rotateMatrices[id] = rotate(mat4(1.0f), radians(-60.0f), vec3(0.0f, 0.0f, 1.0f));
        scaleMatrices[id] = scale(mat4(1.0f), vec3(7.0f, 0.5f, 0.5f));
        modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);
        shader.setMat4("modelMatrix", modelMatrices[id]);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        translateMatrices[id] = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));//important to reset the translation matrix
        rotateMatrices[id] = rotate(mat4(1.0f), radians(0.0f), vec3(0.0f, 1.0f, 0.0f));
        modelMatrices[id] = translateMatrices[id] * rotateMatrices[id];
        shader.setMat4("modelMatrix", modelMatrices[id]);
    }
}

void build_A_letter(Shader &shader, GLuint initialCube, float offsetX, float offsetY, float offsetZ, int id) {

    // the letter O can be modelled using 3 edges: goingUp, goingDown, Middle
    offsetY+=4.0f;
    glBindVertexArray(initialCube);// CUBE BASE

    for (int i = 0; i < 3; i++) {
        // DRAWING GOING_UP EDGE
        translateMatrices[id] = translate(mat4(1.0f), vec3(3.0f + offsetX - 1.0f, 3.0f + offsetY + 12.0f, -2.0f + offsetZ + 0.6f*(float)i));
        rotateMatrices[id] = rotate(mat4(1.0f), radians(60.0f), vec3(0.0f, 0.0f, 1.0f));
        scaleMatrices[id] = scale(mat4(1.0f), vec3(7.0f, 0.5f, 0.5f));
        modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);
        shader.setMat4("modelMatrix", modelMatrices[id]);
        //shader.setVec3("color", 0.0f,0.0f,0.3f * (float)(i+1));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // DRAWING GOING_DOWN EDGE
        translateMatrices[id] = translate(mat4(1.0f), vec3(3.0f + offsetX + 1.0f, 3.0f + offsetY + 12.0f, -2.0f + offsetZ + 0.6f*(float)i));
        rotateMatrices[id] = rotate(mat4(1.0f), radians(-60.0f), vec3(0.0f, 0.0f, 1.0f));
        scaleMatrices[id] = scale(mat4(1.0f), vec3(7.0f, 0.5f, 0.5f));
        modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);
        shader.setMat4("modelMatrix", modelMatrices[id]);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // DRAW MIDDLE EDGE
        translateMatrices[id] = translate(mat4(1.0f), vec3(3.0f + offsetX, 3.0f + offsetY + 11.0f, -2.0f + offsetZ + 0.6f*(float)i));
        rotateMatrices[id] = rotate(mat4(1.0f), radians(0.0f), vec3(0.0f, 0.0f, 1.0f));
        scaleMatrices[id] = scale(mat4(1.0f), vec3(7.0f, 0.5f, 0.5f));
        modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);
        shader.setMat4("modelMatrix", modelMatrices[id]);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        translateMatrices[id] = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));//important to reset the translation matrix
        rotateMatrices[id] = rotate(mat4(1.0f), radians(0.0f), vec3(0.0f, 1.0f, 0.0f));
        modelMatrices[id] = translateMatrices[id] * rotateMatrices[id];
        shader.setMat4("modelMatrix", modelMatrices[id]);
    }
}

void build_R_letter(Shader &shader, GLuint initialCube, float offsetX, float offsetY, float offsetZ, int id) {

    // the letter O can be modelled using 4 edges: LongLeg, zigZag1, zigZag2, zigZag3
    offsetY+=9.0f;
    glBindVertexArray(initialCube);// CUBE BASE

    for (int i = 0; i < 3; i++) {
        // DRAWING LONG_LEG EDGE
        translateMatrices[id] = translate(mat4(1.0f), vec3(3.0f + offsetX - 0.75f, 3.0f + offsetY + 12.0f, -2.0f + offsetZ + 0.6f*(float)i));
        rotateMatrices[id] = rotate(mat4(1.0f), radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
        scaleMatrices[id] = scale(mat4(1.0f), vec3(10.0f, 0.5f, 0.5f));
        modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);
        shader.setMat4("modelMatrix", modelMatrices[id]);
        //shader.setVec3("color",0.2f * (float)(i+1),0.2f * (float)(i+1),0.2f * (float)(i+1));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // DRAWING ZIG_ZAG_1 EDGE
        translateMatrices[id] = translate(mat4(1.0f), vec3(3.0f + offsetX, 3.0f + offsetY + 14.0f, -2.0f + offsetZ + 0.6f*(float)i));
        rotateMatrices[id] = rotate(mat4(1.0f), radians(-30.0f), vec3(0.0f, 0.0f, 1.0f));
        scaleMatrices[id] = scale(mat4(1.0f), vec3(5.0f, 0.5f, 0.5f));
        modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);
        shader.setMat4("modelMatrix", modelMatrices[id]);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // DRAW ZIG_ZAG_2 EDGE
        translateMatrices[id] = translate(mat4(1.0f), vec3(3.0f + offsetX, 3.0f + offsetY + 12.5f, -2.0f + offsetZ + 0.6f*(float)i));
        rotateMatrices[id] = rotate(mat4(1.0f), radians(30.0f), vec3(0.0f, 0.0f, 1.0f));
        scaleMatrices[id] = scale(mat4(1.0f), vec3(5.0f, 0.5f, 0.5f));
        modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);
        shader.setMat4("modelMatrix", modelMatrices[id]);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // DRAW ZIG_ZAG_3 EDGE
        translateMatrices[id] = translate(mat4(1.0f), vec3(3.0f + offsetX , 3.0f + offsetY + 11.0f, -2.0f + offsetZ + 0.6f*(float)i));
        rotateMatrices[id] = rotate(mat4(1.0f), radians(-60.0f), vec3(0.0f, 0.0f, 1.0f));
        scaleMatrices[id] = scale(mat4(1.0f), vec3(5.0f, 0.5f, 0.5f));
        modelMatrices[id] = armModelMatrix[id] * (translateMatrices[id] * rotateMatrices[id] * scaleMatrices[id]);
        shader.setMat4("modelMatrix", modelMatrices[id]);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        translateMatrices[id] = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));//important to reset the translation matrix
        rotateMatrices[id] = rotate(mat4(1.0f), radians(0.0f), vec3(0.0f, 1.0f, 0.0f));
        modelMatrices[id] = translateMatrices[id] * rotateMatrices[id];
        shader.setMat4("modelMatrix", modelMatrices[id]);
    }
}


void buildNet(Shader &shader, GLuint initialCube) {

    glBindVertexArray(initialCube);// CUBE BASE

    for (int i = 0; i < 39; i++) {
        translateMatrix = translate(mat4(1.0f), vec3(-9.5f + (float)i * 0.5f, 1.0f, 0.0 ));
        rotateMatrix = rotate(mat4(1.0f), radians(0.0f), vec3(0.0f, 0.0f, 1.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(2.0f , 1.0f, 0.3f ));
        modelMatrix = translateMatrix * rotateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);

        //shader.setVec3("color", 1, 1, 1);
        glDrawArrays(GL_LINES, 0, 36);
    }

    // drawing the small upper edge of the net
    translateMatrix = translate(mat4(1.0f), vec3(0.0f, 2.2f, 0.0f));
    scaleMatrix = scale(mat4(1.0f), vec3(41.1f , 0.15f, 0.3f ));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    //shader.setVec3("color", 0.3f, 0.3f, 0.3f);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // drawing the left edge of the net
    translateMatrix = translate(mat4(1.0f), vec3(-10.0f, 1.0f, 0.0f));
    scaleMatrix = scale(mat4(1.0f), vec3(1.0f , 1.3f, 0.5f ));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // drawing the right edge of the net
    translateMatrix = translate(mat4(1.0f), vec3(10.0f, 1.0f, 0.0f));
    scaleMatrix = scale(mat4(1.0f), vec3(1.0f , 1.3f, 0.5f ));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void buildCoordinateVectors(Shader shader, GLuint axisX, GLuint axisY, GLuint axisZ) {
    //sets line width
    glLineWidth(6.5f);
    glBindVertexArray(axisX);
    //shader.setVec3("color", 1.0f, 0.0f, 0.0f);
    glDrawArrays(GL_LINES, 0, 2);

    glBindVertexArray(axisY);
    //shader.setVec3("color", 0.0f, 1.0f, 0.0f);
    glDrawArrays(GL_LINES, 0, 2);

    glBindVertexArray(axisZ);
    //shader.setVec3("color", 0.0f, 0.0f, 1.0f);
    glDrawArrays(GL_LINES, 0, 2);

    rotateMatrix = rotate(mat4(1.0f), radians(0.0f), vec3(0.0f, 0.0f, 1.0f));
    translateMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));
    scaleMatrix = scale(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));
    modelMatrix = translateMatrix * rotateMatrix * scaleMatrix;

    glLineWidth(1.0f);//resets line width to original value
}

void buildGround(Shader &shader, GLuint VAO, GLuint initialCube) {
    glBindVertexArray(VAO);//drawing the grid

    //shader.setVec3("color", 0.0f, 0.0f, 0.0f);

    float i = 0.0f;
    while (i < 20.0f) {//left to right lines
        i += 0.2f;
        translateMatrix = translate(mat4(1.0f), vec3(0.0f + i, 0.0f, 0.0f));
        modelMatrix = translateMatrix * rotateMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glDrawArrays(GL_LINES, 0, 2);
    }

    translateMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));
    modelMatrix = translateMatrix * rotateMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    rotateMatrix = rotate(mat4(1.0f), radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = translateMatrix * rotateMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawArrays(GL_LINES, 0, 2);
    i = 0;
    while (i < 20.0f) {
        i += 0.2f;
        translateMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, -20.0f + i));
        modelMatrix = translateMatrix * rotateMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glDrawArrays(GL_LINES, 0, 2);
    }

    glBindVertexArray(initialCube);//drawing the ground
    translateMatrix = translate (mat4(1.0f), vec3(0.0f, -0.05f, 0.0f));
    scaleMatrix = scale(mat4(1.0f), vec3(44.0f, 0.05f, 44.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    //shader.setVec3("color", 0.0f, 0.8f, 0.0f); //green
    glDrawArrays(GL_TRIANGLES, 0, 36);

}

void buildBlueCube(Shader &shader, GLuint blueBigCube) {
    glBindVertexArray(blueBigCube);// Big Blue Cube
    translateMatrix = translate(mat4(1.0f), vec3(0.0f, 15.0f, 0.0f));
    rotateMatrix = rotate(mat4(1.0f), radians(0.0f), vec3(0.0f, 0.0f, 1.0f));
    scaleMatrix = scale(mat4(1.0f), vec3(70.0f, 15.0f, 70.0f));
    modelMatrix = translateMatrix * rotateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);

    //shader.setVec3("color", 129.0f/255, 174.0f/255, 208.0f/255);
    glDisable(GL_CULL_FACE);
    shader.setInt("reverse_normals", 1);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    shader.setInt("reverse_normals", 0);
    glEnable(GL_CULL_FACE);

}

void drawScene(Shader shader, GLuint initialCube, GLuint blueBigCube) {
    buildRacket(shader, initialCube, locations[0].x, locations[0].y, locations[0].z, 0);
    buildRacket(shader, initialCube, locations[1].x, locations[1].y, locations[1].z, 1);

    build_O_letter(shader, initialCube, locations[0].x, locations[0].y, locations[0].z, 0);
    build_M_letter(shader, initialCube, locations[0].x, locations[0].y, locations[0].z, 0);
    build_A_letter(shader, initialCube, locations[1].x, locations[1].y, locations[1].z, 1);
    build_R_letter(shader, initialCube, locations[1].x, locations[1].y, locations[1].z, 1);

    buildNet(shader, initialCube);
    //buildGround(shader, VAO, initialCube);


    buildBlueCube(shader, blueBigCube);
}

void drawShape() {
    if (isTriangles)
        glDrawArrays(GL_TRIANGLES, 0, 36);
    else if (isLines)
        glDrawArrays(GL_LINES, 0, 36);
    else if (isPoints)
        glDrawArrays(GL_POINTS, 0, 36);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        isTriangles = false;
        isLines = false;
        isPoints = true;
    }

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        isTriangles = false;
        isLines = true;
        isPoints = false;
    }

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        isTriangles = true;
        isLines = false;
        isPoints = false;
    }

    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
        scaleX[activeRacket] += 0.05f;
        scaleY[activeRacket] += 0.05f;
        scaleZ[activeRacket] += 0.05f;


    }

    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
        scaleX[activeRacket] -= 0.05f;
        scaleY[activeRacket] -= 0.05f;
        scaleZ[activeRacket] -= 0.05f;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        locations[activeRacket].x-=1.0f;

    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        locations[activeRacket].x+=1.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        locations[activeRacket].z-=1.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        locations[activeRacket].z+=1.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        angleOfRackets[activeRacket] += radians(5.0f);
    }

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        angleOfRackets[activeRacket] -= radians(5.0f);
    }


    // move camera position using ASDW keys
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        locations[activeRacket].y+=1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        locations[activeRacket].y-=1.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        locations[activeRacket].x-=1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        locations[activeRacket].x+=1.0f;

    // move camera down
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        activeRacket = 0;
        resetCamera();
    }

    // move camera up
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        activeRacket = 1;
        resetCamera();
    }

    // move camera to the right
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        activeRacket = 2;
        resetCamera();
    }

    // move camera to the left
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
        activeRacket = 3;
        resetCamera();
    }

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