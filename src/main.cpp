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

#define GLEW_STATIC 1
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"
#include "lib/Camera.h"
#include "lib/shader.h"
#include "vao.h"

using namespace glm;
using namespace std;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
GLuint loadTexture(const char* filename);
void drawScene(Shader shader, GLuint initialCube, GLuint blueBigCube, GLuint sphere);
void buildBackground(Shader &shader, GLuint blueBigCube);
void buildBuildingABCD(Shader &shader, GLuint initialCube);

void buildScrapers(Shader &shader, GLuint initialCube);
void buildTree(Shader &shader, GLuint initialCube);
void buildStreetAndDecor(Shader &shader, GLuint initialCube, GLuint sphere);
void buildShops(Shader &shader, GLuint initialCube);
void buildFountain(Shader &shader, GLuint initialCube, GLuint sphere);
void buildLightCube(Shader &shader, GLuint sphere, vec3 lightPos);

bool collisionDetection();

const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;
bool noShadows = false;
bool noShadowsKeyPressed = true;
vec3 initialCameraPos = vec3(10.0f, 4.0f, 0.0f);
Camera camera(initialCameraPos);
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
// this is to set the initial location of the rackets
Location locations[2] = {{0.0f, 0.0f, 13.0f},
                         {0.0f, 0.0f, -13.0f},};


