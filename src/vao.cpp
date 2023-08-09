
#include <vector>

#define GLEW_STATIC 1
#include <GL/glew.h>

// initializing OpenGL and binding inputs
#include <glm/glm.hpp>  // GLM is an optimized math library with syntax similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // include this to create transformation matrices

#include "vao.h"

using namespace glm;
using namespace std;

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;

GLuint createCubeCoordinate() {
    if (cubeVAO == 0) {
        float vertices[] = {
                // back face
                // positions                        //normals                     //uvs
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // bottom-left
                1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // top-right
                1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // bottom-right

                1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // top-right
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // bottom-left
                -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // top-left
                // front face
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // bottom-left
                1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // bottom-right
                1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // top-right

                1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // top-right
                -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // top-left
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // bottom-left
                // left face
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // top-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left

                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // bottom-right
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                // right face
                1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // top-right

                1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // bottom-left
                // bottom face
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left

                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                // top face
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right

                1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    return cubeVAO;
}



GLuint createSphere(int resolution, float radius)
{
    struct TexturedVertex
    {
        TexturedVertex(glm::vec3 _position, glm::vec2 _uv)
                : position(_position), uv(_uv) {}

        glm::vec3 position;
        glm::vec2 uv; // UV coordinates
    };

    std::vector<TexturedVertex> vertices;
    std::vector<GLuint> indices;

    for (int i = 0; i < resolution / 2; i++)
    {
        for (int j = 0; j < resolution; j++)
        {
            float iRads = static_cast<float>(i) / resolution * glm::two_pi<float>();
            float jRads = static_cast<float>(j) / resolution * glm::two_pi<float>();

            float x = std::sin(jRads) * std::sin(iRads);
            float y = std::cos(iRads);
            float z = std::cos(jRads) * std::sin(iRads);

            float u = static_cast<float>(j) / (resolution - 1);
            float v = static_cast<float>(i) / ((resolution / 2) - 1);

            vertices.push_back(TexturedVertex(glm::vec3(x, y, z) * radius, glm::vec2(u, v)));
        }
    }

    // Bottom vertex
    vertices.push_back(TexturedVertex(glm::vec3(0, -radius, 0), glm::vec2(0.0f, 0.0f)));

    int vertexCount = vertices.size();

    for (int i = 0; i < resolution * ((resolution - 2) / 2); i++)
    {
        indices.push_back(i);
        indices.push_back(i + resolution - 1);
        indices.push_back(i + resolution);

        indices.push_back(i + resolution);
        indices.push_back(i + 1);
        indices.push_back(i);
    }

    // Bottom faces
    for (int i = 0; i < resolution - 1; i++)
    {
        indices.push_back(vertexCount - 1);
        indices.push_back(vertexCount - 1 - resolution + i + 1);
        indices.push_back(vertexCount - 1 - resolution + i);
    }

    // Last triangle
    indices.push_back(vertexCount - resolution - 1);
    indices.push_back(vertexCount - 2);
    indices.push_back(vertexCount - 1);

    std::vector<GLfloat> vertexDataVector;

    for (int i = 0; i < vertexCount; i++)
    {
        TexturedVertex vertex = vertices[i];
        vertexDataVector.push_back(vertex.position.x);
        vertexDataVector.push_back(vertex.position.y);
        vertexDataVector.push_back(vertex.position.z);
        vertexDataVector.push_back(vertex.uv.x);
        vertexDataVector.push_back(vertex.uv.y);
    }


    // Create a vertex array object
    GLuint vertexArrayObject;
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    // Upload Vertex Buffer to the GPU, keep a reference to it (vertexBufferObject)
    GLuint vertexBufferObject;
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, vertexDataVector.size() * sizeof(GLfloat), &vertexDataVector[0], GL_STATIC_DRAW);

    // Set the vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0); // Vertex position
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat))); // UV coordinates

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(2);

    // Upload Index Buffer to the GPU, keep a reference to it (indexBufferObject)
    GLuint indexBufferObject;
    glGenBuffers(1, &indexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    // Unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return vertexArrayObject;
}


