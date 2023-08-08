#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 modelMatrix;

void main()
{
    gl_Position = modelMatrix * vec4(aPos.x/4.0f, aPos.y, aPos.z/4.0f , 1.0);
}