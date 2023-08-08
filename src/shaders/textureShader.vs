#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

uniform bool reverse_normals;

void main()
{
    vs_out.FragPos = vec3(modelMatrix * vec4(aPos.x/4.0f, aPos.y, aPos.z/4.0f , 1.0));
    if(reverse_normals) // a slight hack to make sure the outer large cube displays lighting from the 'inside' instead of the default 'outside'.
        vs_out.Normal = transpose(inverse(mat3(modelMatrix))) * (-1.0 * aNormal);
    else
        vs_out.Normal = transpose(inverse(mat3(modelMatrix))) * aNormal;
    vs_out.TexCoords = aTexCoords;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPos.x/4.0f, aPos.y, aPos.z/4.0f , 1.0);
}