// textures variables
unsigned int buildingTextures[4];unsigned int balloonTextures[2];unsigned int shopTextures[8];unsigned int scraperTextures[6];
unsigned int fountainTextures[3];
unsigned int backgroundTexture;unsigned int treeTexture;unsigned int tree2Texture;unsigned int woodTexture;unsigned int streetTexture;
unsigned int floorTexture;unsigned int firehydrantTexture;unsigned int stopTexture;unsigned int welcomeTexture;unsigned int heyTexture;
unsigned int adventureTexture;unsigned int haveyouTexture;unsigned int visitTexture;

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
    GLuint sphere = createSphere(resolution, 1);

    // Enable Backface culling and depth test
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    Shader shader("shaders/textureShader.vs", "shaders/textureShader.fs");
    Shader depthShader("shaders/depthShader.vs", "shaders/depthShader.fs", "shaders/depthShader.gs");
    Shader lightCubeShader("shaders/lightShader.vs", "shaders/lightShader.fs");

    buildingTextures[0] = loadTexture("rec/textures/building.png");
    buildingTextures[1] = loadTexture("rec/textures/buildingB.png");
    buildingTextures[2] = loadTexture("rec/textures/buildingC.png");
    buildingTextures[3] = loadTexture("rec/textures/buildingD.png");
    backgroundTexture = loadTexture("rec/textures/background.png");
    treeTexture = loadTexture("rec/textures/tree.png");
    tree2Texture = loadTexture("rec/textures/tree2.png");
    woodTexture = loadTexture("rec/textures/wood.png");
    streetTexture = loadTexture("rec/textures/street.png");
    floorTexture = loadTexture("rec/textures/brick.jpg");
    scraperTextures[0] = loadTexture("rec/textures/scraper.png");
    scraperTextures[1] = loadTexture("rec/textures/scraper2.png");
    scraperTextures[2] = loadTexture("rec/textures/scraper3.png");
    scraperTextures[3] = loadTexture("rec/textures/scraper4.png");
    scraperTextures[4] = loadTexture("rec/textures/scraper5.png");
    scraperTextures[5] = loadTexture("rec/textures/scraper6.png");
    firehydrantTexture = loadTexture("rec/textures/firehydrant.png");
    stopTexture = loadTexture("rec/textures/stop.png");
    shopTextures[0] = loadTexture("rec/textures/JC.png");
    shopTextures[1] = loadTexture("rec/textures/MC.png");
    shopTextures[2] = loadTexture("rec/textures/TH.png");
    shopTextures[3] = loadTexture("rec/textures/SB.png");
    shopTextures[4] = loadTexture("rec/textures/FS.png");
    shopTextures[5] = loadTexture("rec/textures/HQ.png");
    shopTextures[6] = loadTexture("rec/textures/police.png");
    shopTextures[7] = loadTexture("rec/textures/RBC.png");
    balloonTextures[0] = loadTexture("rec/textures/balloon.png");
    balloonTextures[1] = loadTexture("rec/textures/blimp.png");
    welcomeTexture = loadTexture("rec/textures/welcome.png");
    heyTexture = loadTexture("rec/textures/hey.png");
    adventureTexture = loadTexture("rec/textures/adventure.png");
    haveyouTexture = loadTexture("rec/textures/haveyou.png");
    visitTexture = loadTexture("rec/textures/visit.png");
    fountainTextures[0] = loadTexture("rec/textures/fountainbase.png");
    fountainTextures[1] = loadTexture("rec/textures/fountaintop.png");
    fountainTextures[2] = loadTexture("rec/textures/water.png");

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

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        depthShader.use();
        for (unsigned int i = 0; i < 6; ++i)
            depthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        depthShader.setFloat("far_plane", far_plane);
        depthShader.setVec3("lightPos", lightPos);
        drawScene(depthShader, initialCube, blueBigCube, sphere);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. render scene as normal
        // -------------------------
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        shader.setVec3("viewPos", camera.Position);
        shader.setFloat("material.shininess", 32.0f);
        shader.setBool("isSpotLightOn", isSpotLightOn);

        /*
           Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index
           the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
           by defining light types as classes and set their values in there, or by using a more efficient uniform approach
           by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
        */
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

        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, backgroundTexture);
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, backgroundTexture);
        // DONT TOUCH
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

        drawScene(shader, initialCube, blueBigCube, sphere);
        buildLightCube(lightCubeShader, sphere, lightPos);

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
    buildBackground(shader, blueBigCube);
    buildBuildingABCD(shader, initialCube);
    buildScrapers(shader, initialCube);
    buildTree(shader, initialCube);
    buildStreetAndDecor(shader, initialCube, sphere);
    buildShops(shader, initialCube);
    buildFountain(shader, initialCube, sphere);
}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        if ( collisionDetection()) {
            camera.ProcessKeyboard(FORWARD, deltaTime);
        } else {
            camera.Position = initialCameraPos;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        if (collisionDetection()) {
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        } else {
            camera.Position = initialCameraPos;
        }

    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        if (collisionDetection()) {
            camera.ProcessKeyboard(LEFT, deltaTime);
        } else {
            camera.Position = initialCameraPos;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        if (collisionDetection()) {
            camera.ProcessKeyboard(RIGHT, deltaTime);
        } else {
            camera.Position = initialCameraPos;
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
    // bind specular map
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
    glDisable(GL_CULL_FACE);
    shader.setInt("reverse_normals", 1);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    shader.setInt("reverse_normals", 0);
    glEnable(GL_CULL_FACE);

}
void buildBuildingABCD(Shader &shader, GLuint initialCube) {
    glBindVertexArray(initialCube);// CUBE BASE
    for(int i=0;i<10;i++) {
        //Builds BuildingA
        int textureIndex = i % 4;
        translateMatrix = translate(mat4(1.0f), vec3(-115.0f+i*25, 20.0f, -115.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 20.0f, 25.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, buildingTextures[textureIndex]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //Builds BuildingA other side

        translateMatrix = translate(mat4(1.0f), vec3(-115.0f+i*25, 20.0f, 115.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 20.0f, 25.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, buildingTextures[textureIndex]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}
//this builds the main buildings in the center
void buildScrapers(Shader &shader, GLuint initialCube) {
    glBindVertexArray(initialCube);// CUBE BASE
    for(int i=0;i<3;i++) {

        translateMatrix = translate(mat4(1.0f), vec3(12.5f, 40.0f, -50.0f+i*50));
        scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 40.0f, 25.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, scraperTextures[i]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        translateMatrix = translate(mat4(1.0f), vec3(-12.5f, 40.0f, -50.0f+i*50));
        scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 40.0f, 25.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, scraperTextures[i+3]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
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
void buildStreetAndDecor(Shader &shader, GLuint initialCube, GLuint sphere) {
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
    //this builds the flooring
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
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    // this builds the stop signs
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
    //this draws the balloon
    float time = glfwGetTime(); // Get the current time
    float yOffset = cos(time) * 45.0f;
    translateMatrix = translate(mat4(1.0f), vec3(90.0f, 90.0f + yOffset, 0.0f));
    scaleMatrix = scale(mat4(1.0f), vec3(15.0f, 5.0f, 15.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, balloonTextures[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // testing drawing a sphere
    //////////////// 1 //////////////
    translateMatrix = translate(mat4(1.0f), vec3(0.0f, 50.0f, 20.0f));
    scaleMatrix = scale(mat4(1.0f), vec3(20.0f, 5.0f, 20.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, balloonTextures[0]);
    glBindVertexArray(sphere);
    glDrawElements(GL_TRIANGLES, vertexCount ,GL_UNSIGNED_INT,(void*)0);

    //this draws the blimp
    translateMatrix = translate(mat4(1.0f), vec3( yOffset, 100.0f, 0.0f));
    scaleMatrix = scale(mat4(1.0f), vec3(70.0f, 5.0f, 15.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, balloonTextures[1]);
    glDrawElements(GL_TRIANGLES, vertexCount ,GL_UNSIGNED_INT,(void*)0);
    // this draws the welcome sign
    glBindVertexArray(initialCube);
    translateMatrix = translate(mat4(1.0f), vec3( yOffset-31, 100.0f, 0.0f));
    scaleMatrix = scale(mat4(1.0f), vec3(50.0f, 2.0f, 0.1f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heyTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    translateMatrix = translate(mat4(1.0f), vec3( -70.0f, 5.0, -90.0f));
    scaleMatrix = scale(mat4(1.0f), vec3(22.5f, 5.0f, 0.25f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, welcomeTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    // this draws the adventure sign
    translateMatrix = translate(mat4(1.0f), vec3( -124.9f, 100.0, 0.0f));
    scaleMatrix = scale(mat4(1.0f), vec3(0.1f, 10.0f, 50.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, visitTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    // this draws the hey sign opposite to adventiure
    translateMatrix = translate(mat4(1.0f), vec3( 124.9f, 100.0, 0.0f));
    scaleMatrix = scale(mat4(1.0f), vec3(0.1f, 10.0f, 75.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, haveyouTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    translateMatrix = translate(mat4(1.0f), vec3(0.0f , 100.0, 124.9f));
    scaleMatrix = scale(mat4(1.0f), vec3(50.0f, 10.0f, 0.1f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, adventureTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    translateMatrix = translate(mat4(1.0f), vec3(0.0f , 100.0, -124.9f));
    scaleMatrix = scale(mat4(1.0f), vec3(50.0f, 10.0f, 0.1f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heyTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}
void buildShops(Shader &shader, GLuint initialCube) {
    for (int i = 0; i < 4; i++) {
        translateMatrix = translate(mat4(1.0f), vec3(-115.0f, 15.0f, 75.0f-i*50));
        scaleMatrix = scale(mat4(1.0f), vec3(20.0f, 15.0f, 70.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shopTextures[i]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    for (int i = 0; i < 4; i++) {
        translateMatrix = translate(mat4(1.0f), vec3(115.0f, 15.0f, 75.0f-i*50));scaleMatrix = scale(mat4(1.0f), vec3(20.0f, 15.0f, 70.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shopTextures[i+4]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

}
void buildFountain(Shader &shader, GLuint initialCube, GLuint sphere) {
    float time = glfwGetTime() * 5; // Get the current time
    float xOffset = sin(time) * 0.3f;
    float zOffset = cos(time) * 0.3f;
    //bottom of the fountain
    glBindVertexArray(initialCube);
    translateMatrix = translate(mat4(1.0f), vec3(-50.0f, 2.0f, -50.0f));
    scaleMatrix = scale(mat4(1.0f), vec3(27.0f, 2.0f, 27.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fountainTextures[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    // animating water
    glBindVertexArray(sphere);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fountainTextures[2]);
    translateMatrix = translate(mat4(1.0f), vec3(-49.75f - xOffset, 3.99f, -49.75f + zOffset));
    scaleMatrix = scale(mat4(1.0f), vec3(23.0f, 0.1f, 23.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, (void *) 0);
    translateMatrix = translate(mat4(1.0f), vec3(-50.0f + zOffset, 3.99f, -50.0f - xOffset));
    scaleMatrix = scale(mat4(1.0f), vec3(23.0f, 0.1f, 23.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, (void *) 0);
    translateMatrix = translate(mat4(1.0f), vec3(-50.25f - xOffset, 3.99f, -50.25f + zOffset));
    scaleMatrix = scale(mat4(1.0f), vec3(23.0f, 0.1f, 23.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, (void *) 0);
    translateMatrix = translate(mat4(1.0f), vec3(-50.5f + zOffset, 3.99f, -50.5f - zOffset));
    scaleMatrix = scale(mat4(1.0f), vec3(23.0f, 0.1f, 23.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, (void *) 0);
    translateMatrix = translate(mat4(1.0f), vec3(-50.f - xOffset, 3.99f, -50.0f + xOffset));
    scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 0.1f, 25.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, (void *) 0);
// top of the fountain
    glBindVertexArray(initialCube);
    translateMatrix = translate(mat4(1.0f), vec3(-50.0f, 9.0f, -50.0f));
    scaleMatrix = scale(mat4(1.0f), vec3(5.0f, 5.0f, 5.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fountainTextures[1]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    //animating water as squares
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fountainTextures[2]);
    translateMatrix = translate(mat4(1.0f), vec3(-50.0f , 3.998f, -50.0f ));
    scaleMatrix = scale(mat4(1.0f), vec3(26.0f, 0.01f, 26.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    translateMatrix = translate(mat4(1.0f), vec3(-50.5f + zOffset, 3.996f, -50.5f - zOffset));
    scaleMatrix = scale(mat4(1.0f), vec3(23.0f, 0.01f, 23.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    translateMatrix = translate(mat4(1.0f), vec3(-50.f - xOffset, 3.999f, -50.0f + xOffset));
    scaleMatrix = scale(mat4(1.0f), vec3(25.0f, 0.01f, 25.0f));
    modelMatrix = translateMatrix * scaleMatrix;
    shader.setMat4("modelMatrix", modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    //fountain bricks
    for (int i = 0; i < 2; i++) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        translateMatrix = translate(mat4(1.0f), vec3(-50.0f, 3.999f, -43.35f -i* 13.3));
        scaleMatrix = scale(mat4(1.0f), vec3(27.0f, 0.25f, 2.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    for (int i = 0; i < 2; i++) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        translateMatrix = translate(mat4(1.0f), vec3(-43.35f -i* 13.3, 3.999f,-50.0f));
        scaleMatrix = scale(mat4(1.0f), vec3(2.0f, 0.25f, 27.0f));
        modelMatrix = translateMatrix * scaleMatrix;
        shader.setMat4("modelMatrix", modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}
void buildLightCube(Shader &shader, GLuint sphere, vec3 lightPos) {

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
    return (camera.Position.y < bigWall.maxY && camera.Position.y > bigWall.minY)  &&  (camera.Position.x < bigWall.maxX && camera.Position.x > bigWall.minX)  &&  (camera.Position.z < bigWall.maxZ && camera.Position.z > bigWall.minZ);
}
