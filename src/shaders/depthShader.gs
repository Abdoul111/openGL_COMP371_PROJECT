#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform bool isNight;
uniform bool noShadows;
uniform int nrLights;
uniform mat4 shadowMatrices[600];

out vec4 FragPos; // FragPos from GS (output per emitvertex)

void main()
{
    if (isNight && !noShadows) {
        for (int m = 0; (m < nrLights && m < shadowMatrices.length()); m++) {
            for(int face = 0; face < 6; ++face)
            {
                gl_Layer = face; // built-in variable that specifies to which face we render.
                for(int i = 0; i < 3; ++i) // for each triangle's vertices
                {
                    FragPos = gl_in[i].gl_Position;
                    gl_Position = shadowMatrices[m * face + face] * FragPos;
                    EmitVertex();
                }
                EndPrimitive();
            }
        }
    }

} 